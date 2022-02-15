/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "hitLedTimer.h"
#include "leds.h"
#include <stdbool.h>
#include <stdio.h>

// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensure that only one hit is detected per 1/2-second interval.

#define HIT_LED_TIMER_EXPIRE_VALUE 50000 // Defined in terms of 100 kHz ticks.
#define HIT_LED_TIMER_OUTPUT_PIN 11      // JF-3
#define LED_VAL_ON 0x1
#define LED_VAL_OFF 0x0

bool LedTimerRunning;
uint16_t LedTimer;


static enum hitLedTimer {
  init_st,              // initial state
  off_st,               // led off state
  on_st,                // led on state
} currentState;

// Calling this starts the timer.
void hitLedTimer_start(){
    LedTimerRunning = true;
}

// Returns true if the timer is currently running.
bool hitLedTimer_running(){
    return LedTimerRunning;
}

// Standard tick function.
void hitLedTimer_tick(){
    switch(currentState){
        case init_st:
            hitLedTimer_init();
            currentState = off_st;
        break;
        case off_st:
            hitLedTimer_turnLedOff();
            if(hitLedTimer_running())
                currentState = on_st;
            else
                currentState = off_st;
        break;
        case on_st:
            hitLedTimer_turnLedOn();
            if(LedTimer == HIT_LED_TIMER_EXPIRE_VALUE)
                currentState = off_st;
        break;
        default:
            printf("FAILURE IN HIT LED TIMER SM\n");
        break;
    }
    switch(currentState){
        case init_st:
            LedTimer = 0;
        break;
        case on_st:
            LedTimer++;
        break;
        default:
            printf("FAILURE!!\n");
        break;
    }
}

// Need to init things.
void hitLedTimer_init(){
    currentState = init_st;
    LedTimer = 0;
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn(){
    leds_write(LED_VAL_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff(){
    leds_write(LED_VAL_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable(){
    LedTimerRunning = false;
}

// Enables the hitLedTimer.
void hitLedTimer_enable(){
    LedTimerRunning = true;
}

// Runs a visual test of the hit LED.
// The test continuously blinks the hit-led on and off.
void hitLedTimer_runTest(){
    while(1){
        hitLedTimer_start();
        while(!hitLedTimer_running()){
            utils_msDelay(300);
        }
    }
}

