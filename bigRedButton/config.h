/**
 * EC2 network latency project
 *
 * File: config.h
 *
 * Author: Jung Chak
 *
 * BigRedButton configuration
 */

#pragma once

// Whitelisted from termination
const std::set<std::string> INSTANCEWHITELIST = {};

// Whitelisted from deletion
const std::set<std::string> SGWHITELIST = {};

// Update instance loop delay (sec)
const unsigned int UPDATEINSTANCEDELAY = 3;
