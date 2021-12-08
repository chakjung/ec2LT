/**
 * EC2 network latency project
 *
 * File: config.h
 *
 * Author: Jung Chak
 *
 * Minion task configuration
 */

#pragma once

// TCP connection ports
const int MINIONSPORT = 37261;
const int GRUSPORT = MINIONSPORT + 1;

// Communication buffer size (bytes)
const unsigned short BSIZE = 255;

// Wait Minion to come online delay (sec)
const unsigned int CONNECTMINIONDELAY = 5;

// Number of trials per connection
const unsigned char TRIALSPERCONNECTION = 10;
