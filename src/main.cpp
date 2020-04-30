/**
  文件注释样例，参考： http://www.edparrish.net/common/cppdoc.html

  @project netTester
  @author Tao Zhang, Tao, Tao
  @since 2020/4/26
  @version 0.1.3 2020/4/30
*/

#include "config.h"
#include "cxxopts.hpp"
#include <iostream>
#include <string>
#include "seeker/logger.h"

using std::cout;
using std::endl;
using std::string;

cxxopts::ParseResult parse(int argc, char* argv[]);


void startClient(const string& serverHost, int serverPort, int timeInSeconds);

void startServer(int port);

int main(int argc, char* argv[]) {
  auto result = parse(argc, argv);
  seeker::Logger::init();

  try {
    if(result.count("s")) {
      int port = result["p"].as<int>();
      startServer(port);
    } else if(result.count("c")) {
      string host = result["c"].as<string>();
      int port = result["p"].as<int>();
      int time = result["t"].as<int>();
      startClient(host, port, time);
    } else {
      E_LOG("params error, it should not happen.");
    }



  
  } catch(...) {         
    cout << "unknown error." << endl;
  }
  return 0;
}
