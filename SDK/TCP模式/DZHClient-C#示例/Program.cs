using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DZHClient
{
    class Program
    {
        static void Main(string[] args)
        {
            //DZHClient dzh = new DZHClient(new Dictionary<string, StockEntity>());
            SdkReceiver sr = new SdkReceiver();

            sr.RecvMutithread(20);

            //dzh.Start();
            Console.Read();
        }
    }
}
