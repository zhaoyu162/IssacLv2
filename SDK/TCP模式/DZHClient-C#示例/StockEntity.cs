using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Collections.ObjectModel;
using System.ComponentModel;

namespace DZHClient
{
    //最新值
    public class NewDataInfo
    {
        public int close_num;//此次总量
        public int Close_num
        {
            get { return close_num; }
            set { close_num = value; }
        }
        public int delta_num;//此次增量
        public int Delta_num
        {
            get { return delta_num; }
            set { delta_num = value; }
        }
        public double last_price;
        public double Last_price
        {
            get { return last_price; }
            set { last_price = value; }
        }
        public string time;
        public string Time
        {
            get { return time; }
            set { time = value; }
        }

        public List<ZhubiData> ZhubiDataList { get; set; }
    }
    //历史记录
    public class NewDataInfoList
    {
        public List<NewDataInfo> DataInfoList { get; set; }
    }

    public class ZhubiData
    {
        public UInt32 Time { get;set;}   // 毫秒数，from 1970
        public float Price { get; set; }  // 价格
        public int Volumn { get; set; }     // 成交量
    }
    public class StockEntity : INotifyPropertyChanged
    {
        public event PropertyChangedEventHandler PropertyChanged;

        /// <summary>
        /// Price&Volume for ask&bid panels.
        /// </summary>
        public class PCPair : INotifyPropertyChanged
        {
            public event PropertyChangedEventHandler PropertyChanged;

            float _price;
            public float Price
            {
                get { return _price; }
                set {  
                    _price = value;
                    if (null != PropertyChanged)
                        PropertyChanged(this, new PropertyChangedEventArgs("Price"));
                }
            }
            int _count;
            public int Count
            {
                get { return _count; }
                set { _count = value;
                    if (null != PropertyChanged)
                        PropertyChanged(this, new PropertyChangedEventArgs("Count"));
                }
            }

            int _index;
            public int Index
            {
                get { return _index; }
                set { _index = value;
                    if (null != PropertyChanged)
                        PropertyChanged(this, new PropertyChangedEventArgs("Index"));
                }
            }
        }

        public StockEntity()
        {
            for (int i = 0; i < 10; i++)
            {
                _askPairs.Add(new PCPair { Price = 0f, Count = 0, Index = i });
                _bidPairs.Add(new PCPair { Price = 0f, Count = 0, Index = i });
            }

            DataInfoList = new List<NewDataInfo>();
        }

        public StockEntity(string code) : this()
        {
        }

        public int DigitCount { 
            get 
            {
                if (!string.IsNullOrEmpty(UniqueCode) &&
                    (UniqueCode.StartsWith("h510") || UniqueCode.StartsWith("z160")))
                {
                    return 3;
                }

                return 2;
            } 
        }
        string _code;
        public string Code
        {
            get { return _code; }
            set { _code = value; }
        }
        string _uniqueCode;
        public string UniqueCode
        {
            get { return _uniqueCode; }
            set { _uniqueCode = value; }
        }
        string _name;
        public string Name
        {
            get { return _name; }
            set { _name = value; }
        }
        float  _preClose;
        public float PreClose
        {
            get { return _preClose; }
            set { _preClose = value; 
                _FirePropertyChanged("PreClose"); }
        }
        float  _open;
        public float Open
        {
            get { return _open; }
            set { _open = value; _FirePropertyChanged("Open"); }
        }
        float  _highest;
        public float Highest
        {
            get { return _highest; }
            set { _highest = value; _FirePropertyChanged("Highest"); }
        }
        float  _lowest;
        public float Lowest
        {
            get { return _lowest; }
            set { _lowest = value; _FirePropertyChanged("Lowest"); }
        }
        float  _latest;
        public float Latest
        {
            get { return _latest; }
            set { _latest = value; _FirePropertyChanged("Latest"); }
        }
        float  _limitHigh;
        public float LimitHigh
        {
            get { return _limitHigh; }
            set { _limitHigh = value; _FirePropertyChanged("LimitHigh"); }
        }
        float  _limitLow;
        public float LimitLow
        {
            get { return _limitLow; }
            set { _limitLow = value; _FirePropertyChanged("LimitLow"); }
        }
        int    _lentOfBroker;
        public int LentOfBroker
        {
            get { return _lentOfBroker; }
            set { _lentOfBroker = value; _FirePropertyChanged("LentOfBroker"); }
        }
        int    _lengOfMe;
        public int LengOfMe
        {
            get { return _lengOfMe; }
            set { _lengOfMe = value; _FirePropertyChanged("LentOfMe"); }
        }

        public UInt32 TotalDealtCount { get; set; }
        public UInt32 TotalVolume { get; set; }
        public UInt64 TotalAmount { get; set; }
        ObservableCollection<PCPair> _askPairs = new ObservableCollection<PCPair>();
        public ObservableCollection<PCPair> AskPairs
        {
            get { return _askPairs; }
            set { _askPairs = value; }
        }
        ObservableCollection<PCPair> _bidPairs = new ObservableCollection<PCPair>();
        public ObservableCollection<PCPair> BidPairs
        {
            get { return _bidPairs; }
            set { _bidPairs = value; }
        }

        public List<NewDataInfo> DataInfoList { get; set; }

        private void _FirePropertyChanged(string strPropName)
        {
            if (null != PropertyChanged)
                PropertyChanged(this, new PropertyChangedEventArgs(strPropName));
        }
    }
}
