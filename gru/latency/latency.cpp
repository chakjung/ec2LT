#include "latency.h"

#include <netdb.h> // getaddrinfo

#include <unistd.h> // close

#include <stdlib.h> // exit

#include "../database/database.h" // putRttEntry()

#include "../errorCode.h" // MINION*ERRNUM

// Test latency between all AZs
void testLatency(const std::string &tableName, const std::string &statTableName,
                 std::vector<AZ *> &AZs, const int &port, const int &buffSize,
                 const int &delay, const int &trialsCount) {

  std::cout << "Starting latency test ..." << std::endl;
  std::cout << "AZ count: " << AZs.size() << "\n" << std::endl;

  // Hints of Minion connection
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  char portStr[10];
  sprintf(portStr, "%d", port);

  // Minion socket descriptors
  std::vector<int> minionSds;

  for (AZ *&az : AZs) {
    // Result of getting Minion connection info
    struct addrinfo *result;

    // Get Minion connection info
    int getaddrinfoStat = getaddrinfo(az->Instance.GetPublicDnsName().c_str(),
                                      portStr, &hints, &result);
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
      std::cout << "Waiting for " << az->Instance.GetInstanceId()
                << " to come online ..." << std::endl;
      sleep(delay);
    }
    std::cout << "Connected to " << az->Instance.GetInstanceId() << "\n"
              << std::endl;

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

  // DataBase Client
  Aws::Client::ClientConfiguration clientConfig;
  Aws::DynamoDB::DynamoDBClient DBClient(clientConfig);

  // Test 20 times
  for (unsigned int count = 0; count < 20; ++count) {
    // Test all possible Minion combinations
    for (unsigned int src = 0; src < AZs.size(); ++src) {
      for (unsigned int des = 0; des < AZs.size(); ++des) {
        if (src == des) {
          continue;
        }

        std::cout << AZs[src]->AZId << " -> " << AZs[des]->AZId << std::endl;
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
        if (send(minionSds[src], AZs[des]->Instance.GetPublicDnsName().c_str(),
                 AZs[des]->Instance.GetPublicDnsName().length() + 1, 0) == -1) {
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

          Entry &DSTEntry = AZs[src]->DST[AZs[des]->AZId];
          if (firstTest) {
            putDBEntry(DBClient, tableName, AZs[src]->AZId, AZs[des]->AZId,
                       resolveTBuff, handShakeTBuff, utsBuff, rttBuff);

            DSTEntry.AddRES(resolveTBuff);
            DSTEntry.AddHDS(handShakeTBuff);
            DSTEntry.AddRTT(rttBuff);

            firstTest = false;
          } else {
            putDBEntry(DBClient, tableName, AZs[src]->AZId, AZs[des]->AZId,
                       utsBuff, rttBuff);
            DSTEntry.AddRTT(rttBuff);
          }
        }
        std::cout << std::endl;

        putStatEntry(DBClient, statTableName, AZs[src]->AZId, AZs[des]->AZId,
                     AZs[src]->DST[AZs[des]->AZId]);
      }
    }
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
    // Minion didn't reply "BYE"
    if (strcmp(buffer, "BYE") != 0) {
      fprintf(stderr, "Minion didn't reply BYE\n");
      exit(MINIONNOTBYEERRNUM);
    }
  }

  std::cout << "All Minions said bye~\n" << std::endl;

  // Close sockets
  for (int &minionSd : minionSds) {
    close(minionSd);
  }
}
