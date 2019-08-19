using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;

namespace DZHClient
{
    public class MateSdk
    {
        [DllImport("MateSDK.dll", CallingConvention=CallingConvention.Cdecl, CharSet=CharSet.Ansi)]
        public extern static IntPtr ConnectServer(string strServer);

        [DllImport("MateSDK.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public extern static int Subscribe(IntPtr ptrZMQ, string strChannel);

        [DllImport("MateSDK.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public extern static int GetData(IntPtr ptrZMQ, StringBuilder sb, int sbLen);

        [DllImport("MateSDK.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.None)]
        public extern static int Disconnect(IntPtr ptrZMQ, string strServer);

    }

    public class SdkReceiver
    {
        public SdkReceiver()
        {
            socket_ptr = MateSdk.ConnectServer("tcp://127.0.0.1:19908");
            MateSdk.Subscribe(socket_ptr,"");
        }

        DateTime _dtScrach = new DateTime(1970, 1, 1).AddHours(8);
        string _rootDir = null;
        IntPtr socket_ptr = IntPtr.Zero;

        void CreateIfNotExist(string dir)
        {
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);
        }

        public void RecvMutithread(int nThreadNum)
        {
            //Recv();
            for (int n = 0; n < nThreadNum; n++)
            {
                (new Action(() => { Recv(); })).BeginInvoke(null, null);
            }
        }
        public void Recv()
        {
            StringBuilder sb = new StringBuilder(1024 * 1024 * 10);

            while (true)
            {
                sb.Clear();
                var nr = MateSdk.GetData(socket_ptr,sb, sb.Capacity);

                if (nr == 100)
                    continue;
                else if (0 != nr)
                {
                    // need reconnect server....
                    break;
                }
                else
                {
                    //Console.WriteLine(sb.ToString());
                    var joo = JArray.Parse(sb.ToString());
                    for (int n = 0; n < joo.Count; n++)
                    {
                        var jo = joo[n];
                        var qt = jo["QuoteType"].ToString();
                        if (qt == "WDPK")
                        {
                            jo["Stocks"].ToList().ForEach(stock =>
                            {
                                var ntime = stock["Time"].ToObject<int>();
                                if (ntime > 0)
                                {
                                    var date = _dtScrach.AddSeconds(ntime).Date.ToString("yyyyMMdd");
                                    CreateIfNotExist(date);
                                    CreateIfNotExist(date + "/五档盘口");

                                    _rootDir = date;

                                    File.AppendAllText(date + "/五档盘口/" + stock["StockCode"] + ".txt", stock.ToString() + "\r\n");
                                }
                            });
                        }
                        else if (qt == "SDPK")
                        {
                            if (null != _rootDir)
                            {
                                CreateIfNotExist(_rootDir + "/高五档盘口");
                                File.AppendAllText(_rootDir + "/高五档盘口/" + jo["StockCode"] + Thread.CurrentThread.ManagedThreadId.ToString()+ ".txt", jo.ToString() + "\r\n");
                            }
                        }
                        else if (qt == "ZBCJ")
                        {
                            if (null != _rootDir)
                            {
                                CreateIfNotExist(_rootDir + "/逐笔成交");
                                File.AppendAllText(_rootDir + "/逐笔成交/" + jo["StockCode"] + Thread.CurrentThread.ManagedThreadId.ToString()+ ".txt", jo.ToString() + "\r\n");
                            }
                        }
                        else if (qt == "ZBWT")
                        {
                            if (null != _rootDir)
                            {
                                CreateIfNotExist(_rootDir + "/委托队列");
                                File.AppendAllText(_rootDir + "/委托队列/" + jo["StockCode"] + Thread.CurrentThread.ManagedThreadId.ToString()+ ".txt", jo.ToString() + "\r\n");
                            }
                        }
                        else if (qt == "BDPK")
                        {
                            if (null != _rootDir)
                            {
                                CreateIfNotExist(_rootDir + "/深圳百档盘口");
                                File.AppendAllText(_rootDir + "/深圳百档盘口/" + jo["StockCode"] + Thread.CurrentThread.ManagedThreadId.ToString()+ ".txt", jo.ToString() + "\r\n");
                            }
                        }
                        else if (qt == "ZBDD")
                        {
                            if (null != _rootDir)
                            {
                                CreateIfNotExist(_rootDir + "/逐笔成交委托单");
                                File.AppendAllText(_rootDir + "/逐笔成交委托单/" + jo["StockCode"] + Thread.CurrentThread.ManagedThreadId.ToString()+ ".txt", jo.ToString() + "\r\n");
                            }
                        }
                        else if (qt == "SZQQ")
                        {
                            if (null != _rootDir)
                            {
                                CreateIfNotExist(_rootDir + "/期权");
                                File.AppendAllText(_rootDir + "/期权/" + jo["StockCode"] + Thread.CurrentThread.ManagedThreadId.ToString()+ ".txt", jo.ToString() + "\r\n");
                            }
                        }
                    }

                }
            }
        }
    }
}
