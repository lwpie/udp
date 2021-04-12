# UDP 编程

## 作业要求

> 设计并实现一个 UDP 客户端/服务器，客户端向服务器登记自己发送数据包的 IP 地址和端口号，同时可以向服务器查询其他用户的 IP 地址和端口号。

> 要求协议设计和实现可靠、一致。

这次编程作业看起来是比较简单的 UDP 编程，实现以下流程即可：

1. 客户端向服务端发起请求
2. 服务端记录客户端的 IP 地址与端口号（下称端点信息）
3. 服务器端向客户端返回所有用户的端点信息

但实际上需要设计一个完善的处理流程来解决协议的可靠与一致性，需要做到：

1. 可以处理多客户端请求
2. 可以处理失败的请求并保持运行
3. 可以处理超时的请求并保持运行
4. 可以校验受到数据的正确性

## 协议设计

为了解决如上问题，采用下列设计，主要针对服务端：

1. 使用异常 `exception` 处理机制处理非法请求，保持服务端在线
2. 使用异步 `async` 协程处理请求，避免长时间占用计算资源
3. 使用定时器 `deadline_timer` 限制协程执行时间，避免被单次请求持续占用
4. 使用序列化与反序列化 `serialization` 对发送的数据结构进行处理，保持传输效率与正确性

具体来说，服务端 `udp_server` 主要分为三个任务：接收数据，存储数据，发送数据，被套在主进程中 `try ... catch` 块中，并用 `while (true)` 保持在线

对于接受 `receive` 与发送 `send`，均使用异步接口 `async_receive_from` 与 `async_send_to` 进行数据收发，并通过 `expires_from_now` 计时器控制超时，分别由 `handle_receive`, `handle_send` 与 `check_deadline` 处理回调

在存储数据 `store` 时，使用无序集合 `unordered_set` 存储端口信息 `endpoint`，并在 `udp_pack` 中封装成支持序列化/反序列化的 `udp_endpoint_set` 类，供发送时序列化

客户端 `udp_client` 较为简单，向服务端建立连接后即可受到服务端发送的序列化数据，尝试反序列化回 `udp_endpoint_set` 类，成功则说明数据正确传输并打印输出，失败则报错

## 设计优势

通过异常处理与定时器，服务器可以保持稳定在线，不受非法请求与持续占用请求的影响

通过异步机制降低计算资源使用率

通过合理的序列化与反序列化，使数据传输更加高效，并可以在传输出现错误时及时发现

通过合理的功能划分，保持代码高可读性与功能解耦合性

## 代码运行

```shell
make
sudo ./udp_server <listen_addr> <listen_port>
sudo ./udp_client <addr> <port>
```

示例输出：

```shell
# udp_server
Receive error: Operation canceled # timeout
Received: 123 # client 0
Sent: 93
Receive error: Operation canceled
Receive error: Operation canceled
Receive error: Operation canceled
Received: abc # client 1
Sent: 116
```

```shell
# udp_client 0
Enter message: 123
Reply length: 93
127.0.0.1:41021
```

```shell
# udp_client 1
Enter message: abc
Reply length: 116
127.0.0.1:41021
127.0.0.1:35207
```

