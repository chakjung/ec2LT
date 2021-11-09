#include <algorithm> // random_shuffle()

#include <aws/core/Aws.h> // InitAPI(), ShutdownAPI()

#include <aws/core/utils/HashingUtils.h> // HashingUtils

#include "region/region.h" // Region

#include "subroutines/subroutines.h" // describeRegions()
// updateInstances(), describeInstances(), terminateInstances()
// addSGRule()

#include "database/database.h" // createDBTable()

#include "latency/latency.h" // testLatency()

#include "config.h" // TAGKEY
// DEBIAN10AWSACCOUNTID, OSFILTERS
// SECURITYGROUPNAME, SECURITYGROUPDESCRIPTION
// INSTANCETYPE, MINIONSPORT
// UPDATEINSTANCEDELAY

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

  // Limiter
  /**
  std::vector<Region> temp;
  for (Region &region : regions) {
    if (region.RegionName == "us-east-1" || region.RegionName == "eu-west-2" ||
        region.RegionName == "ap-northeast-1") {
      temp.push_back(region);
    }
  }
  regions = temp;
  */

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

  // Minion init script
  const Aws::String MinionInitScript(
      "#!/bin/bash\n"
      "cd /home/admin\n"
      "yes | sudo apt update\n"
      "yes | sudo apt upgrade\n"
      "yes | sudo apt install git\n"
      "yes | git clone https://github.com/chakjung/ec2LT.git\n"
      "cd ec2LT\n"
      "yes | sudo ./minionInit.sh\n");

  Aws::Utils::HashingUtils hashingUtils;
  const Aws::String MinionInitScript64 = hashingUtils.Base64Encode(
      Aws::Utils::ByteBuffer((unsigned char *)MinionInitScript.c_str(),
                             MinionInitScript.length()));

  // Create instance request for t2 instances
  Aws::EC2::Model::RunInstancesRequest INCrtReqT2;
  // Set Instance type
  INCrtReqT2.SetInstanceType(Aws::EC2::Model::InstanceType::t2_micro);
  INCrtReqT2.SetMinCount(1);
  INCrtReqT2.SetMaxCount(1);
  // Create instance request for t3 instances
  Aws::EC2::Model::RunInstancesRequest INCrtReqT3;
  INCrtReqT3.SetInstanceType(Aws::EC2::Model::InstanceType::t3_micro);
  INCrtReqT3.SetMinCount(1);
  INCrtReqT3.SetMaxCount(1);
  // Set tag
  Aws::EC2::Model::TagSpecification INTagSpec;
  INTagSpec.AddTags(tag);
  INTagSpec.SetResourceType(Aws::EC2::Model::ResourceType::instance);
  INCrtReqT2.AddTagSpecifications(INTagSpec);
  INCrtReqT3.AddTagSpecifications(INTagSpec);
  INCrtReqT2.WithUserData(MinionInitScript64);
  INCrtReqT3.WithUserData(MinionInitScript64);

  // Foreach region
  for (Region &region : regions) {
    std::cout << region.RegionName << std::endl;
    region.QueryOSId(OSIdDesReq);
    region.CreateSG(SGCrtReq);

    // Some AZs in these region only support t3
    if (region.RegionName == "us-west-2" || region.RegionName == "af-south-1" ||
        region.RegionName == "eu-north-1" ||
        region.RegionName == "ap-south-1" ||
        region.RegionName == "ap-northeast-3" ||
        region.RegionName == "ap-northeast-2" ||
        region.RegionName == "sa-east-1" ||
        region.RegionName == "ca-central-1" ||
        region.RegionName == "eu-south-1") {
      region.CreateInstances(INCrtReqT3);
    } else {
      region.CreateInstances(INCrtReqT2);
    }

    std::cout << std::endl;
  }

  // Sleep 3hr for Chrony calibration
  sleep(10800);

  std::cout << "Updating instance informations...\n" << std::flush;
  updateInstances(Aws::EC2::Model::InstanceStateName::running, regions,
                  UPDATEINSTANCEDELAY);
  std::cout << "Instance informations updated\n\n" << std::flush;
  describeInstances(regions);

  // Config SG request
  Aws::EC2::Model::AuthorizeSecurityGroupIngressRequest SGConfigReq;
  SGConfigReq.SetGroupName(SECURITYGROUPNAME);

  // Add Gru's ips
  addSGRule(SGConfigReq, GRUPUBLICIP, "tcp", GRUSPORT, GRUSPORT);
  addSGRule(SGConfigReq, GRUPRIVATEIP, "tcp", GRUSPORT, GRUSPORT);

  for (Region &region : regions) {
    for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
         region.Instances) {

      // Add Minion's ips
      addSGRule(SGConfigReq, instance.second.GetPublicIpAddress() + "/32",
                "tcp", MINIONSPORT, GRUSPORT);
      addSGRule(SGConfigReq, instance.second.GetPrivateIpAddress() + "/32",
                "tcp", MINIONSPORT, GRUSPORT);
    }
  }

  std::cout << "Configuring SG foreach region..." << std::endl;
  for (Region &region : regions) {
    std::cout << region.RegionName << "\n" << std::flush;
    region.ConfigSG(SGConfigReq);
  }
  std::cout << "All SGs configured\n" << std::endl;

  // EC2 Latency Test - proj specific start
  Aws::Client::ClientConfiguration DBClientConfig;
  Aws::DynamoDB::DynamoDBClient DBClient(DBClientConfig);
  createDBTable(DBClient, DBTABLENAME, UPDATEDBTABLEDELAY);

  // Gather all instances
  std::vector<std::pair<Aws::String, Aws::EC2::Model::Instance> *> instances;
  for (Region &region : regions) {
    for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
         region.Instances) {
      instances.push_back(&instance);
    }
  }
  std::random_shuffle(instances.begin(), instances.end());

  testLatency(DBClient, DBTABLENAME, instances, GRUSPORT, BSIZE,
              CONNECTMINIONDELAY, TRIALSPERCONNECTION);
  // EC2 Latency Test - proj specific end

  std::cout << "Terminating all instances..." << std::endl;
  terminateInstances(regions);
  updateInstances(Aws::EC2::Model::InstanceStateName::terminated, regions,
                  UPDATEINSTANCEDELAY);
  std::cout << "All instances terminated\n" << std::endl;

  std::cout << "Deleting SG foreach region..." << std::endl;
  for (Region &region : regions) {
    region.DeleteSG();
  }
  std::cout << "All SGs deleted\n" << std::endl;

  // Terminate aws sdk
  Aws::ShutdownAPI(options);

  exit(0);
}
