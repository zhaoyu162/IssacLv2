using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Sockets;
using System.Net;
using System.IO;
using System.Threading;
using System.Collections.ObjectModel;
using System.ComponentModel;
using ZeroMQ;
using System.Configuration;

namespace DZHClient
{
    class TCPClient
    {
        internal TCPClient(Dictionary<string, StockEntity> dictRef)
        {
            _dtScrach = _dtScrach.AddHours(8);
            _dictSERef = (dictRef);
            _ipEndPoint = new IPEndPoint(IPAddress.Loopback, 9909);
            _tcpClient = new TcpClient();
            _tcpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
            _tcpClient.Client.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ExclusiveAddressUse, false);
            _tcpClient.ExclusiveAddressUse = false;
            _tcpClient.Client.Connect(_ipEndPoint);
            _tcpClient.Client.ReceiveBufferSize = 1024 * 1024 * 100;
            _tcpClient.Client.Blocking = true;
            for(int i = 0; i < 1; i++)
            {
                _thread = new Thread(new ParameterizedThreadStart(_RecvRoute));
                _thread.IsBackground = true;
                _thread.Start();
            }
        }

        internal event Action QuoteChanged;

        void _RecvRoute(object obj)
        {
            using (var tcpstm = _tcpClient.GetStream())
            {
                // WDPK, 五档盘口
                // SDPK, 十档盘口
                // ZBCJ，逐笔成交
                // ZBWT，逐笔委托
                // ZBDD，逐笔大单
                // BDPK，百档盘口
                // QQPK，期权盘口
                var readBuff = new byte[1024 * 1024*10];

                while (true)
                {
                    var rdbytes = tcpstm.Read(readBuff, 0, readBuff.Length);
                    var newbuff = readBuff.Take(rdbytes).ToArray();

                    var action = (Action)(() =>
                    {
                        try
                        {
                            _ResolveDatagram(newbuff);

                            if (_hsAltered.Any() && QuoteChanged != null)
                            {
                                QuoteChanged();
                            }
                        }
                        catch (ThreadAbortException)
                        {
                            //LogManager.LogMgr.WriteLog(LogManager.LogFile.Trace, "UDP recv aborted.");
                        }
                        catch (System.Exception ex)
                        {
                            //LogManager.LogMgr.WriteLog(LogManager.LogFile.Error, "UDP recv err:" + ex.Message);
                            writelog(DateTime.Now.ToString("yyyyMMdd") + "\\exception.txt", ex.Message);
                        }
                    });
                    action.BeginInvoke(null, null);
                }
            }
        }
        public void Start()
        {
            _thread.Start(null);
        }

        public void Stop()
        {
            _thread.Abort();
            _thread.Join();
        }

        void _ResolveDatagram(byte[] bytes)
        {
            using (var ms = new MemoryStream(bytes))
            using (var br = new BinaryReader(ms))
            {
                switch (br.ReadInt32())
                {
                    case 0:
                        _ResolveL1Quote(br);
                        break;
                    case 1:
                        _ResolveL2Quote(br);
                        break;
                    case 4:// 逐笔数据
                        _ResolveZhubiData(br);
                        break;
                    case 5: // 逐笔委托数据
                        _ResolveZhubiOrderData(br);
                        break;
                    case 6: // 高五档盘口数据
                        _ResolveHigh5Data(br);
                        break;
                    case 7: // 百档盘口
                        _ResolveBdPankou(br);
                        break;
                    case 8:
                        _ResolveBigorder(br);
                        break;
                    case 10:
                        _ResolveQQData(br);
                        break;
                }
            }
        }
        void _ResolveQQData(BinaryReader br)
        {
            int count = br.ReadInt32(); // qq数量

            for (var n = 0; n < count; n++)
            {
                br.ReadInt16();
                UInt32 unixTime = br.ReadUInt32();
                var code = Encoding.ASCII.GetString(br.ReadBytes(12)).ToLower().Split(new char[] { '\0' })[0];
                var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(32)).ToLower().Split(new char[] { '\0' })[0];

                br.ReadBytes(16);
                float fn = br.ReadSingle();
                float v = br.ReadSingle();
                br.ReadSingle();
                float bp1 = br.ReadSingle();
                br.ReadBytes(8);
                float bv1 = br.ReadSingle();
                br.ReadBytes(8);
                float sp1 = br.ReadSingle();
                br.ReadBytes(8);
                float sv1 = br.ReadSingle();

                br.ReadBytes(17 * 4);


                var log = "接收到QQ数据：" + name + "，time:" + _dtScrach.AddSeconds(unixTime).ToString("MM-dd HH:mm:ss") + ",最新价格：" + fn.ToString() + "总手：" + v.ToString() + " 买一：" + bp1.ToString() + "," + bv1.ToString() + " 卖一：" + sp1.ToString() + "," + sv1.ToString();
                writelog(DateTime.Now.ToString("yyyyMMdd") + "\\期权\\" + name + ".txt", log);
                if(code.ToLower() == "ho10000834")
                    Console.WriteLine(log);
            }
        }

        void _ResolveBigorder(BinaryReader br)
        {
            int count = br.ReadInt32(); // 大单数量
            var code = Encoding.ASCII.GetString(br.ReadBytes(16)).ToLower().Split(new char[] { '\0' })[0];
            var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(32)).ToLower().Split(new char[] { '\0' })[0];

            for (var n = 0; n < count; n++)
            {
                var packid = br.ReadUInt32();
                var orderid1_2 = br.ReadUInt32();// 0Xfffffff;
                var orderid2_2 = br.ReadUInt32();// & 0Xfffffff;
                var orderid1 = orderid1_2 & 0xfffffff;
                var orderid2 = orderid2_2 & 0xfffffff;
                var orderid1_1 = orderid1_2 >> 28;
                var orderid2_1 = orderid2_2 >> 28;

                var buyPrice = br.ReadUInt32();// 买价
                var sellPrice = br.ReadUInt32();// 卖价
                var buyvol = br.ReadInt32(); // 买单量
                var sellvol = br.ReadUInt32(); // 卖单量
            
                var log = "接收到大单数据：" + code + "," + name + "," + buyPrice.ToString() + "," + sellPrice.ToString() +"," + buyvol.ToString() + "," + sellvol.ToString() + ",序号:" + packid.ToString() + ",买单号：" + orderid1_1.ToString() + "|" + orderid1.ToString() + ",卖单号：" + orderid2_1.ToString() + "|" + orderid2.ToString();
                writelog(DateTime.Now.ToString("yyyyMMdd") + "\\逐笔大单\\" + code + ".txt", log);
                //Console.WriteLine(log);
            }

        }
        void _ResolveL1Quote(BinaryReader br)
        {
            if (_dictSERef == null) return;

            int count   =   br.ReadInt32();
            int size    =   br.ReadInt32();

            _hsAltered.Clear();
            for (int i = 0; i < count; i++ )
            {
                var code = Encoding.ASCII.GetString(br.ReadBytes(16)).ToLower().Substring(1).Split(new char[] { '\0' })[0];
                if (code.Length == 0)
                {
                    br.ReadBytes(170 - 16);
                    break ;
                }
                var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(16)).Split(new char[] { '\0' })[0];
                var factor = code.StartsWith("h510") || code.StartsWith("z160") ? 1000.0f : 100.0f;
                var pc = br.ReadInt32() / factor;
                var lh = br.ReadInt32() / factor;
                var ll = br.ReadInt32() / factor;

                StockEntity se = null;
                if (_dictSERef.ContainsKey(code))
                {
                    se = _dictSERef[code];
                }
                else
                {
                    se = new StockEntity() { 
                        UniqueCode = code, 
                        Code = code.Substring(1), 
                        Name = name};
                    _dictSERef[code] = se;
                }

                DateTime dt;
                lock(se)
                {
                    se.PreClose  = pc;
                    se.LimitHigh = lh;
                    se.LimitLow  = ll;

	                br.ReadBytes(2); // 忽略一个字段SHORT
	                dt      =   _dtScrach.AddSeconds(br.ReadUInt32());

	                se.Open     =   br.ReadInt32() / factor;
	                se.Highest  =   br.ReadInt32() / factor;
	                se.Lowest   =   br.ReadInt32() / factor;
	                se.Latest   =   br.ReadInt32() / factor;
	                var VOL     =   br.ReadUInt32()*100;
	                var VOL_old =   se.TotalVolume;
                    se.TotalVolume = VOL;

                    if (VOL > VOL_old)
                    {
                        se.DataInfoList.Add(new NewDataInfo()
                        {
                            Delta_num = (int)(VOL - VOL_old),
                            Close_num = (int)VOL,
                            Last_price = se.Latest,
                            Time = dt.ToString("hh:mm:ss")
                        });

                        if (se.DataInfoList.Count > 30)
                            se.DataInfoList.RemoveAt(0);
                    }

                    se.TotalAmount = (UInt64)br.ReadDouble();
                    se.TotalDealtCount = br.ReadUInt32();

	                for (int k = 0; k < 5; k++ ){ se.BidPairs[k].Price = br.ReadInt32() / factor; }
	                for (int k = 0; k < 5; k++) { se.BidPairs[k].Count = br.ReadInt32() * 100; }
	                for (int k = 0; k < 5; k++) { se.AskPairs[k].Price = br.ReadInt32() / factor; }
                    for (int k = 0; k < 5; k++) { se.AskPairs[k].Count = br.ReadInt32() * 100; }
                }

                var rn1 = br.ReadInt32();
                var rn2 = br.ReadInt32();

                _hsAltered.Add(se.UniqueCode);
                //if(se.UniqueCode.Contains("002796"))
                    writelog(DateTime.Now.ToString("yyyyMMdd") + "\\盘口\\" + se.Code + ".txt", string.Format("收到 {0} 卖一盘口:{1},{2}", se.Name, se.AskPairs[0].Price, se.AskPairs[0].Count));
                //writelog("dzh_log_pk.txt", string.Format("收到 {0} 买一盘口:{1},{2}", se.Name, se.BidPairs[0].Price, se.BidPairs[0].Count));

                //Console.ForegroundColor = ConsoleColor.Red;
                //Console.Write("new quote arrvies:");
                //Console.ForegroundColor = ConsoleColor.Green;
                //Console.Write(string.Format("{0} 最新价:{1} 时间：{2}\n", se.UniqueCode, se.Latest, dt.ToShortTimeString()));
            }
        }
        void _ResolveL2Quote(BinaryReader br)
        {
            if (_dictSERef == null) return;

            int count = br.ReadInt32();
            int size = br.ReadInt32();

            _hsAltered.Clear();
            for (int i = 0; i < count; i++ )
            {
                var code = Encoding.ASCII.GetString(br.ReadBytes(16)).ToLower().Substring(1).Split(new char[] { '\0' })[0];
                var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(16)).Split(new char[] { '\0' })[0];
                StockEntity se = null;
                if (_dictSERef.ContainsKey(code))
                    se = _dictSERef[code];
                else
                {
                    se = new StockEntity() { UniqueCode = code, Code = code.Substring(1), Name = name };
                    _dictSERef[code] = se;
                }

                br.ReadBytes(32); // ignore useless bytes.
                lock(se)
                {
	                for (int k = 0; k < 5; k++) { se.BidPairs[k + 5].Price = br.ReadSingle(); }
	                for (int k = 0; k < 5; k++) { se.BidPairs[k + 5].Count = (int)br.ReadSingle()*100; }
	                for (int k = 0; k < 5; k++) { se.AskPairs[k + 5].Price = br.ReadSingle(); }
	                for (int k = 0; k < 5; k++) { se.AskPairs[k + 5].Count = (int)br.ReadSingle()*100; }
                }

                if (count < 100)
                    _hsAltered.Add(se.UniqueCode);
            }
        }
        void _ResolveBdPankou(BinaryReader br)
        {
            int size = br.ReadInt32(); // 包体大小
            var code = Encoding.ASCII.GetString(br.ReadBytes(16)).ToLower().Split(new char[] { '\0' })[0];
            var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(32)).ToLower().Split(new char[] { '\0' })[0];

            int count = br.ReadInt32(); // 盘口档位数量，买+卖

            for(var n = 0; n < count; n++)
            {
                var levelNum = br.ReadInt32(); // 档级 负数为买盘，正数为卖盘
                var price = br.ReadInt32(); // 盘口价格
                var volume = br.ReadInt32();// 盘口委托量（股）
                var order_num = br.ReadInt32(); // 委托笔数
                var log = string.Format("接收到百档数据：{0},{1},价格：{2},量:{3},笔数{4},档位:{5}", code, name, price, volume, order_num, levelNum);
                writelog(DateTime.Now.ToString("yyyyMMdd") + "\\百档\\" + code + ".txt", log);
            }

            codeHashOfBdpk.Add(code);
            //Console.WriteLine("接收到百档盘口数据：" + name + ",已收到" + codeHashOfBdpk.Count.ToString() + "个股票数据");
        }

        HashSet<string> codeHash = new HashSet<string>();
        HashSet<string> codeHashOfOrders = new HashSet<string>();
        HashSet<string> codeHashOfHigh5 = new HashSet<string>();
        HashSet<string> codeHashOfBdpk = new HashSet<string>();
        void writelog(string savepath, string logstring = "")
        {
            Directory.CreateDirectory(Path.GetDirectoryName(savepath));
            try
            {
                StreamWriter log = new StreamWriter(savepath, true);
                log.WriteLine(DateTime.Now.ToString("yyyy-MM-dd HH:mm:ss.fff") + " " + logstring);
                log.Flush();
                log.Close();
            }
            catch(Exception ex)
            {

            }
         
        }

        void _ResolveZhubiData(BinaryReader br)
        {
            int count = br.ReadInt32();
            int nLatest = br.ReadInt32();
            int nPackIndex = br.ReadInt32(); // 包序列，从0开始，请按照这个序号进行排序重新组合全部的包
            var code = Encoding.ASCII.GetString(br.ReadBytes(16)).ToLower().Split(new char[] { '\0' })[0];
            var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(32)).ToLower().Split(new char[] { '\0' })[0];


            var bytes = br.ReadBytes(count * 12);
            using (var mbr = new MemoryStream(bytes))
            using(var reader = new BinaryReader(mbr))
            {
                //writelog("dzh_log.txt", "接收到逐笔数据开始");
                // 读取最新的
                writelog(DateTime.Now.ToString("yyyyMMdd") + "\\逐笔成交\\" + code + ".txt", "[最新]接收到逐笔数据：要求的大单数据起始序号：" + nPackIndex.ToString());
                for (var n = 0; n < count; n++)
                {
                    var time = reader.ReadUInt32();
                    var price = reader.ReadUInt32();
                    var vol = reader.ReadInt32();

                    var log = (nLatest == 0 ? "[历史]接收到逐笔数据：" : "[最新]接收到逐笔数据：") + code + "," + name + "," + _dtScrach.AddSeconds(time).ToString("MM-dd HH:mm:ss") + "," + price.ToString() + "," + vol.ToString();
                    writelog(DateTime.Now.ToString("yyyyMMdd") + "\\逐笔成交\\" + code + ".txt", log);
                    //Console.WriteLine(log);
                }

                //writelog("dzh_log.txt", "接收到逐笔数据结束");
                //for (var n = nLatest; n < count - nLatest; n++)
                //{
                //    var time = reader.ReadInt32();
                //    var price = reader.ReadSingle();
                //    var vol = reader.ReadInt32();
                //}
            }

            codeHash.Add(code);
            //Console.WriteLine("接收到逐笔数据：" + name + " 共" + count.ToString() + "笔！其中最新：" + nLatest.ToString() + "笔！" + "已收到" + codeHash.Count.ToString() + "个股票数据");
        }


        void _ResolveZhubiOrderData(BinaryReader br)
        {
            int size = br.ReadInt32();
            var code = Encoding.ASCII.GetString(br.ReadBytes(16)).ToLower().Split(new char[] { '\0' })[0];
            var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(32)).ToLower().Split(new char[] { '\0' })[0];

            br.ReadBytes(4); // 略过4个不用的字节，具体请参照c++的格式声明

            var details = "";

            var countOfInts = br.ReadInt32(); // 这个很关键，表示的是所有价格的委托队列总体占用的int个数，所以要根据这个判定是否读取到了最后。

            for (var n = 0; n < countOfInts; n++)
            {
                var price = br.ReadInt16();
                var bos = br.ReadInt16();
                var nVols = br.ReadInt32(); // 委托量个数，后面是一个连续的int数组
                //var nouse = br.ReadBytes(12);
                details = "价格:" + price.ToString() + ", 总笔数:" + nVols.ToString() + " 逐笔量：";
                for (var j = 0; j < nVols; j++)// 读取连续的数组元素
                {
                    var vol = br.ReadInt32(); // 读取某一个量
                    if (vol > 0)
                        details += vol.ToString() + ",";
                }

                writelog(DateTime.Now.ToString("yyyyMMdd") + "\\逐笔委托\\" + code + ".txt", string.Format("stock {0}, {1}", name, details));
                n += 2 + nVols; // 依据以上的计算，这里N前进了nVols+2个int的长度
            }
          
            //var order_numbers = br.ReadBytes(4 * 100);

            //Console.WriteLine("接收到逐笔委托：" + name + " 已收到" + codeHashOfOrders.Count.ToString() + "个股票逐笔委托数据！");
        }

        void _ResolveHigh5Data(BinaryReader br) // 高五档盘口数据解析
        {
            int size = br.ReadInt32();
            var code = Encoding.ASCII.GetString(br.ReadBytes(16)).ToLower().Split(new char[] { '\0' })[0];
            var name = Encoding.GetEncoding(936).GetString(br.ReadBytes(32)).ToLower().Split(new char[] { '\0' })[0];
            br.ReadBytes(29);
            var price = br.ReadSingle();
            br.ReadBytes(size - 48 - 4 - 33);
            codeHashOfHigh5.Add(code);
            var log = ("接收到高5档盘口数据：" + name + " 买六价：" + price.ToString());
            writelog(DateTime.Now.ToString("yyyyMMdd") + "\\高五档盘口\\" + code + ".txt", log);
        }

        TcpClient   _tcpClient    =     null;
        IPEndPoint  _ipEndPoint   =     null;
        DateTime    _dtScrach     =     new DateTime(1970, 1, 1);
        HashSet<string> _hsAltered=     new HashSet<string>();
        public HashSet<string> HsAltered
        {
            get { return _hsAltered; }
        }
        Dictionary<string, StockEntity> _dictSERef;
        Thread      _thread       =     null;
    }
}
