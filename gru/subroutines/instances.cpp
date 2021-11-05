#include "subroutines.h"

// UpdateInstances within all regions
// Blocked until instance state are satisfied
void updateInstances(const Aws::EC2::Model::InstanceStateName &state,
                     std::vector<Region> &regions, unsigned int &delay) {
  for (Region &region : regions) {
    region.UpdateInstances(state, delay);
  }
}

// DescribeInstances within all regions
void describeInstances(std::vector<Region> &regions) {
  std::cout << "Instances detail\n" << std::endl;

  for (Region &region : regions) {
    for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
         region.Instances) {
      std::cout << "AZ       ID: " << instance.first << std::endl;
      std::cout << "EC2      ID: " << instance.second.GetInstanceId()
                << std::endl;
      std::cout << "Public  DNS: " << instance.second.GetPublicDnsName()
                << std::endl;
      std::cout << "Public   IP: " << instance.second.GetPublicIpAddress()
                << std::endl;
      std::cout << "Private DNS: " << instance.second.GetPrivateDnsName()
                << std::endl;
      std::cout << "Private  IP: " << instance.second.GetPrivateIpAddress()
                << "\n\n"
                << std::flush;
    }
  }
}

// TerminateInstances within all regions
void terminateInstances(std::vector<Region> &regions) {
  for (Region &region : regions) {
    region.TerminateInstances();
  }
}
