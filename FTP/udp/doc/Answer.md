# 用UDP实现聊天软件
服务端方面，将两个客户端分别绑定到特定的端口，并且开两个线程分别用于监听两个端口发来的信息，一个端口收到信息后转发到另外一个端口。

客户端方面，开一个线程用于持续监听服务器端口，另一个线程用于输入并发送信息，不过要注意的是发送信息的时候要注意加互斥锁，不然会很混乱。