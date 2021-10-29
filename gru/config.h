#pragma once

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

// Security Group name
const char SECURITYGROUPNAME[] = "EC2LT";
