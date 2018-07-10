本行情源可以使用zeromq连接TCP端口获取行情：
zeromq支持很多种语言绑定，官方地址：http://zeromq.org
因此，无论哪种语言，请先安装zeromq依赖项，然后连接本机端口19908（默认，也可在dataengine.ini中修改）：
tcp://127.0.0.1:19908
即可接受数据。