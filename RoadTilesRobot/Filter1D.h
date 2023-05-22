
#ifndef FILTER1D_H_
#define FILTER1D_H_

#include <stdint.h>

class Filter1D
{
  public:
    Filter1D();
    Filter1D(uint32_t max_samples);
    void addSample(float cur);
    float getValue();
    void update();
    
  private:
    float kernel[32];
    float samples[32];
    uint32_t idx, max_samples;
};



#endif
