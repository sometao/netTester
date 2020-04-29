#include "Client.h"
#include "seeker/loggerApi.h"
#include "seeker/common.h"
#include <string>
#include <chrono>
#include <iostream>
#include <thread>

#define CLIENT_BUF_SIZE 2048

using seeker::SocketUtil;
using std::string;

int Client::genMid() {
  auto mid = nextMid.fetch_add(1);
  return mid;
}

void Client::sendMsg(const Message& msg) {
  static uint8_t sendBuf[CLIENT_BUF_SIZE]{0};

  size_t len = msg.getLength();
  msg.getBinary(sendBuf, CLIENT_BUF_SIZE);
  conn.sendData((char*)sendBuf, len);
  memset(sendBuf, 0, len);
  T_LOG("send Message, msgType={} msgId={}", msg.msgType, msg.msgId);
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

  static uint8_t recvBuf[CLIENT_BUF_SIZE]{0};


  int time = 10;
  TestRequest req(TestType::rtt, time, genMid(), seeker::Time::microTime());

  sendMsg(req);
  I_LOG("send TestRequest, msgId={} testType={}", req.msgId, req.testType);


  // Waiting confirm.
  auto testId = 0;
  auto recvLen = conn.recvData((char*)recvBuf, CLIENT_BUF_SIZE);
  if(recvLen > 0) {
    TestConfirm confirm(recvBuf);
    I_LOG("receive TestConfirm, result={} reMsgId={} testId={}",
      confirm.result,
      confirm.reMsgId,
      req.testId);
    testId = confirm.testId;
  } else {
    throw std::runtime_error("TestConfirm receive error.");
  }


  int count = 10;
  int testPacketLen = 64;

  while(count > 0) {
    count--;
    RttTestMsg msg(testPacketLen, testId, genMid(), seeker::Time::microTime());
    sendMsg(msg);
    I_LOG("send RttTestMsg, msgId={} testId={} time={}",
      msg.msgId,
      msg.testId,
      msg.timestamp
    );


    recvLen = conn.recvData((char*)recvBuf, CLIENT_BUF_SIZE);
    if(recvLen > 0) {
      RttTestMsg rttResponse(recvBuf);
      auto diffTime = seeker::Time::microTime() - rttResponse.timestamp;
      I_LOG("receive RttTestMsg, msgId={} testId={} time={} diff={}ms",
        rttResponse.msgId,
        rttResponse.testId,
        rttResponse.timestamp,
        (double)diffTime/1000
        );
    } else {
      throw std::runtime_error("TestConfirm receive error.");
    }
  }




  // TODO server process RTT msg
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