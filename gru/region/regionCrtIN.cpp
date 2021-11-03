#include "region.h"

#include "../errorCode.h" // DESCRIBEAZSERRNUM
// CREATEINSTANCESERRNUM, CREATEINSTANCESWRONGAMOUNTERRNUM

// Create instance foreach AZ available in this Region
void Region::CreateInstances(
    Aws::EC2::Model::RunInstancesRequest RegionINRunReq) {

  // Get all AZs
  Aws::EC2::Model::DescribeAvailabilityZonesRequest desAZsReq;
  Aws::EC2::Model::DescribeAvailabilityZonesOutcome desAZsOutcome =
      RegionalClient.DescribeAvailabilityZones(desAZsReq);

  if (!desAZsOutcome.IsSuccess()) {
    std::cout << "Failed to describe AZs\n"
              << desAZsOutcome.GetError().GetMessage() << std::endl;
    exit(DESCRIBEAZSERRNUM);
  }

  // Set OS
  RegionINRunReq.SetImageId(OSId);

  // Set SG
  RegionINRunReq.AddSecurityGroupIds(SGId);

  // Foreach AZ
  for (const Aws::EC2::Model::AvailabilityZone &az :
       desAZsOutcome.GetResult().GetAvailabilityZones()) {

    // Print AZ name & AZ Id
    std::cout << "Creating instance for " << az.GetZoneName() << " ("
              << az.GetZoneId() << ") "
              << "..." << std::endl;

    // Create instance request
    Aws::EC2::Model::RunInstancesRequest INRunReq = RegionINRunReq;

    // Set AZ
    Aws::EC2::Model::Placement placement;
    placement.SetAvailabilityZone(az.GetZoneName());
    INRunReq.SetPlacement(placement);

    // Create instance
    Aws::EC2::Model::RunInstancesOutcome INRunOutcome =
        RegionalClient.RunInstances(INRunReq);

    if (!INRunOutcome.IsSuccess()) {
      std::cout << "Failed to create EC2 instance in " << az.GetZoneName()
                << "\n"
                << "Based on " << OSId << "\n"
                << INRunOutcome.GetError().GetMessage() << std::endl;
      exit(CREATEINSTANCESERRNUM);
    }

    const Aws::Vector<Aws::EC2::Model::Instance> &instances =
        INRunOutcome.GetResult().GetInstances();

    if (instances.size() != 1) {
      std::cout << "Incorrect amount of EC2 instances launched in "
                << az.GetZoneName() << "\n"
                << INRunOutcome.GetError().GetMessage() << std::endl;
      exit(CREATEINSTANCESWRONGAMOUNTERRNUM);
    }

    Instances.push_back(std::pair<Aws::String, Aws::EC2::Model::Instance>(
        az.GetZoneId(), instances[0]));
    std::cout << "Instance launched" << std::endl;
  }
}
