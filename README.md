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
- [X] 定义测试消息基础格式
- [X] 完成RTT功能
- [X] 完成带宽、抖动、丢包率功能
- [ ] 兼容linux系统，并进行windows与linux之间的测试
- [ ] 与iperf测试结果进行对比，特别是本地高带宽（1GBit以上）测试


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


#### 客户端发送包方案
- 根据带宽和包大小，确定每秒发送包的个数（PacketsPerSecond）
- 设定一个检测发送进度的时间跨度（groupTime）
- 根据groupTime，估计出每次检测间连续发送包的个数（packetsPerGroup）
- 发送时，连续发送packetsPerGroup个包，然后检测下发送的数据量是否超前，超前的话就sleep下
- 反复执行上一步，知道测试时间结束


#### 服务端统计方法
- 计算指标: Bandwidth, Delay jitter, Packet loss rate 
- 统计数据量,
  - 收到第一个数据包的时间为起始时间，根据UDP报文长度计算数据量
  - 带宽 = 总数据量 / 时间（s）
- 统计延迟抖动
  - 本地消息时间戳 - 消息时间戳 = 相对延迟
  - 延迟抖动 = max(相对延迟) - min(相对延迟)
- 丢包
  - 根据测试序号进行统计,
  - 测试序号从0开始
  - 测试发送的总包数，由测试结束消息给出


### 约束
- 服务端同一时间只能进行一项测试




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

