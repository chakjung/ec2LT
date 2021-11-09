#include <time.h>

unsigned long long calcTimestampNS(const struct timespec &wallTime) {
  unsigned long long timestamp = wallTime.tv_sec * 1000000000;
  timestamp += wallTime.tv_nsec;
  return timestamp;
}

unsigned long long calcIntervalNS(const struct timespec &start,
                                  const struct timespec &stop) {
  unsigned long long interval = (stop.tv_sec - start.tv_sec) * 1000000000;
  interval += stop.tv_nsec - start.tv_nsec;
  return interval;
}
