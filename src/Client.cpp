/**
  文件注释样例，参考： http://www.edparrish.net/common/cppdoc.html
  socket客户端实现

  @author Tao Zhang, Tao, Tao
  @since 2020/4/26
  @version 0.1.3 2020/4/30
*/

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
  Message::sendMsg(msg, conn);
  //T_LOG("send Message, msgType={} msgId={}", msg.msgType, msg.msgId);
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
  TestRequest req(TestType::rtt, time, genMid());

  sendMsg(req);
  //I_LOG("send TestRequest, msgId={} testType={}", req.msgId, req.testType);


  // Waiting confirm.
  auto testId = 0;
  auto recvLen = conn.recvData((char*)recvBuf, CLIENT_BUF_SIZE);
  if (recvLen > 0) {
    TestConfirm confirm(recvBuf);
    I_LOG("receive TestConfirm, result={} reMsgId={} testId={}",
          confirm.result,
          confirm.reMsgId,
          req.testId);
    testId = confirm.testId;
    memset(recvBuf, 0, recvLen);
  } else {
    throw std::runtime_error("TestConfirm receive error. recvLen=" + std::to_string(recvLen));
  }


  int count = 20;
  int testPacketLen = 64;

  while (count > 0) {
    count--;
    RttTestMsg msg(testPacketLen, testId, genMid());
    sendMsg(msg);
    T_LOG("send RttTestMsg, msgId={} testId={} time={}", msg.msgId, msg.testId, msg.timestamp);

    recvLen = conn.recvData((char*)recvBuf, CLIENT_BUF_SIZE);
    if (recvLen > 0) {
      RttTestMsg rttResponse(recvBuf);
      memset(recvBuf, 0, recvLen);
      auto diffTime = seeker::Time::microTime() - rttResponse.timestamp;
      I_LOG("receive RttTestMsg, msgId={} testId={} time={} diff={}ms",
            rttResponse.msgId,
            rttResponse.testId,
            rttResponse.timestamp,
            (double)diffTime / 1000);
    } else {
      throw std::runtime_error("RttTestMsg receive error. recvLen=" + std::to_string(recvLen));
    }


  }



  // TODO server process RTT msg
}



void startClient(const string& serverHost, int serverPort, int timeInSeconds) {
  I_LOG("Starting send data to {}:{}", serverHost, serverPort);
  Client client(serverHost, serverPort);

  client.startRtt();

  client.close();
}