
#ifndef _TIMEINTERVAL_H
#define _TIMEINTERVAL_H


typedef struct {
  uint32_t interval;     // interval before interupt
  uint32_t curTime;      // current time (in ms)
  uint32_t prevTime;     // previous time (at interupt)
  uint32_t frames;       // frames passed between interuption
  uint32_t timers_flags; // bits for timers usage
  uint32_t timers[2];    // 2 generic timers
} TimeInterval;


#endif // _TIMEINTERVAL_H
