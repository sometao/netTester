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
#include <cmath>

#define CLIENT_BUF_SIZE 2048

using seeker::SocketUtil;
using std::cout;
using std::endl;
using std::string;


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

  TestRequest req(TestType::rtt, 0, Message::genMid());

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
    RttTestMsg msg(packetSize, testId, Message::genMid());
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

extern string formatTransfer(const uint64_t& dataSize);

extern string formatBandwidth(const uint32_t& bytesPerSecond);

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


  assert(testSeconds <= 100);
  assert(packetSize >= 24);
  assert(packetSize <= 1500);
  assert(bandwidthValue < (uint64_t)10 * 1024 * 1024 * 1024 + 1);
  cout << "bandwidthValue in bit:" << bandwidthValue << endl;
  bandwidthValue = bandwidthValue / 8;
  cout << "bandwidthValue in byte:" << bandwidthValue << endl;

  uint8_t recvBuf[CLIENT_BUF_SIZE]{0};
  TestRequest req(TestType::bandwidth, testSeconds, Message::genMid());
  sendMsg(req);
  I_LOG("send TestRequest, msgId={} testType={}", req.msgId, req.testType);


  int testId = 0;
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

  uint8_t sendBuf[CLIENT_BUF_SIZE]{0};

  BandwidthTestMsg msg(packetSize, testId, 0, Message::genMid());
  size_t len = msg.getLength();
  msg.getBinary(sendBuf, MSG_SEND_BUF_SIZE);


  const int groupTimeMs = 5;
  const uint64_t packetsPerSecond = bandwidthValue / packetSize;
  const int packestPerGroup = std::ceil((double)packetsPerSecond * groupTimeMs / 1000);
  const double packetsIntervalMs = (double)1000 / packetsPerSecond;

  I_LOG("bandwidthValue={}, packetsPerSecond={}, packestPerGroup={}, packetsIntervalMs={}",
        bandwidthValue,
        packetsPerSecond,
        packestPerGroup,
        packetsIntervalMs);


  int64_t passedTime = 0;
  int totalPkt = 0;
  int64_t startTime = seeker::Time::currentTime();
  int64_t endTime = startTime + ((int64_t)testSeconds * 1000);

  int64_t lastReportTime = 0;

  uint64_t totalData = 0;
  while (seeker::Time::currentTime() < endTime) {
    for (int i = 0; i < packestPerGroup; i++) {
      T_LOG("send a BandwidthTestMsg, totalPkt={}", totalPkt);
      BandwidthTestMsg::update(sendBuf, Message::genMid(), totalPkt, seeker::Time::microTime());
      conn.sendData((char*)sendBuf, len);
      totalData += len;
      totalPkt += 1;
    }

    passedTime = seeker::Time::currentTime() - startTime;

    if(passedTime - lastReportTime > (int64_t)reportInterval * 1000) {


      lastReportTime = passedTime;
      I_LOG("[ ID] test time   Data transfer          Packets send");

      I_LOG("[{}]       {}s           {}                   {}      ",
        testId,
        passedTime/1000,
        formatTransfer(totalData),
        totalPkt
      );
    }


    int aheadTime = (totalPkt * packetsIntervalMs) - passedTime;
    if (aheadTime > 5) {
      std::this_thread::sleep_for(std::chrono::milliseconds(aheadTime - 2));
    }
  }

  BandwidthFinish finishMsg(testId, totalPkt, Message::genMid());
  sendMsg(finishMsg);
  T_LOG("waiting report.");
  recvLen = conn.recvData((char*)recvBuf, CLIENT_BUF_SIZE);
  if (recvLen > 0) {
    BandwidthReport report(recvBuf);
    I_LOG("bandwidth test report:");
    I_LOG("[ ID] Interval   Transfer   Bandwidth     Jitter   Lost/Total Datagrams");

    int interval = testSeconds;
    int lossPkt = totalPkt - report.receivedPkt;
    I_LOG("[{}] {}s       {}     {}       {}ms   {}/{} ({:.{}f}%)",
          testId,
          interval,
          formatTransfer(report.transferByte),
          formatBandwidth(report.transferByte / interval),
          (double)report.jitterMicroSec / 1000,
          lossPkt,
          totalPkt,
          (double)100 * lossPkt / totalPkt,
          4);
  } else {
    throw std::runtime_error("TestConfirm receive error. recvLen=" + std::to_string(recvLen));
  }
}



void startClientRtt(const string& serverHost, int serverPort, int testSeconds) {
  I_LOG("Starting send data to {}:{}", serverHost, serverPort);
  Client client(serverHost, serverPort);
  client.startRtt(10, 64);
  client.close();
}

void startClientBandwidth(const string& serverHost,
                          int serverPort,
                          int testSeconds,
                          uint32_t bandwidth,
                          char bandwidthUnit,
                          int packetSize,
                          int reportInterval) {
  I_LOG("Starting send data to {}:{}", serverHost, serverPort);
  Client client(serverHost, serverPort);

  client.startBandwidth(bandwidth, bandwidthUnit, packetSize, testSeconds, reportInterval);

  client.close();
}