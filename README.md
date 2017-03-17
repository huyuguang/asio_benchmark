# asio_benchmark
asio性能测试程序集

支持windows和linux平台。

Usage: asio_test socketpair <pair_count> <active_count> <write_count>
Usage: asio_test client1 <host> <port> <threads> <blocksize> <sessions> <time>
Usage: asio_test server1 <address> <port> <threads> <blocksize>
Usage: asio_test client2 <host> <port> <threads> <blocksize> <sessions> <time>
Usage: asio_test server2 <address> <port> <threads> <blocksize>
Usage: asio_test client3 <host> <port> <threads> <totalcount> <sessions>
Usage: asio_test server3 <address> <port> <threads> <totalcount>
Usage: asio_test posttask1 <threads> <totalcount>
Usage: asio_test posttask2 <threads> <totalcount>
Usage: asio_test posttask3 <totalcount>
Usage: asio_test posttask4 <totalcount>
Usage: asio_test posttask5 <totalcount>

1，socketpair是测试socketpair的性能。

2，client1/server1 是固定长度的echo client/server。

3，client2/server2 是任意长度的echo client/server。

4，client3/server3 是带包头的echo client/server。

5，posttask1是多个线程排成一圈，依次给下一个线程post task，task为递增一个成员变量，直到递增到设定次数为止。

6，posttask2是偶数个线程两两分成多组，组内两个线程来回post task，task为递增一个成员变量（需要用atomic），直到递增到设定次数为止。

7，postask3是线程1向线程2发送指定数量的task。

8，postask4是线程1向线程2发送指定数量的task，但是并不真正发送这么多次，而是检查一个带锁的队列，如果队列不为空则直接插入不发送。

9，postask5是posttask4的改进版。队列直接保存task本身。这更接近真实情况。posttask4过于简化任务了。


[huyuguang@dtrans1 ~/code/asio]$ ./asio_test.exe posttask3 10000000
use time(us): 9077386
[huyuguang@dtrans1 ~/code/asio]$ ./asio_test.exe posttask4 10000000
use time(us): 638762
[huyuguang@dtrans1 ~/code/asio]$ ./asio_test.exe posttask5 10000000
use time(us): 4202412

posttask4是理想情况。posttask5比较接近真实情况。但实际上由于posttask5的实现是高度优化的，包括用了2个实现reserved的vector来回swap，因此我怀疑并不值得采用posttask5这样的上层优化。