#pragma once

#include <aws/ec2/EC2Client.h> // EC2Client

#include <aws/ec2/model/DescribeImagesRequest.h> // DescribeImagesRequest

#include <aws/ec2/model/CreateSecurityGroupRequest.h> // CreateSecurityGroupRequest

#include <aws/ec2/model/AuthorizeSecurityGroupIngressRequest.h> // AuthorizeSecurityGroupIngressRequest

#include <aws/ec2/model/DeleteSecurityGroupRequest.h> // DeleteSecurityGroupRequest

#include <aws/ec2/model/Instance.h> // Instance

#include <aws/ec2/model/DescribeAvailabilityZonesRequest.h> // DescribeAvailabilityZonesRequest

#include <aws/ec2/model/RunInstancesRequest.h> // RunInstancesRequest

#include <aws/ec2/model/DescribeInstancesRequest.h> // DescribeInstancesRequest
#include <unistd.h>                                 // sleep

#include <aws/ec2/model/TerminateInstancesRequest.h> // TerminateInstancesRequest

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
  void CreateInstances(Aws::EC2::Model::RunInstancesRequest);
  void UpdateInstances(const Aws::EC2::Model::InstanceStateName &,
                       const unsigned int &);
  void TerminateInstances();
};
