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
#include <string>
#include "Message.h"
#include "seeker/loggerApi.h"

#define SERVER_BUF_SIZE 2048

using seeker::SocketUtil;
using std::cout;
using std::endl;
using std::string;

Server::Server(int p) {
  D_LOG("init server on port[{}]", p);

  SocketUtil::startupWSA();
  conn.setLocalIp("0.0.0.0");
  conn.setLocalPort(p);
  conn.init();

  D_LOG("server socket bind port[{}] success: {}", p, conn.getSocket());
}

void Server::start() {
  static uint8_t recvBuf[SERVER_BUF_SIZE]{0};



  // Waiting msg.
  auto testId = 0;
  auto recvLen = conn.recvData((char*)recvBuf, SERVER_BUF_SIZE);
  if (recvLen > 0) {
    uint8_t msgType;
    Message::getMsgType(recvBuf, msgType);
    switch ((MessageType)msgType) {
      case MessageType::testRequest: {
        TestRequest req(recvBuf);
        T_LOG("Got TestRequest, msgId={}, testType={}", req.msgId, (int)req.msgType);
        int rst = currentTest > 0 ? 1 : 2;
        TestConfirm response(rst, req.msgId, Message::genMid());
        Message::replyMsg(response, conn);
        T_LOG("Reply Msg TestConfirm, msgId={}, testType={}, rst={}", response.msgId, (int)response.msgType, response.result);
        break;
      }

      case MessageType::rttTestMsg: {
        // TODO here...
        break;
      }

      default:
        break;
    }


    TestConfirm confirm(recvBuf);

    testId = confirm.testId;
  } else {
    throw std::runtime_error("msg receive error.");
  }


  // char recvBuf[4096] = {0};

  // int c = 0;
  // while (c < 100) {
  //  cout << "waiting data..." << endl;

  //  int recvNum = conn.recvData(recvBuf, 4096);
  //  if (recvNum < 0) {
  //    auto msg = fmt::format("error: recv_num={}", recvNum);
  //    E_LOG(msg);
  //    throw std::runtime_error(msg);
  //  }

  //  conn.reply(recvBuf, recvNum);
  //  c += 1;
  //  recvBuf[recvNum] = '\0';
  //  I_LOG("[{}]: recv [{} byte] from [{}]: [{}]", c, recvNum,
  //        inet_ntoa(conn.lastAddr.sin_addr), recvBuf);
  //}
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
