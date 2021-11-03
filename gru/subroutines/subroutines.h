#pragma once

#include "../region/region.h" // Region

// DescribeRegions available for this client
void describeRegions(Aws::EC2::EC2Client &, std::vector<Region> &);

// UpdateInstances within all regions
// Blocked until instance state are satisfied
void updateInstances(const Aws::EC2::Model::InstanceStateName &,
                     std::vector<Region> &, unsigned int &);

// DescribeInstances within all regions
void describeInstances(std::vector<Region> &);

// TerminateInstances within all regions
void terminateInstances(std::vector<Region> &);
