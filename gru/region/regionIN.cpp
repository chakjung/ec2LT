#include "region.h"

#include "../errorCode.h"
// UPDATEINSTANCESERRNUM, UPDATEINSTANCESWRONGAMOUNTERRNUM
// TERMINATEINSTANCESERRNUM

// UpdateInstances within this region
// Blocked until instance state are satisfied
void Region::UpdateInstances(const Aws::EC2::Model::InstanceStateName &state,
                             const unsigned int &delay) {
  std::cout << RegionName << std::endl;

  // Regional client
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = RegionName;
  Aws::EC2::EC2Client client(clientConfig);

  for (AZ &az : AZs) {
    Aws::EC2::Model::DescribeInstancesRequest desReq;
    desReq.AddInstanceIds(az.Instance.GetInstanceId());

    std::cout << az.Instance.GetInstanceId() << std::endl;

    // Keep requesting until instance state == state
    while (true) {
      const Aws::EC2::Model::DescribeInstancesOutcome &desOutcome =
          client.DescribeInstances(desReq);

      if (!desOutcome.IsSuccess()) {
        std::cout << "Failed to describe EC2 instance "
                  << instance.second.GetInstanceId() << "\n"
                  << desOutcome.GetError().GetMessage() << std::endl;
        exit(UPDATEINSTANCESERRNUM);
      }

      const Aws::Vector<Aws::EC2::Model::Instance> &describedInstances =
          desOutcome.GetResult().GetReservations()[0].GetInstances();

      if (describedInstances.size() != 1) {
        std::cout << "Incorrect amount of EC2 instances described for "
                  << az.Instance.GetInstanceId() << "\n"
                  << desOutcome.GetError().GetMessage() << std::endl;
        exit(UPDATEINSTANCESWRONGAMOUNTERRNUM);
      }

      // Not match yet
      if (describedInstances[0].GetState().GetName() != state) {
        sleep(delay);
        continue;
      }

      az.Instance = describedInstances[0];
      break;
    }
  }
}

// TerminateInstances within this region
void Region::TerminateInstances() {
  // No instances
  if (AZs.empty()) {
    return;
  }

  // Regional client
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = RegionName;
  Aws::EC2::EC2Client client(clientConfig);

  Aws::EC2::Model::TerminateInstancesRequest termReq;

  for (AZ &az : AZs) {
    termReq.AddInstanceIds(az.Instance.GetInstanceId());
  }

  const Aws::EC2::Model::TerminateInstancesOutcome &termOutcome =
      client.TerminateInstances(termReq);

  if (!termOutcome.IsSuccess()) {
    std::cout << "Failed to terminate EC2 instance at " << RegionName << "\n"
              << termOutcome.GetError().GetMessage() << std::endl;
    exit(TERMINATEINSTANCESERRNUM);
  }
}
