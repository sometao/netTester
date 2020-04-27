#include "config.h"
#include "Server.h"
#include <string>

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
  char recvBuf[4096] = {0};

  int c = 0;
  while (c < 100) {
    cout << "waiting data..." << endl;

    int recvNum = conn.recvData(recvBuf, 4096);
    if (recvNum < 0) {
      auto msg = fmt::format("error: recv_num={}", recvNum);
      E_LOG(msg);
      throw std::runtime_error(msg);
    }

    conn.reply(recvBuf, recvNum);
    c += 1;
    recvBuf[recvNum] = '\0';
    I_LOG("[{}]: recv [{} byte] from [{}]: [{}]", c, recvNum,
          inet_ntoa(conn.lastAddr.sin_addr), recvBuf);
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
