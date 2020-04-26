#include "UdpConnection.h"
#include "seeker/loggerApi.h"



UdpConnection::UdpConnection() {}

UdpConnection& UdpConnection::setLocalIp(const string& ip) {
  localIp = ip;
  return *this;
}

UdpConnection& UdpConnection::setLocalPort(int port) {
  localPort = port;
  return *this;
}

UdpConnection& UdpConnection::setRemoteIp(const string& ip) {
  remoteIp = ip;
  return *this;
}

UdpConnection& UdpConnection::setRemotePort(int port) {
  remotePort = port;
  return *this;
}


void UdpConnection::updateRemoteAddr(const SOCKADDR_IN& addr) {
  // TODO lock maybe need here.
  remoteIp = inet_ntoa(addr.sin_addr);
  remotePort = ntohs(addr.sin_port);
  remoteAddr.sin_addr = addr.sin_addr;
  remoteAddr.sin_port = addr.sin_port;
}

void UdpConnection::sendData(char* buf, size_t len) {
  if (inited && remotePort > 0) {
    static auto addrLen = sizeof(SOCKADDR);
    sendto(sock, buf, len, 0, (sockaddr*)&remoteAddr, addrLen);
  } else {
    throw std::runtime_error("connection not inited or remoteAddr not set.");
  }
}

int UdpConnection::recvData(char* buf, size_t len, bool updatAddr) {
  if (inited) {
    socklen_t addrLen;
    int recvNum =
        recvfrom(sock, buf, len, 0, (sockaddr*)&lastRemoteAddr, (socklen_t*)&addrLen);
    if (updatAddr) {
      updateRemoteAddr(lastRemoteAddr);
    }
    return recvNum;
  } else {
    throw std::runtime_error("connection not inited");
  }
}
