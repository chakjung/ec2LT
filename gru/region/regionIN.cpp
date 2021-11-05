#include "region.h"

#include "../errorCode.h"
// UPDATEINSTANCESERRNUM, UPDATEINSTANCESWRONGAMOUNTERRNUM
// TERMINATEINSTANCESERRNUM

// UpdateInstances within this region
// Blocked until instance state are satisfied
void Region::UpdateInstances(const Aws::EC2::Model::InstanceStateName &state,
                             const unsigned int &delay) {
  // Foreach instance in instances
  for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
       Instances) {
    Aws::EC2::Model::DescribeInstancesRequest desReq;
    desReq.AddInstanceIds(instance.second.GetInstanceId());

    std::cout << instance.second.GetInstanceId() << "\n" << std::flush;

    // Keep requesting until instance state == state
    bool match = false;
    while (!match) {
      Aws::EC2::Model::DescribeInstancesOutcome desOutcome =
          RegionalClient.DescribeInstances(desReq);

      if (!desOutcome.IsSuccess()) {
        std::cout << "Failed to describe EC2 instance "
                  << instance.second.GetInstanceId() << "\n"
                  << desOutcome.GetError().GetMessage() << "\n"
                  << std::flush;
        exit(UPDATEINSTANCESERRNUM);
      }

      const Aws::Vector<Aws::EC2::Model::Instance> &describedInstances =
          desOutcome.GetResult().GetReservations()[0].GetInstances();

      if (describedInstances.size() != 1) {
        std::cout << "Incorrect amount of EC2 instances described for "
                  << instance.second.GetInstanceId() << "\n"
                  << desOutcome.GetError().GetMessage() << "\n"
                  << std::flush;
        exit(UPDATEINSTANCESWRONGAMOUNTERRNUM);
      }

      // Not match yet
      if (describedInstances[0].GetState().GetName() != state) {
        sleep(delay);
        continue;
      }

      match = true;
      instance.second = describedInstances[0];
    }
  }
}

// TerminateInstances within this region
void Region::TerminateInstances() {
  if (Instances.empty()) {
    return;
  }

  Aws::EC2::Model::TerminateInstancesRequest termReq;

  // Foreach instance in instances
  for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
       Instances) {
    termReq.AddInstanceIds(instance.second.GetInstanceId());
  }

  Aws::EC2::Model::TerminateInstancesOutcome termOutcome =
      RegionalClient.TerminateInstances(termReq);

  if (!termOutcome.IsSuccess()) {
    std::cout << "Failed to terminate EC2 instance at " << RegionName << "\n"
              << termOutcome.GetError().GetMessage() << std::endl;
    exit(TERMINATEINSTANCESERRNUM);
  }
}
