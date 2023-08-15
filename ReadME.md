# CS144 Computer Network 学习记录

## 参考引用
- [代码仓库](https://gitee.com/kangyupl/sponge)

## 进展

- [x] lab0 大约10h
- [x] lab1 大约20h
- [x] lab2 大约12h
- [x] lab3 大约12h
- [ ] lab4
- [ ] lab5
- [ ] lab6
- [ ] lab7

## lab0

### 1 环境安装
笔者采用的方式远程linux机器+vscode remote进行实验

远程服务器为ubuntu 20.04，具体的实验环境安装方法可以参考以下链接

1 [lab0环境安装](https://www.misaka-9982.com/2023/02/15/CS144-%E7%8E%AF%E5%A2%83%E9%85%8D%E7%BD%AE/)

2 [lab0环境安装](http://doraemonzzz.com/2021/12/12/2021-12-12-CS144-%E5%AE%9E%E9%AA%8C%E7%8E%AF%E5%A2%83%E9%85%8D%E7%BD%AE/)
注意要使用这个链接的gitee 仓库获取相关的代码库，因为原cs144的库已经消失了，所以用了这里的备用库，
因此希望大家还是遵守课程约定，不看其他人的实现代码

### 2 Networking by hand

#### 2.1 Fetch a Web page
```
ymk@ubuntu:~/proj/cs144/sponge/build$ telnet cs144.keithw.org http
Trying 104.196.238.229...
Connected to cs144.keithw.org.
Escape character is '^]'.
GET /hello HTTP/1.1
Host: cs144.keithw.org
Connection: close

HTTP/1.1 200 OK
Date: Wed, 26 Jul 2023 15:27:32 GMT
Server: Apache
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
ETag: "e-57ce93446cb64"
Accept-Ranges: bytes
Content-Length: 14
Connection: close
Content-Type: text/plain

Hello, CS144!
Connection closed by foreign host.

ymk@ubuntu:~/proj/cs144/sponge/build$ telnet cs144.keithw.org http
Trying 104.196.238.229...
Connected to cs144.keithw.org.
Escape character is '^]'.
GET /lab0/01 HTTP/1.1
Host: cs144.keithw.org
Connection: close

HTTP/1.1 200 OK
Date: Wed, 26 Jul 2023 15:34:18 GMT
Server: Apache
X-You-Said-Your-SunetID-Was: 01
X-Your-Code-Is: 920194
Content-length: 106
Vary: Accept-Encoding
Connection: close
Content-Type: text/plain

Hello! You told us that your SUNet ID was "01". Please see the HTTP headers (above) for your secret code.
Connection closed by foreign host.
```

#### 2.2 Send yourself an email
因不是本校原因，所以跳过该章节

#### 2.3 Listening and connecting
```
nc -lvnp 9090 # 因为修改了localhost，netcat失败，因此使用了代替方案
Listening on 0.0.0.0 9090
Connection received on 127.0.0.1 60000
fadfas
fadfads
^C
```
说实话，做到这个实验发现可以实现双方通信了感觉还是有点神奇的。

### 3 Writing a network program using an OS stream socket

3.1 - 3.3 是一些规范性内容，其中的配置问题已在上面的链接中说明，此不做赘述

#### 3.4 Writing webget
具体的TCPSocket使用方式可以参考 [TCP使用范例](https://cs144.github.io/doc/lab0/class_t_c_p_socket.html#a45d4ff72e1f0c755225e60f89326085c)
- 尤其要注意write的内容,真的是一个空格都不能多

### 4 An in-memory reliable byte stream
实现一个比特流的处理类, 主要是熟悉下c++编程。

## lab1
最开始准备采用的index + string，通过维护数据段的方式来实现unassembled区域的管理的，但是维护区间合并实在是太麻烦了，
所以参考了一些大佬的做法决定采用以位为单位的做法。呕了，使用位做法的话对string data会很大，有点不太符合需要，最终还是准备
写下维护数据段的做法。其实这个做法来说的话有点像是线段树了已经。

记录一下坑点
- 实际上维护都两个部分，一部分是接入output的连续片段，另一部分是离散的片段，两部分大小为capacity
- 不用纠结计算可以存下多少空间，可以先计算出窗口所能达到的上限位置，从而进行空间的限制
- 先对输入进行基本的裁切会简单一点

## lab2
### 索引转化
没想到一个简单的转化居然会这么麻烦，写了4h，期间不断遇到问题。

主要理一下unwrap的思路，感觉代码还是有点不够清晰。
1、将 n 从相对isn的索引转化为真实值的索引，写作x（注意，不要反过来将checkpoint转到isn，这样会麻烦）
2、将 checkpoint 的 MOD（1ul << 32）部分取出，作为真实值 n_t 的偏移部分，写作base。取出 MOD 取模的部分，用以比较和 n 的相对关系，写作y
3、根据x 和 y 的左右关系比较，尤其需要注意，当 x 需要减去一个 MOD 才能是更接近checkpoint时，要判断是否会小于0

### TCPReceiver
按照手册的逻辑实现即可。有几点需要注意
- 当收到 FIN 时，返回的 ack 应该是 FIN 之后的值，因为 FIN 也占用了一个序列号单位
- 当没有设置 SYN 时，直接丢弃

## lab3
sender的条理上没有receiver清晰，我也是看了很多相关的解释。我觉得棘手的点主要在于没有一个清晰的事件处理过程，要是有时序图可能会更好理解。
我简单列一下我觉得的流程。

启动，调用fill_windows -> 等待ack_received(内部会继续调用fill_windows)；如此循环。

还需要注意的是，fill_windows中可能遇到需要拆分的情况，应该在一次调用中解决。

一些坑点：
- 如果当前发送的 segment 的实际大小为 0，需要提前终止，不需要发送
- fill_windows 里面可以发送的内容大小，要保证 **发送的** + **已发送的** 的和在窗口内
- 初始化的窗口大小应当设置为 1，因为我们至少要发送 SYN 来开始连接，且可以与 0 窗口大小的值进行区分。（这个在FAQ里有，写的时候忘记了，被坑到）
- 对于 FIN 的处理，由于是最后写入的，要判断加上FIN后是否会导致超过窗口和超过segment大小限制
- 非法Ack的处理
    - 超过 _next_seqno 的部分，直接舍弃
    - 是以往的包的 ack，这时候不一定直接舍弃，可以更新窗口大小，但不一定更新timer
- FIN 包只能发送一次，防止等待应答过程中多次发送
- TCPConfig::MAX PAYLOAD SIZE 只限制payload部分的大小

## lab4
好好好，lab0-lab3 加起来也就30几个测试用例，到lab4就变成160几是吧，难怪都说这个难啊。

写的过程很困惑，即使看完了handout也不知道该怎么写。比如TCP connection 的函数，都不清楚它该在什么时候调用，有个什么样的预期行为。这可能也和我直接做的lab没有看视频有关系，对于整个过程没有一个自顶向下的了解，所以在写的时候寸步难行。

还有一个就是看到大家都会说这个实验要写状态图啥的，我是一点没明白要啥状态图啊，难道不是按教程里的写吗？

直接写实在是太难了，我才用分步的方式，先写建立连接的部分，完成fsm_connect_relaxed再说。
在写fsm_active_close的时候，需要额外注意当sender不会发送有内容的包时，手动判断了发送空的ack包的情况（还挺麻烦的）

### 实验过程
- receiver 和 sender 主要维护报文中的 SYN、FIN、seqno、payload部分，而ack，ackno，win、rst等由 TCPConnection 来维护
- 这个`linger_after_streams_finish`到底是什么意思，干什么用的？
- 有些状态除了sender 和 receiver 状态外，还需要考虑`linger_after_streams_finish`
