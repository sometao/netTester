### NetTester v0.0.1
---


### 目标
- 掌握UDP通信编程
- 提升CPP编码能力
- 了解网络传输基本知识


### 功能
- 通过UDP包测试带宽，抖动，丢包率
- 能够通过UDP测试往返时延（RTT: Round-Trip Time）
- 提供友好的命令行参数交互，使用：https://github.com/jarro2783/cxxopts/tree/v2.2.0
- 原理和交互可参考iperf/iperf3



### TODOS
- [X] Client发送UDP消息，Server接收UDP消息
- [X] Server接收到消息后，进行回复
- [X] Client接收Server返回的消息
- [X] 命令行参数解析
- [ ] 定义测试消息基础格式
- [ ] 完成RTT功能
- [ ] 完成带宽、抖动、丢包率功能


### RTT 测试

#### 主要流程
1. 客户端发送测试请求
2. 服务端确认接受测试
3. 客户端开始发送udp测试数据包
4. 服务端接收到udp数据包后，原样回复
5. 客户端多轮发包后，对数据进行统计，打印出RTT结果

#### 所需消息
- 测试请求，指明测试类型，消息id
- 测试确认，指明接受测试，消息id，测试id，以及测试请求的消息id
- RTT测试消息，消息id，测试id，时间戳（微秒）




### 带宽测试

#### 主要流程
1. 客户端发送测试请求
2. 服务端确认接受测试
3. 客户端开始发送测试数据包
4. 服务端接收到udp数据包后进行统计和记录，并按照时间间隔打印日志
5. 客户端完成测试后，发送测试结束消息给服务端
6. 服务端收到测试结束的消息后，将结果报告发送给客户端
7. 客户端打印出结果报告

#### 所需消息
- 测试请求，指明测试类型，消息id，测试时长（秒）
- 测试确认，指明接受测试，消息id，测试id，以及测试请求的消息id
- 带宽测试消息，消息id，测试id，测试包序号（uint32），时间戳（微妙）
- 测试结束，消息id，测试id，最后一个测试包的序号（uint32）


#### 约束
- 服务端同一时间只能进行一个带宽测试




### 通信协议
- head
  - 固定长度
  - 消息类型：1个字节
  - 测试id：2个字节
  - 消息id：4个字节
  - 消息时间戳：8个字节
- payLoad：不同类型的消息，包含不同的payLoad
  - 测试请求：测试类型（RTT、带宽），测试时长
  - 测试确认：是否接受测试，分配的测试id，以及测试请求的消息id
  - RTT 测试包：根据包大小设置，填充随机数据
  - 带宽测试包：测试包序号，根据包大小设置，填充随机数据
  - 带宽测试结束请求：空
  - 带宽测试报告：测试实际时长，实际接收数据量，平均带宽，时延抖动，丢包数量、包总量

