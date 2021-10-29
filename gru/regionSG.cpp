#include "region.h"

#include "errorCode.h" // CREATESGERRNUM, CONFIGSGERRNUM, DELETESGERRNUM

// Create SG via CreateSecurityGroupRequest
void Region::CreateSG(
    const Aws::EC2::Model::CreateSecurityGroupRequest &crtReq) {
  // Create SG
  Aws::EC2::Model::CreateSecurityGroupOutcome crtOutcome =
      RegionalClient.CreateSecurityGroup(crtReq);

  if (!crtOutcome.IsSuccess()) {
    std::cout << "Failed to create security group\n"
              << crtOutcome.GetError().GetMessage() << std::endl;
    exit(CREATESGERRNUM);
  }

  SGId = createOutcome.GetResult().GetGroupId();

  std::cout << "Security Group: " << crtReq.GetGroupName() << std::endl;
}

// Configure SG via AuthorizeSecurityGroupIngressRequest
void ConfigSG(
    const Aws::EC2::Model::AuthorizeSecurityGroupIngressRequest &authReq) {
  // Configure SG
  Aws::EC2::Model::AuthorizeSecurityGroupIngressOutcome authOutcome =
      RegionalClient.AuthorizeSecurityGroupIngress(authReq);

  if (!authOutcome.IsSuccess()) {
    std::cout << "Failed to configure security group " << SGId << "\n"
              << authOutcome.GetError().GetMessage() << std::endl;
    exit(CONFIGSGERRNUM);
  }
}

// Delete SG
void Region::DeleteSG() {
  Aws::EC2::Model::DeleteSecurityGroupRequest delReq;
  delReq.SetGroupId(SGId);

  // Delete SG
  Aws::EC2::Model::DeleteSecurityGroupOutcome delOutcome =
      RegionalClient.DeleteSecurityGroup(delReq);

  if (!delOutcome.IsSuccess()) {
    std::cout << "Failed to delete security group " << SGId << "\n"
              << delOutcome.GetError().GetMessage() << std::endl;
    exit(DELETESGERRNUM);
  }
}
