#include "seeker/loggerApi.h"
#include "seeker/socketUtil.h"

namespace netTester{

}

class Server {
  int port;
  SOCKET sock;



 public:
  Server(int p);

  void start();


  void close();



};