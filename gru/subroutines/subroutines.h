#pragma once

#include "../region/region.h" // Region

// DescribeRegions available for this account
void describeRegions(std::vector<Region> &);

// UpdateInstances within all regions
// Blocked until instance state are satisfied
void updateInstances(const Aws::EC2::Model::InstanceStateName &,
                     std::vector<Region> &, const unsigned int &);

// DescribeInstances within all regions
void describeInstances(std::vector<Region> &);

// TerminateInstances within all regions
void terminateInstances(std::vector<Region> &);

// Add rule to Security Group
void addSGRule(Aws::EC2::Model::AuthorizeSecurityGroupIngressRequest &,
               const Aws::String &, const char *, const int &, const int &);
