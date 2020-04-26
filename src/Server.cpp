#include "config.h"
#include "Server.h"
#include <iostream>
#include <string>

using seeker::SocketUtil;
using std::cout;
using std::endl;
using std::string;

Server::Server(int p) : port(p) {
  D_LOG("init server on port[{}]", port);

  if (SocketUtil::startupWSA() == ERR) {
    cout << "WSAStartup error." << endl;
    throw std::runtime_error("WSAStartup error.");
  }

  sock = socket(AF_INET, SOCK_DGRAM, 0);

  sockaddr_in serverAddr = {0};
  SocketUtil::setSocketAddr(&serverAddr, "0.0.0.0");
  serverAddr.sin_family = AF_INET;
  serverAddr.sin_port = htons(port);

  if (bind(sock, (sockaddr*)&serverAddr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
    SocketUtil::closeSocket(sock);
    SocketUtil::cleanWSA();
    cout << "bind error." << endl;
    throw std::runtime_error("bind error.");
  }

  D_LOG("server socket bind port[{}] success: {}", port, sock);

}

void Server::start() {
  SOCKADDR_IN peerAddr;
  size_t addrLen = sizeof(SOCKADDR_IN);

  char recvBuf[4096] = {};

  int c = 0;
  while (c < 10) {
    cout << "waiting data..." << endl;
    int recvNum = recvfrom(sock, recvBuf, 4096, 0, (sockaddr*)&peerAddr, (socklen_t*)&addrLen);
    if (recvNum < 0) {
      auto msg = fmt::format("error: recv_num={}", recvNum);
      E_LOG(msg);
      throw std::runtime_error(msg);
    }

    sendto(sock, recvBuf, recvNum, 0, (sockaddr*)&peerAddr, addrLen);

    c += 1;
    recvBuf[recvNum] = '\0';
    I_LOG("[\t{}\t]: recv [{} byte] from [{}]: [{}]", c, recvNum, inet_ntoa(peerAddr.sin_addr),
          recvBuf);
  }
}

void Server::close() {
  I_LOG("Server finish.");
  SocketUtil::closeSocket(sock);
  SocketUtil::cleanWSA();
}



void startServer(int port) {
  I_LOG("Starting server on port={}", port);
  Server server{port};
  server.start();
  server.close();
}

