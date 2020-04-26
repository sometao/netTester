#include "config.h"
#include "seeker/loggerApi.h"
#include "seeker/socketUtil.h"


class Server {
  int port;
  SOCKET sock;



 public:
  Server(int p);

  void start();


  void close();



};