#include <aws/core/Aws.h> // InitAPI(), ShutdownAPI()

#include "region/region.h" // Region

#include "subroutines/subroutines.h" // describeRegions()

#include "config.h" // DEBIAN10AWSACCOUNTID, OSFILTERS

int main() {
  // Init aws sdk
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  // Default Gru client
  Aws::EC2::EC2Client gruClient;

  // All regions
  std::vector<Region> regions;
  describeRegions(gruClient, regions);

  // OSId describe request
  Aws::EC2::Model::DescribeImagesRequest OSIdDesReq;
  // Set OS owner
  OSIdDesReq.SetOwners(Aws::Vector<Aws::String>{DEBIAN10AWSACCOUNTID});
  // Set OS filters
  for (const std::pair<std::string, std::string> &OSfilter : OSFILTERS) {
    Aws::EC2::Model::Filter filter;
    filter.SetName(OSfilter.first);
    filter.SetValues(Aws::Vector<Aws::String>{OSfilter.second});

    OSIdDesReq.AddFilters(filter);
  }

  // Foreach region
  for (Region &region : regions) {
    std::cout << region.RegionName << std::endl;
    region.QueryOSId(OSIdDesReq);
  }

  // Terminate aws sdk
  Aws::ShutdownAPI(options);

  exit(0);
}
