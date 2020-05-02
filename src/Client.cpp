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
#include <cassert>

#define CLIENT_BUF_SIZE 2048

using seeker::SocketUtil;
using std::string;

int Client::genMid() { return nextMid.fetch_add(1); }

void Client::sendMsg(const Message& msg) {
  Message::sendMsg(msg, conn);
  // T_LOG("send Message, msgType={} msgId={}", msg.msgType, msg.msgId);
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

void Client::startRtt(int testTimes, int packetSize) {
  uint8_t recvBuf[CLIENT_BUF_SIZE]{0};

  assert(packetSize >= 24);

  TestRequest req(TestType::rtt, 0, genMid());

  sendMsg(req);
  I_LOG("send TestRequest, msgId={} testType={}", req.msgId, req.testType);


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



  while (testTimes > 0) {
    testTimes--;
    RttTestMsg msg(packetSize, testId, genMid());
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

void Client::startBandwidth(uint32_t bandwidth,
                            char bandwidthUnit,
                            int packetSize,
                            int testSeconds,
                            int reportInterval) {
  uint64_t bandwidthValue = 0;
  switch (bandwidthUnit) {
    case 'b':
    case 'B':
      bandwidthValue = bandwidth;
      break;
    case 'k':
    case 'K':
      bandwidthValue = 1024 * (uint64_t)bandwidth;
      break;
    case 'm':
    case 'M':
      bandwidthValue = 1024 * 1024 * (uint64_t)bandwidth;
      break;
    case 'g':
    case 'G':
      bandwidthValue = 1024 * 1024 * 1024 * (uint64_t)bandwidth;
      break;
    default:
      throw std::runtime_error("bandwidthUnit error: " + std::to_string(bandwidthUnit));
  }

  assert(packetSize >= 24);
  assert(packetSize <= 1500);
  assert(bandwidthValue < (uint64_t)10 * 1024 * 1024 * 1024 + 1);


  uint8_t recvBuf[CLIENT_BUF_SIZE]{0};
  TestRequest req(TestType::bandwidth, testSeconds, genMid());

  sendMsg(req);
  I_LOG("send TestRequest, msgId={} testType={}", req.msgId, req.testType);


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



  const int packetsPerSecond = bandwidthValue / packetSize;
  const int groupsPerSecond = 250;

  const int packestPerGroup = packetsPerSecond / groupsPerSecond;
  const int restPacketsPerSecond = packetsPerSecond - (packestPerGroup * groupsPerSecond);



  uint8_t sendBuf[CLIENT_BUF_SIZE]{0};

  //TODO find how many group should one seconds have;
  //TODO find packets per group
  //TODO send a group packets and check whether sleep or not.


  BandwidthTestMsg msg(packetSize, testId, 0, genMid());
  size_t len = msg.getLength();
  msg.getBinary(sendBuf, MSG_SEND_BUF_SIZE);

  int testNum = 0;
  int64_t startTime = seeker::Time::currentTime();
  int64_t endTime = startTime + ((int64_t)testSeconds * 1000);
  int passedTime = 0;
  for (; seeker::Time::currentTime() < endTime * 1000;) {
    for (int g = 0; g < groupsPerSecond; g++) {
      for (int i = 0; i < packestPerGroup; i++) {
        BandwidthTestMsg::update(sendBuf, genMid(), testNum, seeker::Time::microTime());
        conn.sendData((char*)sendBuf, len);
        testNum += 1;
      }
      passedTime = seeker::Time::currentTime() - startTime;
    }

    for (int i = 0; i < restPacketsPerSecond; i++) {

    }
    passedTime = seeker::Time::currentTime() - startTime;
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
}



void startClient(const string& serverHost, int serverPort, int timeInSeconds) {
  I_LOG("Starting send data to {}:{}", serverHost, serverPort);
  Client client(serverHost, serverPort);

  client.startRtt(10, 64);

  client.close();
}