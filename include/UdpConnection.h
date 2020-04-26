#include "config.h"
#include <string>
#include "seeker/socketUtil.h"


using std::string;


class UdpConnection {
  int localPort{0};
  int remotePort{0};

  string localIp{""};
  string remoteIp{""};

  bool inited{false};
  SOCKET sock{0};

  SOCKADDR_IN remoteAddr{};
  SOCKADDR_IN lastRemoteAddr{};

  void updateRemoteAddr(const SOCKADDR_IN& addr);

 public:
  UdpConnection();

  UdpConnection& setLocalIp(const string& ip);
  UdpConnection& setLocalPort(int port);

  UdpConnection& setRemoteIp(const string& ip);
  UdpConnection& setRemotePort(int port);



  void sendData(char* buf, size_t len);

  int recvData(char* buf, size_t len, bool updatAddr = false);

};