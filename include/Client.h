#pragma once
#include "config.h"
#include <string>
#include <atomic>
#include "UdpConnection.h"
#include "seeker/socketUtil.h"


using std::string;


class Client {


  //const string targetHost;
  //const int targetPort;
  //sockaddr_in targetAddr = {0};
  //SOCKET sock;
  //char* dataBuf = "123456789ABCDEFGHIJKLMN";
  UdpConnection conn;
  std::atomic<int> nextMid{0};

  int genMid();

 public:
  Client(const string& serverHost, int serverPort);

  void close();

  void startRtt();


};


