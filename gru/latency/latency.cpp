#include "latency.h"

#include <netdb.h> // getaddrinfo

#include <unistd.h> // close

#include <stdlib.h> // exit

#include "../database/database.h" // putRttEntry()

#include "../errorCode.h" // MINION*ERRNUM

// Test latency between all instances
void testLatency(
    Aws::DynamoDB::DynamoDBClient &dbClient, const std::string &tableName,
    std::vector<std::pair<Aws::String, Aws::EC2::Model::Instance> *> &instances,
    const int &port, const int &buffSize, const int &delay,
    const int &trialsCount) {

  std::cout << "Starting latency test ...\n" << std::endl;

  // Hints of Minion connection
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  char portStr[10];
  sprintf(portStr, "%d", port);

  // Minion socket descriptors
  std::vector<int> minionSds;

  for (std::pair<Aws::String, Aws::EC2::Model::Instance> *&instance :
       instances) {
    // Result of getting Minion connection info
    struct addrinfo *result;

    // Get Minion connection info
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
  char buffer[buffSize];

  // First test in this combination
  bool firstTest;

  char resolveTBuff[buffSize];
  char handShakeTBuff[buffSize];
  char utsBuff[buffSize];
  char rttBuff[buffSize];

  // Wait for "PROCEED" from Minion
  for (int &minionSd : minionSds) {
    if (recv(minionSd, buffer, buffSize, 0) == -1) {
      perror("recv");
      exit(MINIONRECVERRNUM);
    }
    // Minion not reply "PROCEED"
    if (strcmp(buffer, "PROCEED") != 0) {
      fprintf(stderr, "Minion not PROCEED\n");
      exit(MINIONNOTPROCEEDERRNUM);
    }
  }

  std::cout << "All Minions agree to proceed\n" << std::endl;

  // Test 20 times
  for (unsigned int count = 0; count < 20; ++count) {
    // Test all possible Minion combinations
    for (unsigned int src = 0; src < instances.size(); ++src) {
      for (unsigned int des = 0; des < instances.size(); ++des) {
        if (src == des) {
          continue;
        }

        std::cout << instances[src]->first << " -> " << instances[des]->first
                  << std::endl;
        firstTest = true;

        // Assign roles
        if (send(minionSds[src], "SRC", 4, 0) == -1) {
          perror("send");
          exit(MINIONSENDERRNUM);
        }
        if (send(minionSds[des], "DES", 4, 0) == -1) {
          perror("send");
          exit(MINIONSENDERRNUM);
        }
        // Verify roles
        if (recv(minionSds[src], buffer, buffSize, 0) == -1) {
          perror("recv");
          exit(MINIONRECVERRNUM);
        }
        if (strcmp(buffer, "SRC") != 0) {
          fprintf(stderr, "SRC role unmatch\n");
          exit(MINIONROLEERRNUM);
        }
        if (recv(minionSds[des], buffer, buffSize, 0) == -1) {
          perror("recv");
          exit(MINIONRECVERRNUM);
        }
        if (strcmp(buffer, "DES") != 0) {
          fprintf(stderr, "DES role unmatch\n");
          exit(MINIONROLEERRNUM);
        }

        std::cout << "Roles assigned" << std::endl;

        // Send DES PublicDnsName to SRC
        if (send(minionSds[src],
                 instances[des]->second.GetPublicDnsName().c_str(),
                 instances[des]->second.GetPublicDnsName().length() + 1,
                 0) == -1) {
          perror("send");
          exit(MINIONSENDERRNUM);
        }

        // Get DNS resolution time
        if (recv(minionSds[src], resolveTBuff, buffSize, 0) == -1) {
          perror("recv");
          exit(MINIONRECVERRNUM);
        }
        std::cout << "DNS resolution time: " << resolveTBuff << std::endl;

        // Ask for TCP handshake time
        if (send(minionSds[src], "TCP HANDSHAKE TIME", 19, 0) == -1) {
          perror("send");
          exit(MINIONSENDERRNUM);
        }
        // Get TCP handshake time
        if (recv(minionSds[src], handShakeTBuff, buffSize, 0) == -1) {
          perror("recv");
          exit(MINIONRECVERRNUM);
        }
        std::cout << "TCP  handshake time: " << handShakeTBuff << std::endl;

        std::cout << "UnixTimestamp, RTT" << std::endl;
        for (unsigned char i = 0; i < trialsCount; ++i) {
          // Ask for UnixTimestamp
          if (send(minionSds[src], "UTS", 4, 0) == -1) {
            perror("send");
            exit(MINIONSENDERRNUM);
          }
          // Get UnixTimestamp
          if (recv(minionSds[src], utsBuff, buffSize, 0) == -1) {
            perror("recv");
            exit(MINIONRECVERRNUM);
          }
          std::cout << utsBuff << ", ";

          // Ask for RTT
          if (send(minionSds[src], "RTT", 4, 0) == -1) {
            perror("send");
            exit(MINIONSENDERRNUM);
          }
          // Get RTT
          if (recv(minionSds[src], rttBuff, buffSize, 0) == -1) {
            perror("recv");
            exit(MINIONRECVERRNUM);
          }
          std::cout << rttBuff << std::endl;

          if (firstTest) {
            putRttEntry(dbClient, tableName, instances[src]->first,
                        instances[des]->first, resolveTBuff, handShakeTBuff,
                        utsBuff, rttBuff);
            firstTest = false;
          } else {
            putRttEntry(dbClient, tableName, instances[src]->first,
                        instances[des]->first, utsBuff, rttBuff);
          }
        }
        std::cout << std::endl;
      }
    }

    // Sleep 20min between each round
    sleep(1200);
  }

  // Inform Minions test has ended
  for (int &minionSd : minionSds) {
    if (send(minionSd, "BYE", 4, 0) == -1) {
      perror("send");
      exit(MINIONSENDERRNUM);
    }
  }
  // Wait for "BYE" from Minion
  for (int &minionSd : minionSds) {
    if (recv(minionSd, buffer, buffSize, 0) == -1) {
      perror("recv");
      exit(MINIONRECVERRNUM);
    }
    // Minion not reply "BYE"
    if (strcmp(buffer, "BYE") != 0) {
      fprintf(stderr, "Minion not BYE\n");
      exit(MINIONNOTBYEERRNUM);
    }
  }

  std::cout << "All Minions said bye~\n" << std::endl;

  // Close sockets
  for (int &minionSd : minionSds) {
    close(minionSd);
  }
}
