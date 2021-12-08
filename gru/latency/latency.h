/**
 * EC2 network latency project
 *
 * File: latency.h
 *
 * Author: Jung Chak
 *
 * Test latency
 */

#pragma once

#include "../availabilityZone/availabilityZone.h" // AZ

// Test latency between all AZs
void testLatency(const std::string &, const std::string &, std::vector<AZ *> &,
                 const int &, const int &, const int &, const int &);
