/**
 * EC2 network latency project
 *
 * File: describeRegions.cpp
 *
 * Author: Jung Chak
 *
 * DescribeRegions available
 */

#include "subroutines.h"

#include <aws/ec2/model/DescribeRegionsRequest.h> // DescribeRegionsRequest

#include "../errorCode.h" // DESCRIBEREGIONSERRNUM

// DescribeRegions available for this account
void describeRegions(std::vector<Region> &regions) {
  // Default client
  Aws::EC2::EC2Client client;

  Aws::EC2::Model::DescribeRegionsRequest desReq;
  const Aws::EC2::Model::DescribeRegionsOutcome &desOutcome =
      client.DescribeRegions(desReq);

  if (!desOutcome.IsSuccess()) {
    std::cout << "Failed to describe regions\n"
              << desOutcome.GetError().GetMessage() << std::endl;
    exit(DESCRIBEREGIONSERRNUM);
  }

  // Foreach region
  for (const Aws::EC2::Model::Region &region :
       desOutcome.GetResult().GetRegions()) {
    // Create region
    regions.push_back(Region{region.GetRegionName()});
  }
}
