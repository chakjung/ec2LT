/**
 * EC2 network latency project
 *
 * File: availabilityZone.cpp
 *
 * Author: Jung Chak
 *
 * AZ
 */

#include "availabilityZone.h"

#include <limits.h> // ULLONG_MAX

#include "../errorCode.h" // RESTOULLERRNUM, HDSTOULLERRNUM, RTTTOULLERRNUM

// Add RES entry
void Entry::AddRES(const std::string &RESStr) {
  ++RESCount;
  RESSum += std::stoull(RESStr);
}

// Add RES entry
void Entry::AddRES(const char *RESStr) {
  ++RESCount;
  unsigned long long res = strtoull(RESStr, NULL, 10);
  if (res == 0 || res == ULLONG_MAX) {
    perror("strtoull");
    exit(RESTOULLERRNUM);
  }
  RESSum += res;
}

// Add HDS entry
void Entry::AddHDS(const std::string &HDSStr) {
  ++HDSCount;
  HDSSum += std::stoull(HDSStr);
}

// Add HDS entry
void Entry::AddHDS(const char *HDSStr) {
  ++HDSCount;
  unsigned long long hds = strtoull(HDSStr, NULL, 10);
  if (hds == 0 || hds == ULLONG_MAX) {
    perror("strtoull");
    exit(HDSTOULLERRNUM);
  }
  HDSSum += hds;
}

// Add RTT entry
void Entry::AddRTT(const std::string &RTTStr) {
  unsigned long long rtt = std::stoull(RTTStr);
  _AddRTT(rtt);
}

// Add RTT entry
void Entry::AddRTT(const char *RTTStr) {
  unsigned long long rtt = strtoull(RTTStr, NULL, 10);
  if (rtt == 0 || rtt == ULLONG_MAX) {
    perror("strtoull");
    exit(RTTTOULLERRNUM);
  }
  _AddRTT(rtt);
}

// Add RTT entry
void Entry::_AddRTT(const unsigned long long &rtt) {
  RTTSum += rtt;
  RTT.emplace(rtt);
}
