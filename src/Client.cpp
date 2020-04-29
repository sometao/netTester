#include "Client.h"
#include "seeker/loggerApi.h"
#include "seeker/common.h"
#include <string>
#include <chrono>
#include <iostream>
#include <thread>
#include "Message.h"

using seeker::SocketUtil;
using std::string;

int Client::genMid() {
  auto mid = nextMid.fetch_add(1);
  return mid;
}

Client::Client(const string& serverHost, int serverPort) {
  // Get socket ready
  SocketUtil::startupWSA();

  conn.setRemoteIp(serverHost);
  conn.setRemotePort(serverPort);
  conn.init();
}



void Client::close() {
  I_LOG("client finish.");
  conn.close();
}

void Client::startRtt() {
  constexpr size_t bufSize = 2048;
  uint8_t sendBuf[bufSize];
  uint8_t recvBuf[bufSize];

  int time = 10;
  TestRequest req(TestType::rtt, time, genMid(), seeker::Time::microTime());

  size_t len = req.getLength();
  req.getBinary(sendBuf, bufSize);
  conn.sendData((char*)sendBuf, len);
  I_LOG("send TestRequest, msgId={} testType={}", req.msgId, req.testType);


  // Waiting comfirm.
  auto recvLen = conn.recvData((char*)recvBuf, bufSize);
  TestConfirm confirm(recvBuf);
  I_LOG("receive TestConfirm, result={} reMsgId={} testId={}",
        confirm.result,
        confirm.reMsgId,
        req.testId);


  int count = 10;

  while(count > 0) {
    count--;



  
  }




  // TODO here..
}



void startClient(const string& serverHost, int serverPort, int timeInSeconds) {
  I_LOG("Starting send data to {}:{}", serverHost, serverPort);
  Client client(serverHost, serverPort);
  while (timeInSeconds > 0) {
    I_LOG("send data [{}]", timeInSeconds);
    // client.sendData(10);
    // client.readData();
    timeInSeconds--;
  }
  client.close();
}