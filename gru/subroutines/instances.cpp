#include "subroutines.h"

// UpdateInstances within all regions
// Blocked until instance state are satisfied
void updateInstances(const Aws::EC2::Model::InstanceStateName &state,
                     std::vector<Region> &regions, const unsigned int &delay) {
  for (Region &region : regions) {
    region.UpdateInstances(state, delay);
  }
}

// DescribeInstances within all regions
void describeInstances(std::vector<Region> &regions) {
  std::cout << "Instances detail\n" << std::endl;

  for (Region &region : regions) {
    for (AZ &az : region.AZs) {
      std::cout << "AZ       ID: " << az.Id << std::endl;
      std::cout << "EC2      ID: " << az.Instance.GetInstanceId() << std::endl;
      std::cout << "Public  DNS: " << az.Instance.GetPublicDnsName()
                << std::endl;
      std::cout << "Public   IP: " << az.Instance.GetPublicIpAddress()
                << std::endl;
      std::cout << "Private DNS: " << az.Instance.GetPrivateDnsName()
                << std::endl;
      std::cout << "Private  IP: " << az.Instance.GetPrivateIpAddress() << "\n"
                << std::endl;
    }
  }
}

// TerminateInstances within all regions
void terminateInstances(std::vector<Region> &regions) {
  for (Region &region : regions) {
    region.TerminateInstances();
  }
}
