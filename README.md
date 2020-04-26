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
- [ ] Client发送UDP消息，Server接收UDP消息
- [ ] Server接收到消息后，进行回复
- [ ] Client接收Server返回的消息
- [ ] 命令行参数解析
- [ ] 定义测试消息基础格式
- [ ] 完成RTT功能
- [ ] 完成带宽、抖动、丢包率功能
