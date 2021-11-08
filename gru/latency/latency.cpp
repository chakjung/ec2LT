#include "latency.h"

#include <netdb.h> // getaddrinfo

#include <unistd.h> // close

#include <stdlib.h> // exit

#include "../errorCode.h" // MINION*ERRNUM

// Test latency between all instances
void testLatency(
    Aws::DynamoDB::DynamoDBClient &dbClient,
    std::vector<std::pair<Aws::String, Aws::EC2::Model::Instance> *> &instances,
    const int &port, const int &buffSize, const int &delay) {

  std::cout << "Starting latency test ...\n" << std::endl;

  // Hints of Minion connection
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // Minion socket descriptors
  std::vector<int> minionSds;

  for (std::pair<Aws::String, Aws::EC2::Model::Instance> *&instance :
       instances) {
    // Result of getting Minion connection info
    struct addrinfo *result;

    // Get Minion connection info
    char portStr[10];
    sprintf(portStr, "%d", port);
    int getaddrinfoStat = getaddrinfo(
        instance->second.GetPublicDnsName().c_str(), portStr, &hints, &result);
    if (getaddrinfoStat != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoStat));
      exit(MINIONCONNECTIONERRNUM);
    }

    // Fail to get Minion info
    if (result == NULL) {
      fprintf(stderr, "Could not get Minion connection info\n");
      exit(MINIONCONNECTIONERRNUM);
    }

    // Socket descriptor (IPv4, TCP, Minion connection protocol)
    int sd = socket(AF_INET, SOCK_STREAM, result->ai_protocol);
    if (sd == -1) {
      perror("socket");
      exit(MINIONCONNECTIONERRNUM);
    }

    // Connect to Minion
    while (connect(sd, result->ai_addr, result->ai_addrlen) == -1) {
      std::cout << "Waiting for " << instance->second.GetInstanceId()
                << " to come online ...\n"
                << std::flush;
      sleep(delay);
    }
    std::cout << "Connected to " << instance->second.GetInstanceId() << "\n\n"
              << std::flush;

    minionSds.push_back(sd);

    // Free Minion connection info
    freeaddrinfo(result);
  }

  // Inform Minions has connected to Gru
  for (int &minionSd : minionSds) {
    if (send(minionSd, "GRU", 4, 0) == -1) {
      perror("send");
      exit(MINIONSENDERRNUM);
    }
  }

  // Communication buffer
  char buffer[BSIZE];

  // Wait for "PROCEED" from Minion
  for (int &minionSd : minionSds) {
    if (recv(minionSd, buffer, BSIZE, 0) == -1) {
      perror("recv");
      exit(MINIONRECVERRNUM);
    }
    // Minion not reply "PROCEED"
    if (strcmp(buffer, "PROCEED") != 0) {
      fprintf(stderr, "Minion not PROCEED\n");
      exit(MINIONNOTPROCEEDERRNUM);
    }

    std::cout << "Got PROCEED\n" << std::endl;
  }
}
