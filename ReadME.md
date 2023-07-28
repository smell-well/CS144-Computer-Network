# CS144 Computer Network 学习记录

## 参考引用
- [代码仓库](https://gitee.com/kangyupl/sponge)

## 进展

- [x] lab0
- [ ] lab1
- [ ] lab2
- [ ] lab3
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

