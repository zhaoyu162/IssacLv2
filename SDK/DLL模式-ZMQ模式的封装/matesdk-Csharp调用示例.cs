using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;

namespace DZHClient
{
    public class MateSdk
    {
        [DllImport("MateSDK.dll", CallingConvention=CallingConvention.Cdecl, CharSet=CharSet.Ansi)]
        public extern static int ConnectServer(string strServer);

        [DllImport("MateSDK.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public extern static int Subscribe(string strChannel);

        [DllImport("MateSDK.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public extern static int GetData(StringBuilder sb, int sbLen);

        [DllImport("MateSDK.dll", CallingConvention = CallingConvention.Cdecl, CharSet = CharSet.Ansi)]
        public extern static int Disconnect(string strServer);

    }

    public class SdkReceiver
    {
        public SdkReceiver()
        {
            MateSdk.ConnectServer("tcp://127.0.0.1:19908");
            MateSdk.Subscribe("");
        }

        DateTime _dtScrach = new DateTime(1970, 1, 1).AddHours(8);
        string _rootDir = null;

        void CreateIfNotExist(string dir)
        {
            if (!Directory.Exists(dir))
                Directory.CreateDirectory(dir);
        }
        public void Recv()
        {
            StringBuilder sb = new StringBuilder(1024 * 1024 * 10);

            while (true)
            {
                sb.Clear();
                if (0 != MateSdk.GetData(sb, sb.Capacity))
                {
                    // need reconnect server....
                    break;
                }
                else
                {
                    //Console.WriteLine(sb.ToString());
                    var jo = JObject.Parse(sb.ToString());
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
                                CreateIfNotExist(date+"/五档盘口");

                                _rootDir = date;

                                File.AppendAllText(date + "/五档盘口/" + stock["StockCode"] + ".txt", stock.ToString() + "\r\n");
                            }
                        });
                    }
                    else if(qt == "SDPK")
                    {
                        if(null != _rootDir)
                        {
                            CreateIfNotExist(_rootDir + "/高五档盘口");
                            File.AppendAllText(_rootDir + "/高五档盘口/" + jo["StockCode"] + ".txt", jo.ToString() + "\r\n");
                        }
                    }
                    else if(qt == "ZBCJ")
                    {
                        if (null != _rootDir)
                        {
                            CreateIfNotExist(_rootDir + "/逐笔成交");
                            File.AppendAllText(_rootDir + "/逐笔成交/" + jo["StockCode"] + ".txt", jo.ToString() + "\r\n");
                        }
                    }
                    else if (qt == "ZBWT")
                    {
                        if (null != _rootDir)
                        {
                            CreateIfNotExist(_rootDir + "/委托队列");
                            File.AppendAllText(_rootDir + "/委托队列/" + jo["StockCode"] + ".txt", jo.ToString() + "\r\n");
                        }
                    }
                    else if (qt == "BDPK")
                    {
                        if (null != _rootDir)
                        {
                            CreateIfNotExist(_rootDir + "/深圳百档盘口");
                            File.AppendAllText(_rootDir + "/深圳百档盘口/" + jo["StockCode"] + ".txt", jo.ToString() + "\r\n");
                        }
                    }
                    else if (qt == "ZBDD")
                    {
                        if (null != _rootDir)
                        {
                            CreateIfNotExist(_rootDir + "/逐笔成交委托单");
                            File.AppendAllText(_rootDir + "/逐笔成交委托单/" + jo["StockCode"] + ".txt", jo.ToString() + "\r\n");
                        }
                    }
                    else if (qt == "SZQQ")
                    {
                        if (null != _rootDir)
                        {
                            CreateIfNotExist(_rootDir + "/期权");
                            File.AppendAllText(_rootDir + "/期权/" + jo["StockCode"] + ".txt", jo.ToString() + "\r\n");
                        }
                    }
                }
            }
        }
    }
}
