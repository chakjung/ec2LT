/**
 * EC2 network latency project
 *
 * File: timer.h
 *
 * Author: Jung Chak
 *
 * Stopwatch result translation
 */

#include <time.h>

// Unix Timestamp translation
unsigned long long calcTimestampNS(const struct timespec &wallTime) {
  unsigned long long timestamp = wallTime.tv_sec * 1000000000;
  timestamp += wallTime.tv_nsec;
  return timestamp;
}

// Time interval translation
unsigned long long calcIntervalNS(const struct timespec &start,
                                  const struct timespec &stop) {
  unsigned long long interval = (stop.tv_sec - start.tv_sec) * 1000000000;
  interval += stop.tv_nsec - start.tv_nsec;
  return interval;
}
