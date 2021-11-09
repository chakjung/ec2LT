#pragma once

const int MINIONSPORT = 37261;
const int GRUSPORT = MINIONSPORT + 1;

// Communication buffer size (bytes)
const unsigned short BSIZE = 255;
// Wait Minion to come online delay (sec)
const unsigned int CONNECTMINIONDELAY = 5;

// Number of trials per connection
const unsigned char TRIALSPERCONNECTION = 10;
