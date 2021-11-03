#pragma once

#include "../region/region.h" // Region

// DescribeRegions available for this client
void describeRegions(Aws::EC2::EC2Client &, std::vector<Region> &);

// UpdateInstances within all regions
// Blocked until instance state are satisfied
void updateInstances(Aws::EC2::Model::InstanceStateName &,
                     std::vector<Region> &);

// DescribeInstances within all regions
void describeInstances(std::vector<Region> &);
