#include "UdpConnection.h"
#include "seeker/loggerApi.h"

using seeker::SocketUtil;

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

UdpConnection& UdpConnection::init() {
  if (inited) {
    throw std::runtime_error("UdpConnection has been already inited.");
  }

  sock = socket(AF_INET, SOCK_DGRAM, 0);

  if (localPort > 0) {
    localAddr = SocketUtil::createAddr(localPort, localIp);
    if (bind(sock, (sockaddr*)&localAddr, sizeof(sockaddr_in)) == SOCKET_ERROR) {
      SocketUtil::closeSocket(sock);
      SocketUtil::cleanWSA();
      auto msg = fmt::format("bind error: [{}:{}]", localIp, localPort);
      throw std::runtime_error(msg);
    }
    D_LOG("connect inited with local address [{}:{}]", localIp, localPort);
    inited = true;
  }

  if (remotePort > 0 && !remoteIp.empty()) {
    remoteAddr = SocketUtil::createAddr(remotePort, remoteIp);
    D_LOG("connect inited with remote address [{}:{}]", remoteIp, remotePort);
    inited = true;
  }

  if (!inited) {
    throw std::runtime_error("connection init failed.");
  }

  return *this;
}

void UdpConnection::close() {
  SocketUtil::closeSocket(sock);
  SocketUtil::cleanWSA();
}

SOCKET UdpConnection::getSocket() { return sock; }


void UdpConnection::updateRemoteAddr(const SOCKADDR_IN& addr) {
  // TODO lock maybe need here.
  remoteIp = inet_ntoa(addr.sin_addr);
  remotePort = ntohs(addr.sin_port);
  remoteAddr.sin_family = addr.sin_family;
  remoteAddr.sin_addr = addr.sin_addr;
  remoteAddr.sin_port = addr.sin_port;
}

void UdpConnection::updateRemoteAddr() { updateRemoteAddr(lastAddr); }

void UdpConnection::sendData(char* buf, size_t len) {
  if (inited && remoteAddr.sin_port != 0) {
    static auto addrLen = sizeof(SOCKADDR);
    sendto(sock, buf, len, 0, (sockaddr*)&remoteAddr, addrLen);
  } else {
    throw std::runtime_error("connection not inited or remoteAddr not set.");
  }
}

void UdpConnection::reply(char* buf, size_t len) {
  if (inited && lastAddr.sin_port != 0) {
    static auto addrLen = sizeof(SOCKADDR);
    sendto(sock, buf, len, 0, (sockaddr*)&lastAddr, addrLen);
  } else {
    throw std::runtime_error("connection not inited or lastAddr not set.");
  }
}

int UdpConnection::recvData(char* buf, size_t len) {
  if (inited) {
    return recvfrom(sock, buf, len, 0, (sockaddr*)&lastAddr, (socklen_t*)&lastAddrLen);
  } else {
    throw std::runtime_error("connection not inited");
  }
}