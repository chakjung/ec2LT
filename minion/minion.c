/**
 * EC2 network latency project
 *
 * File: minion.c
 *
 * Author: Jung Chak
 *
 * Minion task~
 */

#include <ifaddrs.h> // getifaddrs
#include <net/if.h>  // IFF_*

#include <netdb.h> // getaddrinfo

#include <sys/socket.h> // socket
#include <unistd.h>     // close

#include <arpa/inet.h> // inet_ntop

#include <string.h> // strcmp

#include <stdio.h> // printf

#include <errno.h>  // perror
#include <stdlib.h> // exit

#include "config.h" // MINIONSPORT, GRUSPORT
#include "timer.h"  // stopwatch

int main() {
  // Get network interfaces info
  struct ifaddrs *ifaddr, *ifa;
  struct sockaddr_in *saddrIn;
  socklen_t saddrInLen;
  if (getifaddrs(&ifaddr) == -1 || ifaddr == NULL) {
    perror("getifaddrs");
    exit(1);
  }

  // Set readhead
  ifa = ifaddr;

  while (true) {

    // IPv4 && !loopback && (up && broadcast && running)
    if (ifa->ifa_addr->sa_family == AF_INET &&
        !(ifa->ifa_flags & IFF_LOOPBACK) &&
        (ifa->ifa_flags & (IFF_UP | IFF_BROADCAST | IFF_RUNNING))) {

      // sockaddr to sockaddr_in
      saddrIn = (struct sockaddr_in *)ifa->ifa_addr;

      // !linklocal
      if ((saddrIn->sin_addr.s_addr & 0x0000ffff) != 0x0000fea9) {

        // Addr found
        saddrInLen = sizeof(*saddrIn); // Set size
        ifa = NULL;                    // Release pointer
        break;                         // Break loop
      }
    }

    // No IPv4 addr available
    if (ifa->ifa_next == NULL) {
      fprintf(stderr, "No IPv4 addr available\n");
      exit(2);
    }

    ifa = ifa->ifa_next;
  }

  // Free network interfaces info
  freeifaddrs(ifaddr);

  // Socket descriptor (IPv4, TCP, default)
  int gruInSd = socket(AF_INET, SOCK_STREAM, 0);
  if (gruInSd == -1) {
    perror("socket");
    exit(3);
  }
  int minionInSd = socket(AF_INET, SOCK_STREAM, 0);
  if (minionInSd == -1) {
    perror("socket");
    exit(4);
  }

  // Specify Gru port
  saddrIn->sin_port = htons(GRUSPORT);
  // Bind socket to addr
  if (bind(gruInSd, (struct sockaddr *)saddrIn, saddrInLen) == -1) {
    perror("bind");
    exit(5);
  }

  // Specify Minion port
  saddrIn->sin_port = htons(MINIONSPORT);
  // Bind socket to addr
  if (bind(minionInSd, (struct sockaddr *)saddrIn, saddrInLen) == -1) {
    perror("bind");
    exit(6);
  }

  // Mark socket as passive
  // Only allow 1 connection (0 backlog)
  if (listen(gruInSd, 0) == -1) {
    perror("listen");
    exit(7);
  }
  if (listen(minionInSd, 0) == -1) {
    perror("listen");
    exit(8);
  }

  // Incoming Gru (Socket descriptor)
  struct sockaddr gru;
  socklen_t gruLen = sizeof(gru);
  int gruSd = accept(gruInSd, &gru, &gruLen);
  if (gruSd == -1) {
    perror("accept");
    exit(9);
  }

  close(gruInSd);

  // Communication buffer
  char buffer[BSIZE];

  // Receive peer name
  if (recv(gruSd, buffer, BSIZE, 0) == -1) {
    perror("recv");
    exit(10);
  }

  // Not connected to Gru
  if (strcmp(buffer, "GRU") != 0) {
    fprintf(stderr, "Fail to connect to Gru\n");
    exit(11);
  }

  // Send "PROCEED"
  if (send(gruSd, "PROCEED", 8, 0) == -1) {
    perror("send");
    exit(12);
  }

  // Amount of char receive
  short recvCCount;

  // Hints of Minion connection
  struct addrinfo hints;
  memset(&hints, 0, sizeof(struct addrinfo));
  hints.ai_family = AF_INET;       // IPv4
  hints.ai_socktype = SOCK_STREAM; // TCP
  char minionsPortStr[10];
  sprintf(minionsPortStr, "%d", MINIONSPORT);

  // Result of getting Minion connection info
  struct addrinfo *result;

  while (true) {
    // Receive role
    recvCCount = recv(gruSd, buffer, BSIZE, 0);
    if (recvCCount == -1) {
      perror("recv");
      exit(13);
    }
    // Echo role
    if (send(gruSd, buffer, recvCCount, 0) == -1) {
      perror("send");
      exit(14);
    }

    if (strcmp(buffer, "SRC") == 0) {
      // Recv DES PublicDnsName
      if (recv(gruSd, buffer, BSIZE, 0) == -1) {
        perror("recv");
        exit(15);
      }

      // DNS resolution time stopwatch
      struct timespec resolveStart, resolveStop;

      // Start the stopwatch
      if (clock_gettime(CLOCK_BOOTTIME, &resolveStart) == -1) {
        perror("clock_gettime");
        exit(16);
      }

      // Get Minion connection info
      int getaddrinfoStat =
          getaddrinfo(buffer, minionsPortStr, &hints, &result);
      if (getaddrinfoStat != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoStat));
        exit(17);
      }

      // Stop the stopwatch
      if (clock_gettime(CLOCK_BOOTTIME, &resolveStop) == -1) {
        perror("clock_gettime");
        exit(18);
      }

      // Fail to get Minion info
      if (result == NULL) {
        fprintf(stderr, "Could not get Minion connection info\n");
        exit(19);
      }

      // Socket descriptor (IPv4, TCP, Minion connection protocol)
      int desSd = socket(AF_INET, SOCK_STREAM, result->ai_protocol);
      if (desSd == -1) {
        perror("socket");
        exit(20);
      }

      // TCP handshake time stopwatch
      struct timespec handshakeStart, handshakeStop;

      // Start the stopwatch
      if (clock_gettime(CLOCK_BOOTTIME, &handshakeStart) == -1) {
        perror("clock_gettime");
        exit(21);
      }

      // Connect to Minion
      while (connect(desSd, result->ai_addr, result->ai_addrlen) == -1) {
        sleep(CONNECTMINIONDELAY);

        // Reset the stopwatch
        if (clock_gettime(CLOCK_BOOTTIME, &handshakeStart) == -1) {
          perror("clock_gettime");
          exit(22);
        }
      }

      // Stop the stopwatch
      if (clock_gettime(CLOCK_BOOTTIME, &handshakeStop) == -1) {
        perror("clock_gettime");
        exit(23);
      }

      // Free Minion connection info
      freeaddrinfo(result);

      // DNS resolution time to buffer
      int bufferUsed = snprintf(buffer, BSIZE, "%llu",
                                calcIntervalNS(resolveStart, resolveStop));
      if (bufferUsed < 0 || bufferUsed >= BSIZE) {
        perror("snprintf");
        exit(24);
      }

      // Send DNS resolution time
      if (send(gruSd, buffer, bufferUsed + 1, 0) == -1) {
        perror("send");
        exit(25);
      }

      // Receive request
      if (recv(gruSd, buffer, BSIZE, 0) == -1) {
        perror("recv");
        exit(26);
      }

      // TCP handshake time not requested
      if (strcmp(buffer, "TCP HANDSHAKE TIME") != 0) {
        fprintf(stderr, "TCP handshake time not requested\n");
        exit(27);
      }

      // TCP handshake time to buffer
      bufferUsed = snprintf(buffer, BSIZE, "%llu",
                            calcIntervalNS(handshakeStart, handshakeStop));
      if (bufferUsed < 0 || bufferUsed >= BSIZE) {
        perror("snprintf");
        exit(28);
      }

      // Send TCP handshake time
      if (send(gruSd, buffer, bufferUsed + 1, 0) == -1) {
        perror("send");
        exit(29);
      }

      struct timespec wallTime;
      struct timespec rttStart, rttStop;
      char echoBuffer[BSIZE];

      for (unsigned char i = 0; i < TRIALSPERCONNECTION; ++i) {
        // Payload to buffer
        bufferUsed = snprintf(buffer, BSIZE, "%d", i);
        if (bufferUsed < 0 || bufferUsed >= BSIZE) {
          perror("snprintf");
          exit(30);
        }

        // Get wall time
        if (clock_gettime(CLOCK_REALTIME, &wallTime) == -1) {
          perror("clock_gettime");
          exit(31);
        }

        // Start the stopwatch
        if (clock_gettime(CLOCK_BOOTTIME, &rttStart) == -1) {
          perror("clock_gettime");
          exit(32);
        }

        // Send payload
        if (send(desSd, buffer, bufferUsed + 1, 0) == -1) {
          perror("send");
          exit(33);
        }

        // Wait for echo from client
        if (recv(desSd, echoBuffer, BSIZE, 0) == -1) {
          perror("recv");
          exit(34);
        }

        // Stop the stopwatch
        if (clock_gettime(CLOCK_BOOTTIME, &rttStop) == -1) {
          perror("clock_gettime");
          exit(35);
        }

        // Compare echo with payload
        if (strcmp(buffer, echoBuffer) != 0) {
          fprintf(stderr, "Incorrect echo\n");
          exit(36);
        }

        // Receive request
        if (recv(gruSd, buffer, BSIZE, 0) == -1) {
          perror("recv");
          exit(37);
        }

        // UnixTimestamp not requested
        if (strcmp(buffer, "UTS") != 0) {
          fprintf(stderr, "UnixTimestamp not requested\n");
          exit(38);
        }

        // UnixTimestamp to buffer
        bufferUsed = snprintf(buffer, BSIZE, "%llu", calcTimestampNS(wallTime));
        if (bufferUsed < 0 || bufferUsed >= BSIZE) {
          perror("snprintf");
          exit(39);
        }

        // Send UnixTimestamp
        if (send(gruSd, buffer, bufferUsed + 1, 0) == -1) {
          perror("send");
          exit(40);
        }

        // Receive request
        if (recv(gruSd, buffer, BSIZE, 0) == -1) {
          perror("recv");
          exit(41);
        }

        // RTT not requested
        if (strcmp(buffer, "RTT") != 0) {
          fprintf(stderr, "RoundTripTime not requested\n");
          exit(42);
        }

        // RTT to buffer
        bufferUsed =
            snprintf(buffer, BSIZE, "%llu", calcIntervalNS(rttStart, rttStop));
        if (bufferUsed < 0 || bufferUsed >= BSIZE) {
          perror("snprintf");
          exit(43);
        }

        // Send RTT
        if (send(gruSd, buffer, bufferUsed + 1, 0) == -1) {
          perror("send");
          exit(44);
        }
      }

      close(desSd);

    } else if (strcmp(buffer, "DES") == 0) {

      // Incoming src (Socket descriptor)
      struct sockaddr src;
      socklen_t srcLen = sizeof(src);
      int srcSd = accept(minionInSd, &src, &srcLen);
      if (srcSd == -1) {
        perror("accept");
        exit(45);
      }

      // Start RTT test
      ssize_t recvCCount;
      for (unsigned char i = 0; i < TRIALSPERCONNECTION; ++i) {
        // Recv payload
        recvCCount = recv(srcSd, buffer, BSIZE, 0);
        if (recvCCount == -1) {
          perror("recv");
          exit(46);
        }
        // Echo payload to SRC
        if (send(srcSd, buffer, recvCCount, 0) == -1) {
          perror("send");
          exit(47);
        }
      }

      close(srcSd);

    } else if (strcmp(buffer, "BYE") == 0) {
      break;
    } else {
      fprintf(stderr, "Unknown role\n");
      exit(48);
    }
  }

  close(minionInSd);
  close(gruSd);

  exit(0);
}
