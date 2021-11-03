#include <aws/core/Aws.h> // InitAPI(), ShutdownAPI()

#include "region/region.h" // Region

#include "subroutines/subroutines.h" // describeRegions()
// updateInstances(), describeInstances(), terminateInstances()

#include "config.h" // TAGKEY
// DEBIAN10AWSACCOUNTID, OSFILTERS
// SECURITYGROUPNAME, SECURITYGROUPDESCRIPTION
// INSTANCETYPE, MINIONSPORT

int main() {
  // Init aws sdk
  Aws::SDKOptions options;
  Aws::InitAPI(options);

  std::cout << std::endl;

  // Tag for all resources created
  Aws::EC2::Model::Tag tag;
  tag.WithKey(TAGKEY).SetValue("");

  // Default Gru client
  Aws::EC2::EC2Client gruClient;

  // All regions
  std::vector<Region> regions;
  describeRegions(gruClient, regions);

  // OSId describe request
  Aws::EC2::Model::DescribeImagesRequest OSIdDesReq;
  // Set OS owner
  OSIdDesReq.SetOwners(Aws::Vector<Aws::String>{DEBIAN10AWSACCOUNTID});
  // Set OS filters
  for (const std::pair<std::string, std::string> &OSfilter : OSFILTERS) {
    Aws::EC2::Model::Filter filter;
    filter.SetName(OSfilter.first);
    filter.SetValues(Aws::Vector<Aws::String>{OSfilter.second});
    OSIdDesReq.AddFilters(filter);
  }

  // Create SG request
  Aws::EC2::Model::CreateSecurityGroupRequest SGCrtReq;
  SGCrtReq.SetGroupName(SECURITYGROUPNAME);
  SGCrtReq.SetDescription(SECURITYGROUPDESCRIPTION);
  // Set tag
  Aws::EC2::Model::TagSpecification SGTagSpec;
  SGTagSpec.AddTags(tag);
  SGTagSpec.SetResourceType(Aws::EC2::Model::ResourceType::security_group);
  SGCrtReq.AddTagSpecifications(SGTagSpec);

  // Create instance request
  Aws::EC2::Model::RunInstancesRequest INCrtReq;
  // Set Instance type
  INCrtReq.SetInstanceType(INSTANCETYPE);
  INCrtReq.SetMinCount(1);
  INCrtReq.SetMaxCount(1);
  // Set tag
  Aws::EC2::Model::TagSpecification INTagSpec;
  INTagSpec.AddTags(tag);
  INTagSpec.SetResourceType(Aws::EC2::Model::ResourceType::instance);
  INCrtReq.AddTagSpecifications(INTagSpec);

  // Foreach region
  for (Region &region : regions) {
    std::cout << region.RegionName << std::endl;
    region.QueryOSId(OSIdDesReq);
    region.CreateSG(SGCrtReq);
    region.CreateInstances(INCrtReq);
    std::cout << std::endl;
  }

  updateInstances(Aws::EC2::Model::InstanceStateName::running, regions);
  describeInstances(regions);

  // Config SG request
  Aws::EC2::Model::AuthorizeSecurityGroupIngressRequest SGConfigReq;
  SGConfigReq.SetGroupName(SECURITYGROUPNAME);
  for (Region &region : regions) {
    for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
         region.Instances) {
      Aws::EC2::Model::IpRange publicIp;
      publicIp.SetCidrIp(instance.second.GetPublicIpAddress() + "/32");
      Aws::EC2::Model::IpPermission publicIpPermission;
      publicIpPermission.SetIpProtocol("tcp");
      publicIpPermission.SetToPort(MINIONSPORT);
      publicIpPermission.SetFromPort(MINIONSPORT);
      publicIpPermission.AddIpRanges(publicIp);
      SGConfigReq.AddIpPermissions(publicIpPermission);

      Aws::EC2::Model::IpRange privateIp;
      privateIp.SetCidrIp(instance.second.GetPrivateIpAddress() + "/32");
      Aws::EC2::Model::IpPermission privateIpPermission;
      privateIpPermission.SetIpProtocol("tcp");
      privateIpPermission.SetToPort(MINIONSPORT);
      privateIpPermission.SetFromPort(MINIONSPORT);
      privateIpPermission.AddIpRanges(privateIp);
      SGConfigReq.AddIpPermissions(privateIpPermission);
    }
  }

  std::cout << "Configuring SG foreach region..." << std::endl;
  for (Region &region : regions) {
    region.ConfigSG(SGConfigReq);
  }
  std::cout << "All SGs configured\n" << std::endl;

  terminateInstances(regions);

  std::cout << "Deleting SG foreach region..." << std::endl;
  for (Region &region : regions) {
    region.DeleteSecurityGroup();
  }
  std::cout << "All SGs deleted\n" << std::endl;

  // Terminate aws sdk
  Aws::ShutdownAPI(options);

  exit(0);
}
