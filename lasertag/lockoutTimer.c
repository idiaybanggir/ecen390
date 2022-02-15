#include "lockoutTimer.h"
#include "intervalTimer.h"
#include <stdint.h>
#include <stdio.h>

// States for lockoutTimer SM
typedef enum {
  init_st,         // start here, transition out on first tick
  notLockedOut_st, // everything is normal--wait to be locked out
  lockedOut_st     // stay locked out for 500ms
} lockoutTimer_st_t;
static lockoutTimer_st_t currentState;

static uint16_t lockedOutCounter;
static bool timerRunning;

/*
 * Start the timer
 */
void lockoutTimer_start() { timerRunning = true; }

/*
 * Init the SM state and static variables
 */
void lockoutTimer_init() {
  currentState = init_st;
  lockedOutCounter = 0;
  timerRunning = false;
}

/*
 * Return true if the timer is currently running
 */
bool lockoutTimer_running() { return timerRunning; }

/*
 * Standard tick function
 */
void lockoutTimer_tick() {
  // State update
  switch (currentState) {
  case init_st: { // immediately transition out
    currentState = notLockedOut_st;
    break;
  }
  case notLockedOut_st: { // check if start() was called, stay here if not
    if (timerRunning) {
      currentState = lockedOut_st;
    } else {
      currentState = notLockedOut_st;
    }
    break;
  }
  case lockedOut_st: { // check if timer expired, stay here if not
    if (lockedOutCounter >= LOCKOUT_TIMER_EXPIRE_VALUE) {
      timerRunning = false;
      currentState = notLockedOut_st;
    } else {
      currentState = lockedOut_st;
    }
    break;
  }
  default:
    break;
  }

  // State actions
  switch (currentState) {
  case init_st:
    break;
  case notLockedOut_st:
    lockedOutCounter = 0;
    break;
  case lockedOut_st:
    lockedOutCounter++;
    break;
  default:
    lockoutTimer_init(); // something went wrong, reinit
    break;
  }
}

// Constants for the test fcn
#define TICK_RATE 100e3
#define SPEC_TIMER_DURATION LOCKOUT_TIMER_EXPIRE_VALUE / TICK_RATE
#define ACCURACY_THRESHOLD 1e-3 // good enough if we're within 1ms

/*
 * Tests the lockout timer SM by starting an interval timer,
 * starting the lockout timer, and comparing to ensure the lockout
 * lasts for .5s. Assumes interrupts are enabled, the tick fcn is
 * called by the isr, and the SM has been init'd. Returns true
 * if the SM passes, false otherwise
 */
bool lockoutTimer_runTest() {
  intervalTimer_init(INTERVAL_TIMER_TIMER_1);
  intervalTimer_start(INTERVAL_TIMER_TIMER_0);
  lockoutTimer_start();
  while (lockoutTimer_running()) // wait for the lockout SM to finish
    ;
  intervalTimer_stop(INTERVAL_TIMER_TIMER_0);
  double timeElapsed =
      intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_0);
  // If we're close enough to the specified duration (.5s),
  // we passed the test; else, we failed. Print outcome to console
  if (timeElapsed - SPEC_TIMER_DURATION < ACCURACY_THRESHOLD &&
      SPEC_TIMER_DURATION - timeElapsed < ACCURACY_THRESHOLD) {
    printf("Lockout timer test passed! Time elapsed: %f", timeElapsed);
    return true;
  } else {
    printf("Lockout timer test failed! Time elapsed: %f", timeElapsed);
    return false;
  }
}