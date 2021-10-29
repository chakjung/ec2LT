#!/bin/bash
sudo apt install git
sudo apt install make
sudo apt install g++
sudo apt install chrony
sudo apt install cmake
sudo apt install libcurl4-openssl-dev
sudo apt install libssl-dev
sudo apt install uuid-dev
sudo apt install zlib1g-dev
sudo apt install libpulse-dev

sudo apt update
sudo apt upgrade
sudo apt full-upgrade
sudo apt dist-upgrade

sudo apt update
sudo apt upgrade
sudo apt full-upgrade
sudo apt dist-upgrade

echo ""
echo ""
sudo systemctl start chrony

cd ..
git clone --recurse-submodules https://github.com/aws/aws-sdk-cpp
mkdir sdk_build
cd sdk_build
cmake ../aws-sdk-cpp/ -DCMAKE_BUILD_TYPE=Release -DBUILD_ONLY="dynamodb;ec2"
make
sudo make install
cd ..
