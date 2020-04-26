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
  auto len = sizeof(SOCKADDR);

  char* aaa = "123456789ABCDEF";

  sendto(sock, aaa, strlen(aaa), 0, (sockaddr*)&targetAddr, len);
  I_LOG("send data size={}", strlen(aaa));
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
    client.sendData(10);
    timeInSeconds --;
    I_LOG("send data [{}]", timeInSeconds);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(1000));
  client.close();
}