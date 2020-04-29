#pragma once
#include <config.h>
#include <iostream>
#include "seeker/common.h"



class Message {
 protected:
  void writeHead(uint8_t* buf, size_t size) const {
    if (size < 15) {
      throw std::runtime_error("buf for head too small.");
    }
    using seeker::ByteArray;
    ByteArray::writeData(buf + 0, msgType);
    ByteArray::writeData(buf + 1, testId);
    ByteArray::writeData(buf + 3, msgId);
    ByteArray::writeData(buf + 7, timestamp);
  }

  constexpr uint16_t headLen() const { return 15; }


 public:
  uint8_t msgType;    // index: 0
  uint16_t testId;    // index: 1
  int msgId;          // index: 3
  int64_t timestamp;  // index: 7

  Message() = default;

  Message(uint8_t* data) { reset(data); }

  Message(uint8_t mt, uint16_t tid, int mid, int64_t t)
      : msgType(mt), testId(tid), msgId(mid), timestamp(t) {}

  void reset(uint8_t* data) {
    using seeker::ByteArray;
    ByteArray::readData(data + 0, msgType);
    ByteArray::readData(data + 1, testId);
    ByteArray::readData(data + 3, msgId);
    ByteArray::readData(data + 7, timestamp);
  }


  virtual size_t getLength() const = 0;

  virtual void getBinary(uint8_t* buf, size_t size) const = 0;

  static void getMsgType(uint8_t* data, uint8_t& v) {
    seeker::ByteArray::readData(data + 0, v);
  }

  static void getTimestamp(uint8_t* data, int64_t& v) {
    seeker::ByteArray::readData(data + 7, v);
  }
};


enum class TestType {
  unknown,
  rtt,
  bandwith,
};

class TestRequest : public Message {
 public:
  int testType;  //  1 for RTT, 2 for bandwidth.  index: 15.
  int testTime;  //  secondes.                    index: 19.

  static const uint8_t msgType = 1;

  TestRequest() = default;

  TestRequest(uint8_t* data) : Message(data) {
    seeker::ByteArray::readData(data + 15, testType);
    seeker::ByteArray::readData(data + 19, testTime);
  }

  TestRequest(TestType tType, int tTime, int mid, int64_t t)
      : Message(msgType, 0, mid, t), testType((int)tType), testTime(tTime) {}

  size_t getLength() const override { return headLen() + 8; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }

    writeHead(buf, size);
    seeker::ByteArray::writeData(buf + 15, testType);
    seeker::ByteArray::writeData(buf + 19, testTime);
  }
};


class TestConfirm : public Message {
 public:
  int result;   //  1 for OK, 2 for NO           index: 15.
  int reMsgId;  //  reply mid.                   index: 19.

  static const uint8_t msgType = 2;

  TestConfirm() = default;

  TestConfirm(uint8_t* data) : Message(data) {
    seeker::ByteArray::readData(data + 15, result);
    seeker::ByteArray::readData(data + 19, reMsgId);
  }

  TestConfirm(int rst, int reId, int mid, int64_t t)
      : Message(msgType, 0, mid, t), result(rst), reMsgId(reId) {}

  size_t getLength() const override { return headLen() + 8; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }

    writeHead(buf, size);
    seeker::ByteArray::writeData(buf + 15, result);
    seeker::ByteArray::writeData(buf + 19, reMsgId);
  }
};


class RttTestMsg : public Message {
 public:
  int payloadLen;  //                       index: 15.

  static const uint8_t msgType = 3;


  RttTestMsg(uint8_t* data) : Message(data) {
    seeker::ByteArray::readData(data + 15, payloadLen);
  }

  RttTestMsg(int len, int testId, int mid, int64_t t) : Message(msgType, testId, mid, t), payloadLen(len) {}

  size_t getLength() const override { return headLen() + (size_t)payloadLen; }

  void getBinary(uint8_t* buf, size_t size) const override {
    if (size < getLength()) {
      throw std::runtime_error("buf too small.");
    }

    writeHead(buf, size);
    seeker::ByteArray::writeData(buf + 15, payloadLen);
    for (int i = 0; i < payloadLen - 4; i++) {
      seeker::ByteArray::writeData(buf + 19 + i, (uint8_t)i);
    }
  }
};



// TODO BAND_TEST_MSG
// TODO BAND_FINISH_MSG
// TODO BAND_TEST_REPORT