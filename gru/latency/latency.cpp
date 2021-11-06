#include "latency.h"

#include "../errorCode.h" // MINIONCONNECTIONERRNUM

// Test latency between all instances
void testLatency(
    Aws::DynamoDB::DynamoDBClient &dbClient,
    std::vector<std::pair<Aws::String, Aws::EC2::Model::Instance> &> &instances,
    const int &port, const int &buffSize, const int &delay) {

  // Hints of Minion connection
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP

  // Socket descriptors
  std::vector<int> sds;

  for (std::pair<Aws::String, Aws::EC2::Model::Instance> &instance :
       instances) {
    // Result of getting Minion connection info
    struct addrinfo *result;

    // Get Minion connection info
    int getaddrinfoStat =
        getaddrinfo(instance.second.GetPublicDnsName(), port, &hints, &result);
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
      std::cout << "zzz...\n" << std::flush;
      sleep(delay);
    }
    std::cout << "connected\n" << std::flush;

    sds.push_back(sd);

    // Free Minion connection info
    freeaddrinfo(result);
  }
}
