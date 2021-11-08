#include <ifaddrs.h> // getifaddrs
#include <net/if.h>  // IFF_*

#include <sys/socket.h> // socket
#include <unistd.h>     // close

#include <arpa/inet.h> // inet_ntop
#include <stdio.h>     // printf

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
  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd == -1) {
    perror("socket");
    exit(3);
  }

  // Specify Gru port
  saddrIn->sin_port = htons(GRUSPORT);
  // Bind socket to addr
  if (bind(sd, (struct sockaddr *)saddrIn, saddrInLen) == -1) {
    perror("bind");
    exit(4);
  }

  // Mark socket as passive
  // Only allow 1 connection (0 backlog)
  if (listen(sd, 0) == -1) {
    perror("listen");
    exit(5);
  }

  // Get socket info (addr, port, etc.)
  if (getsockname(sd, (struct sockaddr *)saddrIn, &saddrInLen) == -1) {
    perror("getsockname");
    exit(6);
  }

  // addr to str
  char addrStr[INET_ADDRSTRLEN];
  if (inet_ntop(AF_INET, &(saddrIn->sin_addr), addrStr, INET_ADDRSTRLEN) ==
      NULL) {
    perror("inet_ntop");
    exit(7);
  }
  printf("%s %d\n", addrStr, ntohs(saddrIn->sin_port));

  // Incoming Gru (Socket descriptor)
  struct sockaddr gru;
  socklen_t gruLen = sizeof(gru);
  int grusd = accept(sd, &gru, &gruLen);
  if (grusd == -1) {
    perror("accept");
    exit(8);
  }

  while (true) {
    sleep(10);
  }
}
