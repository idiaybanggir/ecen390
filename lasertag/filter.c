#include "filter.h"
#include "queue.h"
#include <stdio.h>

#define FIR_COEFFICIENT_COUNT 81
#define IIR_A_COEFFICIENT_COUNT 10
#define IIR_B_COEFFICIENT_COUNT 11

#define X_Q_SIZE 81
#define X_Q_NAME "X_QUEUE"
#define Y_Q_SIZE 11
#define Y_Q_NAME "Y_QUEUE"
#define Z_Q_SIZE 10
#define Z_Q_PREFIX "Z_QUEUE_"
#define OUTPUT_Q_SIZE 2000
#define OUTPUT_Q_PREFIX "OUTPUT_QUEUE_"

// FIR coefficients
const static double firCoefficients[FIR_COEFFICIENT_COUNT] = {
    1.7591213271896490e-06,  4.7527158997099115e-06,  1.0682966773943549e-05,
    1.8890949038898897e-05,  2.6981090692382611e-05,  2.9772255217181562e-05,
    1.9229738889071566e-05,  -1.4342900242330625e-05, -7.9478553783674571e-05,
    -1.7914899979642885e-04, -3.0550404113919753e-04, -4.3517640748120487e-04,
    -5.2714549132218652e-04, -5.2529251279684504e-04, -3.6726495437477448e-04,
    4.7091651532046472e-19,  5.9965484473857599e-04,  1.4029878030003705e-03,
    2.3118296878118080e-03,  3.1520123947934909e-03,  3.6840420460927279e-03,
    3.6347719220075213e-03,  2.7497365342947669e-03,  8.6055120297707263e-04,
    -2.0435364589549503e-03, -5.7548928531909634e-03, -9.8139357870712322e-03,
    -1.3517081464089640e-02, -1.5974699090704404e-02, -1.6218658549534564e-02,
    -1.3348568332241188e-02, -6.6958725781881313e-03, 4.0220727039765617e-03,
    1.8588027259092879e-02,  3.6237339589640490e-02,  5.5693644990595842e-02,
    7.5290626434934560e-02,  9.3166894916813708e-02,  1.0750656971034184e-01,
    1.1678804252843547e-01,  1.2000000000000000e-01,  1.1678804252843547e-01,
    1.0750656971034184e-01,  9.3166894916813708e-02,  7.5290626434934560e-02,
    5.5693644990595842e-02,  3.6237339589640490e-02,  1.8588027259092879e-02,
    4.0220727039765617e-03,  -6.6958725781881313e-03, -1.3348568332241188e-02,
    -1.6218658549534564e-02, -1.5974699090704404e-02, -1.3517081464089640e-02,
    -9.8139357870712322e-03, -5.7548928531909634e-03, -2.0435364589549503e-03,
    8.6055120297707263e-04,  2.7497365342947669e-03,  3.6347719220075213e-03,
    3.6840420460927279e-03,  3.1520123947934909e-03,  2.3118296878118080e-03,
    1.4029878030003705e-03,  5.9965484473857599e-04,  4.7091651532046472e-19,
    -3.6726495437477448e-04, -5.2529251279684504e-04, -5.2714549132218652e-04,
    -4.3517640748120487e-04, -3.0550404113919753e-04, -1.7914899979642885e-04,
    -7.9478553783674571e-05, -1.4342900242330625e-05, 1.9229738889071566e-05,
    2.9772255217181562e-05,  2.6981090692382611e-05,  1.8890949038898897e-05,
    1.0682966773943549e-05,  4.7527158997099115e-06,  1.7591213271896490e-06};

// 10 A coefficients for each filter (the first coefficient is always 1 and is
// not used)
const static double iirACoefficientConstants
    [FILTER_FREQUENCY_COUNT][IIR_A_COEFFICIENT_COUNT] = {
        {-5.9637727070164033e+00, 1.9125339333078266e+01,
         -4.0341474540744230e+01, 6.1537466875368942e+01,
         -7.0019717951472359e+01, 6.0298814235239050e+01,
         -3.8733792862566432e+01, 1.7993533279581129e+01,
         -5.4979061224867900e+00, 9.0332828533800036e-01},
        {-4.6377947119071479e+00, 1.3502215749461584e+01,
         -2.6155952405269787e+01, 3.8589668330738398e+01,
         -4.3038990303252703e+01, 3.7812927599537183e+01,
         -2.5113598088113825e+01, 1.2703182701888103e+01,
         -4.2755083391143529e+00, 9.0332828533800225e-01},
        {-3.0591317915750951e+00, 8.6417489609637510e+00,
         -1.4278790253808843e+01, 2.1302268283304294e+01,
         -2.2193853972079218e+01, 2.0873499791105424e+01,
         -1.3709764520609381e+01, 8.1303553577931567e+00,
         -2.8201643879900473e+00, 9.0332828533799825e-01},
        {-1.4071749185996771e+00, 5.6904141470697587e+00,
         -5.7374718273676431e+00, 1.1958028362868923e+01,
         -8.5435280598354861e+00, 1.1717345583835989e+01,
         -5.5088290876998816e+00, 5.3536787286077772e+00,
         -1.2972519209655635e+00, 9.0332828533800225e-01},
        {8.2010906117760252e-01, 5.1673756579268568e+00, 3.2580350909220872e+00,
         1.0392903763919179e+01, 4.8101776408668968e+00, 1.0183724507092489e+01,
         3.1282000712126661e+00, 4.8615933365571866e+00, 7.5604535083144642e-01,
         9.0332828533799725e-01},
        {2.7080869856154477e+00, 7.8319071217995546e+00, 1.2201607990980712e+01,
         1.8651500443681570e+01, 1.8758157568004485e+01, 1.8276088095998958e+01,
         1.1715361303018849e+01, 7.3684394621253206e+00, 2.4965418284511789e+00,
         9.0332828533800058e-01},
        {4.9479835250075901e+00, 1.4691607003177602e+01, 2.9082414772101060e+01,
         4.3179839108869331e+01, 4.8440791644688872e+01, 4.2310703962394328e+01,
         2.7923434247706425e+01, 1.3822186510471004e+01, 4.5614664160654339e+00,
         9.0332828533799869e-01},
        {6.1701893352279846e+00, 2.0127225876810336e+01, 4.2974193398071705e+01,
         6.5958045321253536e+01, 7.5230437667866738e+01, 6.4630411355740023e+01,
         4.1261591079244269e+01, 1.8936128791950619e+01, 5.6881982915180593e+00,
         9.0332828533800391e-01},
        {7.4092912870072389e+00, 2.6857944460290138e+01, 6.1578787811202268e+01,
         9.8258255839887383e+01, 1.1359460153696310e+02, 9.6280452143026224e+01,
         5.9124742025776499e+01, 2.5268527576524267e+01, 6.8305064480743294e+00,
         9.0332828533800336e-01},
        {8.5743055776347745e+00, 3.4306584753117924e+01, 8.4035290411037209e+01,
         1.3928510844056848e+02, 1.6305115418161665e+02, 1.3648147221895832e+02,
         8.0686288623300072e+01, 3.2276361903872257e+01, 7.9045143816245078e+00,
         9.0332828533800080e-01}};

// 11 B coefficients for each filter
const static double
    iirBCoefficientConstants[FILTER_FREQUENCY_COUNT][IIR_B_COEFFICIENT_COUNT] =
        {{9.0928661148190964e-10, 0.0000000000000000e+00,
          -4.5464330574095478e-09, 0.0000000000000000e+00,
          9.0928661148190956e-09, 0.0000000000000000e+00,
          -9.0928661148190956e-09, 0.0000000000000000e+00,
          4.5464330574095478e-09, 0.0000000000000000e+00,
          -9.0928661148190964e-10},
         {9.0928661148182558e-10, 0.0000000000000000e+00,
          -4.5464330574091276e-09, 0.0000000000000000e+00,
          9.0928661148182552e-09, 0.0000000000000000e+00,
          -9.0928661148182552e-09, 0.0000000000000000e+00,
          4.5464330574091276e-09, 0.0000000000000000e+00,
          -9.0928661148182558e-10},
         {9.0928661148195979e-10, 0.0000000000000000e+00,
          -4.5464330574097993e-09, 0.0000000000000000e+00,
          9.0928661148195985e-09, 0.0000000000000000e+00,
          -9.0928661148195985e-09, 0.0000000000000000e+00,
          4.5464330574097993e-09, 0.0000000000000000e+00,
          -9.0928661148195979e-10},
         {9.0928661148191306e-10, 0.0000000000000000e+00,
          -4.5464330574095652e-09, 0.0000000000000000e+00,
          9.0928661148191304e-09, 0.0000000000000000e+00,
          -9.0928661148191304e-09, 0.0000000000000000e+00,
          4.5464330574095652e-09, 0.0000000000000000e+00,
          -9.0928661148191306e-10},
         {9.0928661148200384e-10, 0.0000000000000000e+00,
          -4.5464330574100193e-09, 0.0000000000000000e+00,
          9.0928661148200386e-09, 0.0000000000000000e+00,
          -9.0928661148200386e-09, 0.0000000000000000e+00,
          4.5464330574100193e-09, 0.0000000000000000e+00,
          -9.0928661148200384e-10},
         {9.0928661148199133e-10, 0.0000000000000000e+00,
          -4.5464330574099564e-09, 0.0000000000000000e+00,
          9.0928661148199129e-09, 0.0000000000000000e+00,
          -9.0928661148199129e-09, 0.0000000000000000e+00,
          4.5464330574099564e-09, 0.0000000000000000e+00,
          -9.0928661148199133e-10},
         {9.0928661148195907e-10, 0.0000000000000000e+00,
          -4.5464330574097951e-09, 0.0000000000000000e+00,
          9.0928661148195903e-09, 0.0000000000000000e+00,
          -9.0928661148195903e-09, 0.0000000000000000e+00,
          4.5464330574097951e-09, 0.0000000000000000e+00,
          -9.0928661148195907e-10},
         {9.0928661148184554e-10, 0.0000000000000000e+00,
          -4.5464330574092277e-09, 0.0000000000000000e+00,
          9.0928661148184554e-09, 0.0000000000000000e+00,
          -9.0928661148184554e-09, 0.0000000000000000e+00,
          4.5464330574092277e-09, 0.0000000000000000e+00,
          -9.0928661148184554e-10},
         {9.0928661148198357e-10, 0.0000000000000000e+00,
          -4.5464330574099176e-09, 0.0000000000000000e+00,
          9.0928661148198351e-09, 0.0000000000000000e+00,
          -9.0928661148198351e-09, 0.0000000000000000e+00,
          4.5464330574099176e-09, 0.0000000000000000e+00,
          -9.0928661148198357e-10},
         {9.0928661148194956e-10, 0.0000000000000000e+00,
          -4.5464330574097480e-09, 0.0000000000000000e+00,
          9.0928661148194960e-09, 0.0000000000000000e+00,
          -9.0928661148194960e-09, 0.0000000000000000e+00,
          4.5464330574097480e-09, 0.0000000000000000e+00,
          -9.0928661148194956e-10}};

static queue_t x_Q;
static queue_t y_Q;
static queue_t z_Qs[FILTER_FREQUENCY_COUNT];
static queue_t output_Qs[FILTER_FREQUENCY_COUNT];

static queue_data_t filterPower[FILTER_FREQUENCY_COUNT];

static queue_data_t oldVals[FILTER_FREQUENCY_COUNT];

/*
 * Helper function: inits x queue and fills with zeros
 */
void filter_helper_initXQ() {
  queue_init(&x_Q, X_Q_SIZE, X_Q_NAME);
  filter_fillQueue(&x_Q, 0.0);
}

/*
 * Helper function: inits y queue and fills with zeros
 */
void filter_helper_initYQ() {
  queue_init(&y_Q, Y_Q_SIZE, Y_Q_NAME);
  filter_fillQueue(&y_Q, 0.0);
}

/*
 * Helper function: inits one z queue (specified by filterNumber)
 * and fills with zeros
 */
void filter_helper_initZQ(uint8_t filterNumber) {
  queue_init(&z_Qs[filterNumber], Z_Q_SIZE, Z_Q_PREFIX);
  filter_fillQueue(&z_Qs[filterNumber], 0.0);
}

/*
 * Helper function: inits one output queue (specified by filterNumber)
 * and fills with zeros
 */
void filter_helper_initOutputQ(uint8_t filterNumber) {
  queue_init(&output_Qs[filterNumber], OUTPUT_Q_SIZE, OUTPUT_Q_PREFIX);
  filter_fillQueue(&output_Qs[filterNumber], 0.0);
}

/*
 * Calls all helper queue init functions, and set initial power to 0
 */
void filter_init() {
  filter_helper_initXQ();
  filter_helper_initYQ();
  // init each z and output filter
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    filter_helper_initZQ(i);
    filter_helper_initOutputQ(i);
  }
  // fill the previous power calculations with 0s
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    filterPower[i] = 0;
  }
  // start off oldVals (used in power calculations) filled with 0s
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    oldVals[i] = 0;
  }
}

/*
 * Adds x to the x queue
 */
void filter_addNewInput(double x) { queue_overwritePush(&x_Q, x); }

/*
 * Fills a given queue with fillValue
 */
void filter_fillQueue(queue_t *q, double fillValue) {
  // Until the queue is full, overwrite push fillvalue
  for (uint32_t i = 0; i < q->size; i++) {
    queue_overwritePush(q, fillValue);
  }
}

/*
 * Runs the FIR filter on x queue, pushing output onto y queue
 * Also returns the output
 */
double filter_firFilter() {
  queue_data_t y = 0;
  // Read the appropriate element in x_queue and multiply by the right
  // coefficient
  for (uint32_t i = 0; i < FIR_COEFFICIENT_COUNT; i++) {
    y += queue_readElementAt(&x_Q, FIR_COEFFICIENT_COUNT - 1 - i) *
         firCoefficients[i];
  }
  queue_overwritePush(&y_Q, y);
  return y;
}

/*
 * Runs a given IIR filter on input from y queue,
 * pushes output onto the z and output queue for that filter.
 * Also returns the output.
 */
double filter_iirFilter(uint16_t filterNumber) {
  queue_data_t output = 0;
  // Add the convolution of the B coefficients and the y_queue
  for (uint32_t i = 0; i < IIR_B_COEFFICIENT_COUNT; i++) {
    output += queue_readElementAt(&y_Q, IIR_B_COEFFICIENT_COUNT - 1 - i) *
              iirBCoefficientConstants[filterNumber][i];
  }
  // Subtract the convolution of the z_queue (last outputs) and
  // the A coefficients
  for (uint32_t i = 0; i < IIR_A_COEFFICIENT_COUNT; i++) {
    output -= queue_readElementAt(&z_Qs[filterNumber],
                                  IIR_A_COEFFICIENT_COUNT - 1 - i) *
              iirACoefficientConstants[filterNumber][i];
  }
  queue_overwritePush(&z_Qs[filterNumber], output);
  queue_overwritePush(&output_Qs[filterNumber], output);
  return output;
}

/*
 * Computes and returns the signal power (really energy) for a given
 * filterNumber output queue. Can start over from scratch, or can keep a running
 * total. Debug print does nothing.
 */
double filter_computePower(uint16_t filterNumber, bool forceComputeFromScratch,
                           bool debugPrint) {
  queue_t *q = &output_Qs[filterNumber];
  // If starting over, set power to zero and oldvals to appr. value
  // else, adjust the running total
  if (forceComputeFromScratch) {
    oldVals[filterNumber] = queue_readElementAt(q, 0);
    filterPower[filterNumber] = 0;
    // loop through output queue and add squares of vals to the power
    for (uint32_t idx = 0; idx < OUTPUT_Q_SIZE; idx++) {
      queue_data_t val = queue_readElementAt(q, idx);
      filterPower[filterNumber] += val * val;
    }
  } else {
    queue_data_t oldVal = oldVals[filterNumber];
    oldVals[filterNumber] = queue_readElementAt(q, 0);
    queue_data_t newVal = queue_readElementAt(q, q->size - 1);
    filterPower[filterNumber] -= oldVal * oldVal;
    filterPower[filterNumber] += newVal * newVal;
  }
  return filterPower[filterNumber];
}

/*
 * Return the last power calculated for a given filter
 */
double filter_getCurrentPowerValue(uint16_t filterNumber) {
  return filterPower[filterNumber];
}

/*
 * Copy the last power calculated for each filter into the parameter array
 */
void filter_getCurrentPowerValues(double powerValues[]) {
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    powerValues[i] = filterPower[i];
  }
}

/*
 * Normalize the power array by its largest entry, and copy into
 * normalizedArray. Store the index of largest entry in indexOfMaxValue
 */
void filter_getNormalizedPowerValues(double normalizedArray[],
                                     uint16_t *indexOfMaxValue) {
  filter_getCurrentPowerValues(normalizedArray);
  queue_data_t max = normalizedArray[0];
  uint16_t maxIdx = 0;
  // for each entry in the array, check if it's the max
  for (uint8_t i = 1; i < FILTER_FREQUENCY_COUNT; i++) {
    if (normalizedArray[i] > max) {
      max = normalizedArray[i];
      maxIdx = i;
    }
  }
  *indexOfMaxValue = maxIdx;
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; i++) {
    normalizedArray[i] /= filterPower[*indexOfMaxValue];
  }
}

/*
 * Begin verification-assisting functions.
 */

// Returns the array of FIR coefficients.
const double *filter_getFirCoefficientArray() { return firCoefficients; }

// Returns the number of FIR coefficients.
uint32_t filter_getFirCoefficientCount() { return FIR_COEFFICIENT_COUNT; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirACoefficientArray(uint16_t filterNumber) {
  return iirACoefficientConstants[filterNumber];
}

// Returns the number of A coefficients.
uint32_t filter_getIirACoefficientCount() { return IIR_A_COEFFICIENT_COUNT; }

// Returns the array of coefficients for a particular filter number.
const double *filter_getIirBCoefficientArray(uint16_t filterNumber) {
  return iirBCoefficientConstants[filterNumber];
}

// Returns the number of B coefficients.
uint32_t filter_getIirBCoefficientCount() { return IIR_B_COEFFICIENT_COUNT; }

// Returns the size of the yQueue.
uint32_t filter_getYQueueSize() { return Y_Q_SIZE; }

// Returns the decimation value.
uint16_t filter_getDecimationValue() { return FILTER_FIR_DECIMATION_FACTOR; }

// Returns the address of xQueue.
queue_t *filter_getXQueue() { return &x_Q; }

// Returns the address of yQueue.
queue_t *filter_getYQueue() { return &y_Q; }

// Returns the address of zQueue for a specific filter number.
queue_t *filter_getZQueue(uint16_t filterNumber) { return &z_Qs[filterNumber]; }

// Returns the address of the IIR output-queue for a specific filter-number.
queue_t *filter_getIirOutputQueue(uint16_t filterNumber) {
  return &output_Qs[filterNumber];
}