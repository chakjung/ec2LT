#include "subroutines.h"

// Add rule to Security Group
void addSGRule(
    Aws::EC2::Model::AuthorizeSecurityGroupIngressRequest &SGConfigReq,
    const Aws::String &ip, const char *&protocol, const int &lowerPort,
    const int &upperPort) {
  Aws::EC2::Model::IpRange ipRange;
  ipRange.SetCidrIp(ip);
  Aws::EC2::Model::IpPermission ipPermission;
  ipPermission.SetIpProtocol(protocol);
  ipPermission.SetFromPort(lowerPort);
  ipPermission.SetToPort(upperPort);
  ipPermission.AddIpRanges(ipRange);
  SGConfigReq.AddIpPermissions(ipPermission);
}
