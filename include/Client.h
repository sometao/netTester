#include <string>
#include "seeker/socketUtil.h"


using std::string;


class Client {


  const string targetHost;
  const int targetPort;
  sockaddr_in targetAddr = {0};
  SOCKET sock;
  char dataBuf[4096] = "123456789";

 public:
  Client(const string& serverHost, int serverPort);

  void sendData(size_t num);

  void close();




};


