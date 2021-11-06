#!/bin/bash
sudo apt install git
sudo apt install make
sudo apt install g++
sudo apt install chrony

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

cd minion
make -f minion.makefile
./minion
