#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include <jack/jack.h>

#include <time.h>

static inline unsigned long long ccnt_read(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long long)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

volatile float taps[2048];
volatile float coefs[2048];

float processSampleDirectFull(float x, int numtaps) {
    taps[0] = x;

    float q = 0.0;
    uint16_t i;
    for (i = 0; i<numtaps; i++)
        q += taps[i] * coefs[i];

    for (i = numtaps-1; i>0; i--)
        taps[i] = taps[i-1];

    return q;
}

int head = 0;

float processSampleDirectFullCircular(float x, int numtaps) {
    taps[(numtaps - head) % numtaps] = x;

    float q = 0.0;
    uint16_t i;
    for (i = 0; i<numtaps; i++)
        q += taps[i] * coefs[(i + head) % numtaps];

    head = (head + 1) % numtaps;

    return q;
}

int main (int argc, char *argv[]) {

  volatile float result;

  int j;
  long long t1, t2, t3, t4;

  for (j=3; j<12; j++) {
    result = processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    t1 = ccnt_read();
    result += processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    result += processSampleDirectFull(1.0, 1 << j);
    t2 = ccnt_read();

    result = processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    t3 = ccnt_read();
    result += processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    result += processSampleDirectFullCircular(1.0, 1 << j);
    t4 = ccnt_read();

    printf("%5d  direct %8lld  circular %8lld  ratio %1.3f \n", 1<<j, (t2-t1)/5, (t4-t3)/5, 1.0*(t2-t1)/(t4-t3));
  }
  
  exit (0);
}
