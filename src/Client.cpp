#include "Client.h"
#include "seeker/loggerApi.h"
#include <string>
#include <chrono>
#include <thread>

using seeker::SocketUtil;
using std::string;

Client::Client(const string& serverHost, int serverPort) {
  // Get socket ready
  SocketUtil::startupWSA();

  conn.setRemoteIp(serverHost);
  conn.setRemotePort(serverPort);

  conn.init();

}


void Client::sendData(size_t num) {
  conn.sendData(dataBuf, num);
  I_LOG("send data: [{}]", string(dataBuf, num));
}

void Client::readData()
{
  static char recvBuf[4096] = {};
  int recvNum = conn.recvData(recvBuf, 4096);
  recvBuf[recvNum] = '\0';

  I_LOG("recv data: [{}]", recvBuf);

}

void Client::close() {
  I_LOG("client finish.");
  conn.close();
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