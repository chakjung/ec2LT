#include "region.h"

#include "../config.h" // UPDATEINSTANCEDELAY

#include "../errorCode.h" // UPDATEINSTANCESERRNUM, UPDATEINSTANCESWRONGAMOUNTERRNUM

// UpdateInstances within this regions
// Blocked until instance state are satisfied
void Region::UpdateInstances(const Aws::EC2::Model::InstanceStateName &state) {
  // Foreach instance in instances
  for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
       Instances) {
    Aws::EC2::Model::DescribeInstancesRequest desReq;
    desReq.AddInstanceIds(instance.second.GetInstanceId());

    // Keep requesting until instance state == state
    bool match = false;
    while (!match) {
      Aws::EC2::Model::DescribeInstancesOutcome desOutcome =
          RegionalClient.DescribeInstances(desReq);

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
                  << instance.second.GetInstanceId() << "\n"
                  << describeOutcome.GetError().GetMessage() << std::endl;
        exit(UPDATEINSTANCESWRONGAMOUNTERRNUM);
      }

      // Not match yet
      if (describedInstances[0].GetState().GetName() != state) {
        sleep(UPDATEINSTANCEDELAY);
        continue;
      }

      match = true;
      instance.second = describedInstances[0];
    }
  }
}
