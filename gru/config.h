#pragma once

#include <aws/ec2/model/InstanceType.h> // InstanceType

// Tag key
const char TAGKEY[] = "EC2LT";

// Debian 10 Buster AWS account ID
// Official page: https://wiki.debian.org/Cloud/AmazonEC2Image/Buster
const Aws::String DEBIAN10AWSACCOUNTID("136693071363");

// OS filters
const std::vector<std::pair<std::string, std::string>> OSFILTERS =
    std::vector<std::pair<std::string, std::string>>{
        std::pair<std::string, std::string>("architecture", "x86_64"),
        std::pair<std::string, std::string>("is-public", "true"),
        std::pair<std::string, std::string>("name", "debian-10-amd64*"),
        std::pair<std::string, std::string>("root-device-type", "ebs"),
        std::pair<std::string, std::string>("state", "available"),
        std::pair<std::string, std::string>("virtualization-type", "hvm"),
    };

// Security Group
const char SECURITYGROUPNAME[] = "EC2LT";
const char SECURITYGROUPDESCRIPTION[] = "SG for EC2LT";

const Aws::String GRUPUBLICIP = "3.17.62.197/32";
const Aws::String GRUPRIVATEIP = "172.31.33.96/32";

const int MINIONSPORT = 37261;
const int GRUSPORT = MINIONSPORT + 1;

// Update instance loop delay (sec)
const unsigned int UPDATEINSTANCEDELAY = 5;

// DB table name
const std::string DBTABLENAME("EC2LT");
// Update DB table loop delay (sec)
const unsigned int UPDATEDBTABLEDELAY = 5;

// Wait Minion to come online delay (sec)
const unsigned int CONNECTMINIONDELAY = 15;
// Communication buffer size (bytes)
const unsigned short BSIZE = 255;
// Number of trials per connection
const unsigned char TRIALSPERCONNECTION = 10;
