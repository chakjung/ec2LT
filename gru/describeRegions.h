#pragma once

#include "region.h" // Region

// DescribeRegions available for this client
void describeRegions(Aws::EC2::EC2Client &client,
                     std::vector<Region> &regions) {
  Aws::EC2::Model::DescribeRegionsRequest desReq;
  Aws::EC2::Model::DescribeRegionsOutcome desOutcome =
      client.DescribeRegions(desReq);

  if (!desOutcome.IsSuccess()) {
    std::cout << "Failed to describe regions\n"
              << describeRegionsOutcome.GetError().GetMessage() << std::endl;
    exit(DESCRIBEREGIONSERRNUM);
  }

  // Foreach region
  for (const Aws::EC2::Model::Region &region :
       desOutcome.GetResult().GetRegions()) {
    // Config for regional client
    Aws::Client::ClientConfiguration regionalClientConfig;
    regionalClientConfig.region = region.GetRegionName();

    regions.push_back(
        // Create region
        Region{region.GetRegionName(),
               // Create regional client
               Aws::EC2::EC2Client(regionalClientConfig)});
  }
}
