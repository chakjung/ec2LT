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
    exit(3);
  }

  // Specify Gru port
  saddrIn->sin_port = htons(GRUSPORT);
  // Bind socket to addr
  if (bind(gruInSd, (struct sockaddr *)saddrIn, saddrInLen) == -1) {
    perror("bind");
    exit(4);
  }

  // Specify Minion port
  saddrIn->sin_port = htons(MINIONSPORT);
  // Bind socket to addr
  if (bind(minionInSd, (struct sockaddr *)saddrIn, saddrInLen) == -1) {
    perror("bind");
    exit(4);
  }

  // Mark socket as passive
  // Only allow 1 connection (0 backlog)
  if (listen(gruInSd, 0) == -1) {
    perror("listen");
    exit(5);
  }
  if (listen(minionInSd, 0) == -1) {
    perror("listen");
    exit(5);
  }

  // Incoming Gru (Socket descriptor)
  struct sockaddr gru;
  socklen_t gruLen = sizeof(gru);
  int gruSd = accept(gruInSd, &gru, &gruLen);
  if (gruSd == -1) {
    perror("accept");
    exit(6);
  }

  close(gruInSd);

  // Communication buffer
  char buffer[BSIZE];

  // Receive peer name
  if (recv(gruSd, buffer, BSIZE, 0) == -1) {
    perror("recv");
    exit(7);
  }

  // Not connected to Gru
  if (strcmp(buffer, "GRU") != 0) {
    fprintf(stderr, "Fail to connect to Gru\n");
    exit(8);
  }

  // Send "PROCEED"
  if (send(gruSd, "PROCEED", 8, 0) == -1) {
    perror("send");
    exit(9);
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
      exit(10);
    }
    // Echo role
    if (send(gruSd, buffer, recvCCount, 0) == -1) {
      perror("send");
      exit(11);
    }

    if (strcmp(buffer, "SRC") == 0) {
      // Recv DES PublicDnsName
      if (recv(gruSd, buffer, BSIZE, 0) == -1) {
        perror("recv");
        exit(12);
      }
      // Get Minion connection info
      int getaddrinfoStat =
          getaddrinfo(buffer, minionsPortStr, &hints, &result);
      if (getaddrinfoStat != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(getaddrinfoStat));
        exit(13);
      }
      // Fail to get Minion info
      if (result == NULL) {
        fprintf(stderr, "Could not get Minion connection info\n");
        exit(14);
      }

      // Socket descriptor (IPv4, TCP, Minion connection protocol)
      int desSd = socket(AF_INET, SOCK_STREAM, result->ai_protocol);
      if (desSd == -1) {
        perror("socket");
        exit(15);
      }

      // Connect to Minion
      while (connect(desSd, result->ai_addr, result->ai_addrlen) == -1) {
        sleep(CONNECTMINIONDELAY);
      }

      // Free Minion connection info
      freeaddrinfo(result);

      if (send(gruSd, "result", 7, 0) == -1) {
        perror("send");
        exit(4321);
      }

      close(desSd);

    } else if (strcmp(buffer, "DES") == 0) {

      // Incoming src (Socket descriptor)
      struct sockaddr src;
      socklen_t srcLen = sizeof(src);
      int srcSd = accept(minionInSd, &src, &srcLen);
      if (srcSd == -1) {
        perror("accept");
        exit(19);
      }

      close(srcSd);

    } else if (strcmp(buffer, "BYE") == 0) {
      break;
    } else {
      fprintf(stderr, "Unknown role\n");
      exit(20);
    }
  }

  close(minionInSd);
  close(gruSd);

  exit(0);
}
