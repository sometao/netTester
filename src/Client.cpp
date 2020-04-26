#include "Client.h"
#include "seeker/loggerApi.h"
#include <string>
#include <chrono>
#include <thread>

using seeker::SocketUtil;
using std::string;

Client::Client(const string& serverHost, int serverPort)
    : targetHost(serverHost), targetPort(serverPort) {
  // Get socket ready
  if (SocketUtil::startupWSA() == ERR) {
    E_LOG("WSAStartup error.");
    throw std::runtime_error("WSAStartup error.");
  }
  sock = socket(AF_INET, SOCK_DGRAM, 0);

  // init target address
  targetAddr.sin_family = AF_INET;
  SocketUtil::setSocketAddr(&targetAddr, serverHost.c_str());
  targetAddr.sin_port = htons(serverPort);
}


void Client::sendData(size_t num) {
  static auto len = sizeof(SOCKADDR);
  sendto(sock, dataBuf, num, 0, (sockaddr*)&targetAddr, len);
  I_LOG("send data: [{}]", string(dataBuf, num));
}

void Client::readData()
{
  static char recvBuf[4096] = {};
  SOCKADDR_IN remoteAddr;
  size_t addrLen = sizeof(SOCKADDR_IN);

  int recvNum = recvfrom(sock, recvBuf, 4096, 0, (sockaddr*)&remoteAddr, (socklen_t*)&addrLen);
  if (recvNum < 0) {
    auto msg = fmt::format("error: recv_num={}", recvNum);
    E_LOG(msg);
    throw std::runtime_error(msg);
  }
  recvBuf[recvNum] = '\0';

  I_LOG("recv data: [{}]", recvBuf);

}

void Client::close() {
  I_LOG("client finish.");
  SocketUtil::closeSocket(sock);
  SocketUtil::cleanWSA();
}



void startClient(const string& serverHost, int serverPort, int timeInSeconds) {
  I_LOG("Starting send data to {}:{}", serverHost, serverPort);
  Client client(serverHost, serverPort);
  while (timeInSeconds > 0) {
    I_LOG("send data [{}]", timeInSeconds);
    client.sendData(10);
    client.readData();
    timeInSeconds --;
  }
  client.close();
}