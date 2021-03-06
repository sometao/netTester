/**
  文件注释样例，参考： http://www.edparrish.net/common/cppdoc.html
  socket服务端声明

  @project netTester
  @author Tao Zhang, Tao, Tao
  @since 2020/4/26
  @version 0.1.3 2020/4/30
*/
#pragma once
#include "config.h"
#include "UdpConnection.h"
#include <atomic>
#include "seeker/loggerApi.h"
#include "seeker/socketUtil.h"


class Server {

  UdpConnection conn{};

  int genTestId();
  uint16_t currentTest{0};
  std::atomic<int> testIdGen{1};

  void bandwidthTest(int testSeconds);

 public:
  Server(int p);

  void start();


  void close();



};