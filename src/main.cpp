#include "config.h"
#include "cxxopts.hpp"
#include <iostream>
#include <string>
#define CXXOPTS_USE_UNICODE

using std::cout;
using std::endl;
using std::string;

cxxopts::ParseResult parse(int argc, char* argv[]);



int main(int argc, char* argv[]) {
  auto result = parse(argc, argv);
  try {
    auto s = result["s"].as<bool>();
    cout << "server:" << s << endl;

    auto v = result["input"].as<string>();
    cout << "input:" << v << endl;
  
  } catch(...) {         
    cout << "unknown error." << endl;
  }
   

  return 0;
}
