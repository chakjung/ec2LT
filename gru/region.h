#pragma once

#include <aws/ec2/EC2Client.h> // EC2Client

#include <aws/ec2/model/DescribeImagesRequest.h> // DescribeImagesRequest

#include <aws/ec2/model/CreateSecurityGroupRequest.h> // CreateSecurityGroupRequest

#include <aws/ec2/model/AuthorizeSecurityGroupIngressRequest.h> // AuthorizeSecurityGroupIngressRequest

#include <aws/ec2/model/Instance.h> // Instance

class Region {
public:
  Aws::String RegionName;
  Aws::EC2::EC2Client RegionalClient;

  Aws::String OSId;
  void QueryOSId(const Aws::EC2::Model::DescribeImagesRequest &);

  Aws::String SGId;
  void CreateSG(const Aws::EC2::Model::CreateSecurityGroupRequest &);
  void ConfigSG(const Aws::EC2::Model::AuthorizeSecurityGroupIngressRequest &);
  void DeleteSG();

  std::vector<std::pair<Aws::String, Aws::EC2::Model::Instance>> Instances;
  void UpdateInstances(const Aws::EC2::Model::InstanceStateName &);
  void TerminateInstances();
}
