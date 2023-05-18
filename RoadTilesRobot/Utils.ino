
#include <stdio.h> // sprintf

char *ms_format_time(uint32_t ms, char *dst) {
  uint32_t s = ms / 1000;
  uint32_t m = s / 60;
  uint32_t h = m / 60;
  ms %= 1000;
  s %= 60;
  m %= 60;
  sprintf(dst, "%02lu:%02lu:%02lu", h, m, s);
  return dst;
}

char *watt_dtostrf(float watts, char *str) {
  if (watts < 10) dtostrf(watts, 1, 2, str);
  else if (watts < 100) dtostrf(watts, 1, 1, str);
  else dtostrf(watts, 1, 0, str);
  return str;
}
