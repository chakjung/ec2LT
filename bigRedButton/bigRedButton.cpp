#include <aws/core/Aws.h> // InitAPI(), ShutdownAPI()

#include <aws/ec2/EC2Client.h> // EC2Client

#include <aws/ec2/model/DescribeRegionsRequest.h> // DescribeRegionsRequest

#include <aws/ec2/model/DescribeInstancesRequest.h> // DescribeInstancesRequest

#include <aws/ec2/model/TerminateInstancesRequest.h> // TerminateInstancesRequest

#include <unistd.h> // sleep

#include <aws/ec2/model/DescribeSecurityGroupsRequest.h> // DescribeSecurityGroupsRequest

#include <aws/ec2/model/DeleteSecurityGroupRequest.h> // DeleteSecurityGroupRequest

#include "config.h" // INSTANCEWHITELIST, UPDATEINSTANCEDELAY, SGWHITELIST

#include "errorCode.h" // DESCRIBEREGIONSERRNUM
// UPDATEINSTANCESERRNUM, TERMINATEINSTANCESERRNUM
// DESCRIBESGSERRNUM, DELETESGERRNUM

int main() {
  // Init aws sdk
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  std::cout << std::endl;

  // Default client
  Aws::EC2::EC2Client defaultClient;

  // DescribeRegions available for defaultClient
  Aws::EC2::Model::DescribeRegionsRequest desRegionsReq;
  Aws::EC2::Model::DescribeRegionsOutcome desRegionsOutcome =
      defaultClient.DescribeRegions(desRegionsReq);

  if (!desRegionsOutcome.IsSuccess()) {
    std::cout << "Failed to describe regions\n"
              << desRegionsOutcome.GetError().GetMessage() << std::endl;
    exit(DESCRIBEREGIONSERRNUM);
  }

  // Foreach region
  for (const Aws::EC2::Model::Region &region :
       desRegionsOutcome.GetResult().GetRegions()) {
    std::cout << "Tearing down " << region.GetRegionName() << std::endl;

    // Config for regional client
    Aws::Client::ClientConfiguration regionalClientConfig;
    regionalClientConfig.region = region.GetRegionName();
    Aws::EC2::EC2Client regionalClient(regionalClientConfig);

    std::cout << "Terminating instances..." << std::endl;

    while (true) {
      bool terminationComplete = true;

      // DescribeInstances
      Aws::EC2::Model::DescribeInstancesRequest desINReq;
      Aws::EC2::Model::DescribeInstancesOutcome desINOutcome =
          regionalClient.DescribeInstances(desINReq);

      if (!desINOutcome.IsSuccess()) {
        std::cout << "Failed to describe EC2 instances\n"
                  << desINOutcome.GetError().GetMessage() << std::endl;
        exit(UPDATEINSTANCESERRNUM);
      }

      Aws::EC2::Model::TerminateInstancesRequest termINReq;

      // Foreach instance in this region
      for (const Aws::EC2::Model::Reservation &reservation :
           desINOutcome.GetResult().GetReservations()) {
        for (const Aws::EC2::Model::Instance &instance :
             reservation.GetInstances()) {

          // Current instance whitelisted
          if (INSTANCEWHITELIST.find(instance.GetInstanceId()) !=
              INSTANCEWHITELIST.end()) {
            continue;
          }

          // Current instance not terminated
          if (instance.GetState().GetName() !=
              Aws::EC2::Model::InstanceStateName::terminated) {
            termINReq.AddInstanceIds(instance.GetInstanceId());
            terminationComplete = false;
          }
        }
      }

      if (terminationComplete) {
        break;
      } else {
        Aws::EC2::Model::TerminateInstancesOutcome termINOutcome =
            regionalClient.TerminateInstances(termINReq);
        if (!termINOutcome.IsSuccess()) {
          std::cout << "Failed to terminate EC2 instance at "
                    << region.GetRegionName() << "\n"
                    << termINOutcome.GetError().GetMessage() << std::endl;
          exit(TERMINATEINSTANCESERRNUM);
        }
        sleep(UPDATEINSTANCEDELAY);
      }
    }

    std::cout << "Instances terminated" << std::endl;

    std::cout << "Deleting SGs..." << std::endl;

    // DescribeSGs
    Aws::EC2::Model::DescribeSecurityGroupsRequest desSGReq;
    Aws::EC2::Model::DescribeSecurityGroupsOutcome desSGOutcome =
        regionalClient.DescribeSecurityGroups(desSGReq);
    if (!desSGOutcome.IsSuccess()) {
      std::cout << "Failed to describe SGs\n"
                << desSGOutcome.GetError().GetMessage() << std::endl;
      exit(DESCRIBESGSERRNUM);
    }

    for (const Aws::EC2::Model::SecurityGroup &sg :
         desSGOutcome.GetResult().GetSecurityGroups()) {

      // Default SG can't be deleted
      if (sg.GetGroupName() == "default") {
        continue;
      }

      // Current SG whitelisted
      if (SGWHITELIST.find(sg.GetGroupName()) != SGWHITELIST.end()) {
        continue;
      }

      Aws::EC2::Model::DeleteSecurityGroupRequest delSGReq;
      delSGReq.SetGroupId(sg.GetGroupId());

      // Delete SG
      Aws::EC2::Model::DeleteSecurityGroupOutcome delSGOutcome =
          regionalClient.DeleteSecurityGroup(delSGReq);

      if (!delSGOutcome.IsSuccess()) {
        std::cout << "Failed to delete security group " << sg.GetGroupId()
                  << "\n"
                  << delSGOutcome.GetError().GetMessage() << std::endl;
        exit(DELETESGERRNUM);
      }
    }

    std::cout << "SGs deleted\n" << std::endl;
  }

  // Terminate aws sdk
  Aws::ShutdownAPI(options);

  exit(0);
}
