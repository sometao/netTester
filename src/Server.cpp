/**
  文件注释样例，参考： http://www.edparrish.net/common/cppdoc.html
  socket server implement

  @project netTester
  @author Tao Zhang, Tao, Tao
  @since 2020/4/26
  @version 0.1.3 2020/4/30
*/

#include "config.h"
#include "Server.h"
#include "Message.h"
#include "seeker/loggerApi.h"
#include <string>
#include <set>

#define SERVER_BUF_SIZE 2048

using seeker::SocketUtil;
using std::cout;
using std::endl;
using std::string;

int Server::genTestId() {
  static std::atomic<int> nextTestId{0};
  return nextTestId.fetch_add(1);
}

void Server::bandwidthTest(int testSeconds) {
  uint8_t recvBuf[SERVER_BUF_SIZE]{0};


  uint64_t totalRecvByte = 0;
  int64_t maxDelay = INT_MIN;
  int64_t minDelay = INT_MAX;
  std::set<int> mayMissedTestNum;
  int expectTestNum = 0;
  int startTimeMs = -1;
  int lastArrivalTimeMs = -1;

  while (true) {
    // D_LOG("Waiting msg...");
    auto testId = 0;
    auto recvLen = conn.recvData((char*)recvBuf, SERVER_BUF_SIZE);
    int64_t delay;
    if (recvLen > 0) {
      uint8_t msgType;
      int msgId = 0;
      uint16_t testId = 0;
      int testNum = 0;
      int pktCount = 0;
      int64_t ts = 0;
      Message::getMsgType(recvBuf, msgType);
      Message::getMsgId(recvBuf, msgId);
      Message::getTestId(recvBuf, testId);
      Message::getTimestamp(recvBuf, ts);
      BandwidthTestMsg::getTestNum(recvBuf, testNum);
      if (msgType == (uint8_t)MessageType::bandwidthTestMsg && testId == currentTest) {
        lastArrivalTimeMs = seeker::Time::currentTime();
        if (startTimeMs < 0) {
          startTimeMs = lastArrivalTimeMs;
        }
        totalRecvByte += recvLen;
        delay = seeker::Time::microTime() - ts;
        if (delay < minDelay) minDelay = delay;
        if (delay > maxDelay) maxDelay = delay;
        if (testNum == expectTestNum) {
          pktCount += 1;
          expectTestNum += 1;
        } else if (testNum < expectTestNum) {
          // TODO find pkt in mayMissedTestNum.
          auto search = mayMissedTestNum.find(testNum);
          if(search != mayMissedTestNum.end()) {
            //find it.
            mayMissedTestNum.erase(search);
            pktCount += 1;
          }
        } else {
          if (testNum - expectTestNum > 300) {
            W_LOG("missed too much, testNum={}, expectTestNum={}, len=[{}], ", testNum,
                  expectTestNum, (testNum - expectTestNum));
          }
          for (int i = expectTestNum; i < testNum; i++) {
            mayMissedTestNum.insert(i);
          }
          if (mayMissedTestNum.size() > 2000) {
            // only keep 1000, remove others.
            W_LOG("mayMissedTestNum.size()[] > 2000, only keep 1000", mayMissedTestNum.size());
            auto it = mayMissedTestNum.begin();
            for (int i = 0; i < 1000; i++) ++it;
            mayMissedTestNum.erase(it, mayMissedTestNum.end());
          }
          pktCount += 1;
          expectTestNum = testNum + 1;
        }
      } else if (msgType == (uint8_t)MessageType::bandwidthFinish && testId == currentTest) {
        // TODO print report, save report, send report.

      } else {
        // ignore. nothing to od.
        W_LOG("Got a unexpected msg. msgId={} msgType={} testId={}", msgId, msgType, testId);
      }

    } else {
      throw std::runtime_error("msg receive error.");
    }
  }
}

Server::Server(int p) {
  D_LOG("init server on port[{}]", p);

  SocketUtil::startupWSA();
  conn.setLocalIp("0.0.0.0");
  conn.setLocalPort(p);
  conn.init();

  D_LOG("server socket bind port[{}] success: {}", p, conn.getSocket());
}

void Server::start() {
  uint8_t recvBuf[SERVER_BUF_SIZE]{0};

  while (true) {
    // D_LOG("Waiting msg...");
    auto testId = 0;
    auto recvLen = conn.recvData((char*)recvBuf, SERVER_BUF_SIZE);
    if (recvLen > 0) {
      uint8_t msgType;
      Message::getMsgType(recvBuf, msgType);
      int msgId = 0;
      Message::getMsgId(recvBuf, msgId);
      switch ((MessageType)msgType) {
        case MessageType::testRequest: {
          TestRequest req(recvBuf);
          T_LOG("Got TestRequest, msgId={}, testType={}", req.msgId, (int)req.msgType);
          TestConfirm response(1, req.msgId, Message::genMid());
          T_LOG("Reply Msg TestConfirm, msgId={}, testType={}, rst={}", response.msgId,
                (int)response.msgType, response.result);
          Message::replyMsg(response, conn);
          if (req.testType == 2) {
            currentTest = req.testId;
            bandwidthTest(req.testTime);
          } else {
            // nothing to do.
          }
          break;
        }
        case MessageType::rttTestMsg: {
          conn.reply((char*)recvBuf, recvLen);
          // D_LOG("Reply rttTestMsg, msgId={}", msgId);
          break;
        }
        default:
          W_LOG("Got unknown msg, ignore it. msgType={}, msgId={}", msgType, msgId);
          break;
      }
    } else {
      throw std::runtime_error("msg receive error.");
    }
  }
}



void Server::close() {
  I_LOG("Server finish.");
  conn.close();
}



void startServer(int port) {
  I_LOG("Starting server on port={}", port);
  Server server{port};
  server.start();
  server.close();
}
