//////////////////////////////////////////////////////////////////////////
// 二次开发数据包格式定义
// 本系统是将行情以UDP格式方式定点发送到一台机器上（可配置IP),二次开发只需要监听12415端口就可以收到行情
// 然后按照本文档所定义的行情格式解析数据包就可以收到行情
//////////////////////////////////////////////////////////////////////////
#pragma once
#pragma pack(push, 1)
#include <vector>

// 五档行情数据结构
typedef struct QuoteL1
{
	short		usNumber;
	long		dwTim32; // time 行情时间
	long		lgOpen;  // 开盘价
	long		lgHigh;  // 最高价
	long		lgLow;   // 最低价
	long		lgNew;	 // 最新价
	long		lgAllVol;// 成交总量
	union
	{
		struct{
			long		lgAllAmount; // 成交金额-还未解密，客户不理会
			long		lgNeipan;	 // 未解密内盘量，客户不理会
		}DW;
		double dlbAmount;			 // 解密后成交金额，客户使用
	};
	//long		lgAllAmount; // 成交金额-还未解密
	//double		dblAmount;	 // 解密后
	//long		lgMarketVal; // 总市值
	long		lgTotalDealtNum; // 总成交笔数
	long		lgPricesOfBid[5]; // 五档买盘价格
	long		lgVolumeOfBid[5]; // 五档买盘量
	long		lgPricesOfAsk[5]; // 五档卖盘价格
	long		lgVolumeOfAsk[5]; // 五档卖盘量
	//long		lgZero[2];
	//double		dblNeipan;
	long		lgNeipan;	// 解密后内盘量，客户使用
	long		lgZero;
}QL1, *LPQL1;

typedef struct OrderDetail			// 逐笔委托明细结构
{						
	char		market[2];			// 市场，如SH,SZ
	short		usNumber;			// 股票内部编号，不用考虑
	int			nNumOfOrders;		// orders data的总长度
	int			nOrdersData[0];		// 委托队列数据，注意这个数组的解析方式为： 【WORD(价格）+WORD(正负，正为买盘，负数为卖盘）+INT(委托量的数量）+INT...(委托量数组）】，然后重复【】里面的结构，直到
}ORDERDETAIL, *LPORDERDETAIL;	

// 十档行情数据结构
typedef struct QuoteL2
{
	char		cMarket[2];
	short		usNumber;
	float		fltBuyEven;
	float		fltBuyVol;
	float		fltSellEven;
	float		fltSellVol;
	float		fltPricesOfBid[5];
	float		fltVolumeOfBid[5];
	float		fltPricesOfAsk[5];
	float		fltVolumeOfAsk[5];

}QuoteL2, *LPQuoteL2;
template<typename T>
struct _Datagram
{
	int  nDgType;
	UINT nCount;
	UINT nSize;
	struct Ext
	{
		char cCode[16];
		char cName[16];
		UINT nPrevClose;
		UINT nTopLtd;
		UINT nBotLtd;
		T	 quote;
	};

	Ext	extData[0];
};

template<typename T>
struct _DataPackage
{
	int nDgType;
	UINT nCount;
	UINT nSize;
	T date_t[0];
};

// 短线精灵数据结构
typedef struct MarketBoardInfo{
	char	strCode[16];	//股票代码
	char	strInfo[48];	//短线消息
}*LPMarketBoardInfo;

typedef struct _ZHUBIDATA_
{
	UINT32		lgTime;		//逐笔成交时间
	UINT32		fltPrice;	//逐笔成交价格
	INT			lgVolume;	//逐笔成交量
}*LPZHUBIDATA, ZHUBIDATA;

typedef struct _BIGORDER_	// 大单数据
{
	UINT32		unOrderPackId;	// 大单数据包ID，用来跟逐笔成交数据匹配的
	UINT32		unBuyOrderId;	// 买单大单ID,高字节是一个标志，低位3字节表示单号，同一个大单被拆分成多个小单成交时大单ID相同，请客户自己拆分解析
	UINT32		unSellOrderId;	// 卖单大单ID,高字节是一个标志，低位3字节表示单号，同一个大单被拆分成多个小单成交时大单ID相同，请客户自己拆分解析
	UINT32		unBuyPrice;		// 买单价格
	UINT32		unSellPrice;	// 卖单价格
	UINT32		unBuyVol;	// 买单量
	UINT32		unSellVol;	// 卖单量
}*LPBIGORDER, BIGORDER;

typedef struct _ZHUBILIST_
{
	CHAR		cMarket[2];
	USHORT		usNumber;
	DWORD		dwUnknown1;
	DWORD		dwUnknown2;
	ZHUBIDATA	zbDataList[0];
}*LPZHUBILIST, ZHUBILIST;

// 百档盘口详细
typedef struct _MULTI_PANKOU_DETAIL_
{
	int	  sLevelNum;				// 档级：负数为买盘档，正数为买盘档
	int	  nPrice;					// 盘口价格
	int   nVolume;					// 盘口委托量
	int	  nOrderNum;				// 委托笔数
}MULTI_PANKOU_DETAIL, *LPMULTI_PANKOU_DETAIL;

// 
typedef struct _ZHUBIDATAGRAM_
{
	int		nDgType;	   // 逐笔成交数据包类型：4
	UINT	nCount;
	UINT	nLastestCount; // 表示zbDataList的最新逐笔条数，从0开始！
	UINT	nOrderPackId;
	char	strCode[16];
	char	strName[32];
	ZHUBIDATA	zbDataList[0];
}*LPZHUBIDATAGRAM, ZHUBIDATAGRAM;

typedef struct _ZHUBI_ORDER_DATAGRAM_
{
	int			nDgType; // 5		逐笔委托数据包类型：5
	int			nSize;		//		整包大小，包括前面的nDgType	
	char		strCode[16];//		股票代码
	char		strName[32];//		股票名称
	ORDERDETAIL	orddObj;	//		委托明细
}ZHUBI_ORDER_DATAGRAM, *LPZHUBI_ORDER_DATAGRAM;

typedef struct _BIG_ORDER_DATAGRAM_
{
	int			nDgType; // 8		大单数据包类型：8
	int			nCount; //			大单数量
	char		strCode[16];//		股票代码
	char		strName[32];//		股票名称
	BIGORDER	bigOrders[0];	//	大单数据		
}*LPBIGORDERDATAGRAM, BIGORDERDATAGRAM;

typedef struct _MULTI_PANKOU_DATAGRAM_
{
	int			nDlgType;// 6		百档盘口
	int			nSize;	//			整包大小
	char		strCode[16];//		股票代码
	char		strName[32];//		股票名称
	int			nLevelNum;	//		总档数
	MULTI_PANKOU_DETAIL pkData[0];
}MULTI_PANKOU_DATAGRAM, *LPMULTI_PANKOU_DATAGRAM;

typedef struct _HIGH_LEVEL_QUOTE
{
	char	cUnknownChars[9];		//未知字节，忽略
	char	market[2];				//市场
	short	usNumber;				//股票内部编码，不用考虑
	float	fltAvgPriceBid;			//委买均价
	float	fltTotalVolBid;			//总买总量
	float	fltAvgPriceAsk;			//委卖均价
	float	fltTotalVolAsk;			//委卖总量
	float	fltBidPrices[5];		//买六价到买10价
	float	fltBidVolume[5];		//买六量到买10量
	float	fltAskPrices[5];		//卖六价到卖10价
	float	fltAskVolume[5];		//卖六量到卖10量
}HIGH_LEVEL_QUOTE, *LPHIGH_LEVEL_QUOTE;



typedef struct _HIGH_LVQUOTE_DATAGRAM_
{
	int			nDgType;	// 		高五档盘口数据包类型：6
	int			nSize;		//		整包大小，包括前面的nDgType	
	char		strCode[16];//		股票代码
	char		strName[32];//		股票名称
	HIGH_LEVEL_QUOTE	quoteObj;	//		高五档数据
}HIGH_LVQUOTE_DATAGRAM, *LPHIGH_LVQUOTE_DATAGRAM;

typedef struct _QIQUAN_DATA_
{
	short		sUnknown;
	unsigned	unTime;	   	//	UNIX时间戳
	char		code[12];  	//	代码
	char		name[32];	//	名称
	float		fLastClosePrice;	// 昨收
	float		fThisOpenPrice;		// 今开
	float		fHighPrice;			// 最高
	float		fLowPrice;			// 最低
	float		fNewPrice;			// 最新
	float		fTotalLots;			// 总手
	float		fTotalAmonut;		// 总额
	float		fBuyPrice1;			// 买一价
	float		fBuyPrice2;			// 买二价
	float		fBuyPrice3;
	float		fBuyVol1;			// 买一量
	float		fBuyVol2;
	float		fBuyVol3;

	float		fSellPrice1;		// 买一价
	float		fSellPrice2;		// 卖二价
	float		fSellPrice3;
	float		fSellVol1;			// 卖一量
	float		fSellVol2;
	float		fSellVol3;

	float		fBuyPrice4;			// 买四价
	float		fBuyVol4;
	float		fSellPrice4;		// 卖四价
	float		fSellVol4;

	float		fBuyPrice5;			// 买五价
	float		fBuyVol5;
	float		fSellPrice5;		// 卖五价
	float		fSellVol5;

	float		fUnknown;
	float		fTotalBuyLots;		// 总买手
	float		fTopPrice;			// 涨停
	float		fBottomPrice;		// 跌停
	float		fTotalPositions;	// 总持仓
	float		fUnknow2;
	float		fUnknow3;
}QIQUAN_DATA, *LPQIQUAN_DATA;

typedef struct _QIQUAN_DATA_BLOCK_
{
	int		nDgType;	   // 期权数据包类型：10
	UINT	nCount;
	QIQUAN_DATA buff[0];
}QIQUAN_DATA_BLOCK, *LPQIQUAN_DATA_BLOCK;

typedef struct _KLINE_DATA_
{
	int unixDt;
	float open;
	float high;
	float low;
	float close;
	float volume;
	float amount;
	unsigned unknown;
}KLINEDATA,*LPKLINEDATA;
typedef struct _KLINE_DATA_BLOCK_
{
	int		nDgType;		// k线数据包类型：11
	UINT	nCount;
	char	strCode[16];//		股票代码
	char	strName[32];//		股票名称
	KLINEDATA data[0];
};
//////////////////////////////////////////////////////////////////////////
// 本段的定义为二次开发准备
typedef const LPQuoteL2	   LPCQuoteL2;
typedef _Datagram<QuoteL1> L1DataGram;					// 接收方使用的五档行情数据格式
typedef _Datagram<QuoteL2> L2DataGram;					// 接收方使用的十档行情数据格式
typedef ZHUBIDATAGRAM ZhubiDataGram;					// 接收方使用的逐笔成交数据格式
typedef _ZHUBI_ORDER_DATAGRAM_ ZhubiOrderDataGram;		// 接收方使用的逐笔委托数据格式
typedef HIGH_LVQUOTE_DATAGRAM HightLvQuoteDataGram;		// 接收方使用的高五档盘口数据
typedef QIQUAN_DATA_BLOCK QiQuanDataGram;				// 接收方使用的期权数据格式
typedef _KLINE_DATA_BLOCK_ KLINEDATABLOCK;				// 接收方使用的K线数据格式
//////////////////////////////////////////////////////////////////////////
// 大智慧软件五档行情数据块结构，内部使用，第三方不用涉及
typedef struct FreeDataBlock
{
	short		usReserved;
	USHORT		usQuoteCount;
	QuoteL1		ql1Block[0];
}FreeDataBlock, *LPFreeDataBlock;

typedef const LPFreeDataBlock LPCFreeDataBlock;

typedef _DataPackage<MarketBoardInfo> DataPackage_MBI;  // 接收方使用的短线精灵数据格式，如UDP包。

// 大智慧软件五档行情数据块结构，内部使用，第三方不用涉及
typedef struct StockSignature
{
	short		usNumber;
	char		lpszCode[10];
	char		lpszName[20];
	char		chrReserved[24];
	UINT		nPrevClose;
	UINT		nTopLtd;
	UINT		nBotLtd;
}StockSig, *LPStockSig;

// 大智慧软件五档行情数据块结构，内部使用，第三方不用涉及
typedef struct ShortStock
{
	char		cMarket[2];
	WORD		wdNumber;
	WORD		wdReserved3;
}*LPShortStock, ShortStock;

// 大智慧软件五档行情数据块结构，内部使用，第三方不用涉及
typedef struct L2RequestParam
{
	WORD		wdRqSig;
	char		pcReserved1[5];
	DWORD		dwReserved2;
	WORD		wdStockCount;
	ShortStock sMarket[0];

	//L2RequestParam(){
	//	memset(this, 0, sizeof(L2RequestParam));
	//	
	//}

	LPVOID GetSubParam(){return &dwReserved2;}
}*LPL2RequestParam;

// 逐笔委托数据结构
typedef struct OrderDetailBlock
{
	WORD	wdPrice;
	SHORT	buyOfSell;	//正数为买盘，负数为卖盘
	int	 nNumber;
	int  nVolArray[0];
}OrderDetailBlock;

// 逐笔委托数据内部结构，第三方不涉及
typedef struct OrderDetailHeader
{
	WORD wsNumber;
	WORD wdUnknown1;
	OrderDetailBlock odb;
}OrderDetailHeader;

typedef struct _FREEQUOTE_
{
	short		usNumber;
	long		dwTim32; // time
	long		lgOpen;
	long		lgHigh;
	long		lgLow;
	long		lgNew;
	long		lgAllVol;
	long		lgAllAmount;
	long		lgMarketVal;
	long		lgReserver2;
	long		lgPricesOfBid[5];
	long		lgVolumeOfBid[5];
	long		lgPricesOfAsk[5];
	long		lgVolumeOfAsk[5];
	long		lgZero[2];
	char		cMarket[2];
}*LPFREEQUOTE, FREEQUOTE;
#pragma pack(pop)