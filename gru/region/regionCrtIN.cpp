/**
 * EC2 network latency project
 *
 * File: regionCrtIN.cpp
 *
 * Author: Jung Chak
 *
 * Create regional instances
 */

#include "region.h"

#include <aws/ec2/model/DescribeAvailabilityZonesRequest.h> // DescribeAvailabilityZonesRequest

#include "../errorCode.h" // DESCRIBEAZSERRNUM
// CREATEINSTANCESERRNUM, CREATEINSTANCESWRONGAMOUNTERRNUM

// Create instance foreach AZ available in this Region
void Region::CreateInstances(
    Aws::EC2::Model::RunInstancesRequest RegionINRunReq) {

  // Regional client
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = RegionName;
  Aws::EC2::EC2Client client(clientConfig);

  // Get all AZs
  Aws::EC2::Model::DescribeAvailabilityZonesRequest desAZsReq;
  const Aws::EC2::Model::DescribeAvailabilityZonesOutcome &desAZsOutcome =
      client.DescribeAvailabilityZones(desAZsReq);

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
    const Aws::EC2::Model::RunInstancesOutcome &INRunOutcome =
        client.RunInstances(INRunReq);

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

    AZs.push_back(AZ{az.GetZoneId(), instances[0]});

    std::cout << "Instance launched" << std::endl;
  }
}
