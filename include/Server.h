#pragma once
#include "config.h"
#include "UdpConnection.h"
#include "seeker/loggerApi.h"
#include "seeker/socketUtil.h"


class Server {
  //int port;
  //SOCKET sock;

  UdpConnection conn{};



 public:
  Server(int p);

  void start();


  void close();



};