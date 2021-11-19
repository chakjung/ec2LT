#include "region.h"

#include "../errorCode.h" // CREATESGERRNUM, CONFIGSGERRNUM, DELETESGERRNUM

// Create SG via CreateSecurityGroupRequest
void Region::CreateSG(
    const Aws::EC2::Model::CreateSecurityGroupRequest &crtReq) {
  // Regional client
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = RegionName;
  Aws::EC2::EC2Client client(clientConfig);

  // Create SG
  const Aws::EC2::Model::CreateSecurityGroupOutcome &crtOutcome =
      client.CreateSecurityGroup(crtReq);

  if (!crtOutcome.IsSuccess()) {
    std::cout << "Failed to create security group\n"
              << crtOutcome.GetError().GetMessage() << std::endl;
    exit(CREATESGERRNUM);
  }

  SGId = crtOutcome.GetResult().GetGroupId();

  std::cout << "Security Group: " << crtReq.GetGroupName() << std::endl;
}

// Configure SG via AuthorizeSecurityGroupIngressRequest
void Region::ConfigSG(
    const Aws::EC2::Model::AuthorizeSecurityGroupIngressRequest &authReq) {
  // Regional client
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = RegionName;
  Aws::EC2::EC2Client client(clientConfig);

  // Configure SG
  const Aws::EC2::Model::AuthorizeSecurityGroupIngressOutcome &authOutcome =
      client.AuthorizeSecurityGroupIngress(authReq);

  if (!authOutcome.IsSuccess()) {
    std::cout << "Failed to configure security group " << SGId << "\n"
              << authOutcome.GetError().GetMessage() << std::endl;
    exit(CONFIGSGERRNUM);
  }
}

// Delete SG
void Region::DeleteSG() {
  // Regional client
  Aws::Client::ClientConfiguration clientConfig;
  clientConfig.region = RegionName;
  Aws::EC2::EC2Client client(clientConfig);

  Aws::EC2::Model::DeleteSecurityGroupRequest delReq;
  delReq.SetGroupId(SGId);

  // Delete SG
  const Aws::EC2::Model::DeleteSecurityGroupOutcome &delOutcome =
      client.DeleteSecurityGroup(delReq);

  if (!delOutcome.IsSuccess()) {
    std::cout << "Failed to delete security group " << SGId << "\n"
              << delOutcome.GetError().GetMessage() << std::endl;
    exit(DELETESGERRNUM);
  }
}
