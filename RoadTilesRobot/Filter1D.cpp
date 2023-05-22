
#include "Filter1D.h"


Filter1D::Filter1D()
{
  this->max_samples = 16;
  this->idx = 0;
}
Filter1D::Filter1D(uint32_t max_samples)
{
  this->max_samples = max_samples;
  this->idx = 0;
  for (uint32_t i = 0; i < max_samples; i++) {
    kernel[i] = 1.0 / max_samples;
  }
}



void Filter1D::addSample(float sample)
{
  samples[(++idx) % max_samples] = sample;
}

float Filter1D::getValue()
{
  if (idx < max_samples) return 0;

  float acc = 0;
  uint32_t idx_zero = idx+1;
  for (uint32_t i = 0; i < max_samples; i++) {
    acc += samples[(idx_zero + i) % max_samples] * kernel[i];
  }
  return acc;
}

void Filter1D::update()
{
  //prev = this->getValue();
}
