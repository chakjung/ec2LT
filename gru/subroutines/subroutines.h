#pragma once

#include "../region/region.h" // Region

// DescribeRegions available for this client
void describeRegions(Aws::EC2::EC2Client &, std::vector<Region> &);
