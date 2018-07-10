#!/usr/bin/env python
# -*- coding: gb2312 -*-

import time
import zmq
import struct
import numpy as np
import threading
import urllib2
import random

class ThsMarket():
    ZMQ_HOST = 'tcp://127.0.0.1:19908'
    SUBSCRIBE_URI = 'http://localhost:10010/requestqx?list='

    _context = None
    _socket = None
    _recv_thread = None
    _recv_run_flag = False

    parse_buy_1_queue_only = True

    def __init__(self):
        pass

    def __del__(self):
        self.disconnect()

    # 连接ZeroMQ
    def connect(self):
        self.disconnect()
        self._context = zmq.Context()
        self._socket = self._context.socket(zmq.SUB)
        self._socket.connect(self.ZMQ_HOST)
        '''
        可订阅的主题：
            WDPK = Level1 五档盘口(无效）
            SDPK = Level2 十档盘口（无效）
            ZBCJ = 逐笔成交
            ZBWT = 逐笔委托（全息盘口）
            BDPK = 百档盘口
            ZBDD = 逐笔大单
            SZQQ = 期权
        '''
        self._socket.setsockopt(zmq.SUBSCRIBE, '')

        # self._recv_run_flag = True
        # self._recv_thread = threading.Thread(target=self._recv_loop)
        # self._recv_thread.start()

    # 断开连接
    def disconnect(self):
        # if self._recv_thread is not None:
        #     self._recv_run_flag = False
        #     self._recv_thread.join()
        #
        if self._socket is not None:
            self._socket.disconnect(self.ZMQ_HOST)
            self._socket = None
            self._context.destroy()
            self._context = None

    # 订阅传入的股票，股票代码格式为SZ000001, SH600858..etc.
    def subscribe(self, security_list):
        send_headers = {
            'Cache-Control': 'no-cache'
        }

        param = ','.join(security_list)
        url = self.SUBSCRIBE_URI + param
        url += '&rnd=' + str(int(random.random() * 1000000000))
        req = urllib2.Request(url, headers=send_headers)
        try:
            response = urllib2.urlopen(req, timeout=10)
            response.read()
            return True
        except:
            return False

    # 解析收到的消息
    def _parse(self, msg):
        msg_type = struct.unpack('i', msg[:4])[0]
        #print 'msg_type', msg_type
        # 5档盘口(废弃）
        if msg_type == 0:
            pass
        # 10档盘口(废弃）
        elif msg_type == 1:
            pass
        # 逐笔成交数据
        elif msg_type == 4:
            self._parse_transaction(msg)
        # 逐笔委托数据(全息盘口)
        elif msg_type == 5:
            self._parse_order_queue(msg)
        # 高五档盘口数据
        elif msg_type == 6:
            pass
        # 百档盘口
        elif msg_type == 7:
            self._parse_quotes(msg)
        # 逐笔大单数据
        elif msg_type == 8:
            self._parse_big_order(msg)
        # 期权数据
        elif msg_type == 10:
            pass

    '''
    解析买卖队列
    注意：
        如果parse_buy_1_queue_only=True则只解析买一队列
    返回:
        code - 股票代码
        buy_queue - 买队列的数组
        sell_queue - 卖队列的数组，买卖队列数组的item为tuple (price, vol_num, vol_list)
            price - 委托价（单位：分）
            vol_num - 委托个数
            vol_list - 委托列表（tuple类型）
    '''
    def _parse_order_queue(self, msg):
        list_start = 64
        block_size, code, name, un_used, count_of_ints = struct.unpack('1I16s32s2I', msg[4:list_start])
        # print block_size, code, count_of_ints

        buy_queue = []
        sell_queue = []
        read_ints = 0
        while read_ints < count_of_ints:
            # buy 16384, sell 32768
            price, buy_or_sell, vol_num = struct.unpack('1H1h1I', msg[list_start:list_start + 8])
            # print price, buy_or_sell, vol_num
            list_start += 8
            list_end = list_start + vol_num * 4
            vol_list = struct.unpack(str(vol_num) + 'I', msg[list_start: list_end])
            list_start = list_end
            read_ints += 2 + vol_num

            if buy_or_sell == 16384:
                buy_queue.append((price, vol_num, vol_list,))
            else:
                sell_queue.append((price, vol_num, vol_list,))

            if self.parse_buy_1_queue_only:
                print 'order queue', len(vol_list), vol_list[:10]
                break
        return code, buy_queue, sell_queue

    '''
    解析百档盘口
    返回:
        code - 股票代码, SZ000001, SH600858..etc.
        buy_quotes - 买档数据, 数组
        sell_quotes - 卖档数据， 数组
            买卖档数据item为tuple类型(level_num, price, volume, order_num)
                level_num - 档位，正数表示卖档，负数表示买档(奇怪的表示方式，注意不要搞反）
                price - 委托价格（单位：分）
                volume - 委托数量（单位：股）
                order_num - 委托笔数
    '''
    def _parse_quotes(self, msg):
        list_start = 60
        block_size, code, name, count = struct.unpack('1I16s32s1I', msg[4:list_start])

        buy_quotes = []
        sell_quotes = []
        for i in range(count):
            quote = struct.unpack('1i3I', msg[list_start: list_start + 16])
            if quote[0] > 0:
                buy_quotes.append(quote)
            else:
                sell_quotes.append(quote)
            list_start += 16
        print 'buy sell quotes', code, buy_quotes[:1], sell_quotes[:1]
        return code, buy_quotes, sell_quotes

    '''
    解析逐笔成交(注意：同样的包会收到两次）
    返回：
        code - 
        count - 成交记录数
        pack_index - 包序列，从0开始，请按照这个序号进行排序重新组合全部的包
        transactions - 成交数据列表，item为元组(time, price, vol)
            time - 成交时间
            price - 成交价(单位：分）
            vol - 成交量（单位：股）
    '''
    def _parse_transaction(self, msg):
        list_start = 64
        count, latest_num, pack_index, code, name = struct.unpack('3I16s32s', msg[4:list_start])

        transactions = []
        for i in range(count):
            t = struct.unpack('2I1i', msg[list_start: list_start+12])
            transactions.append(t)
            list_start += 12
        print 'detail transaction', code, count, pack_index, transactions[-3:]
        return code, count, pack_index, transactions

    '''
    解析逐笔大单数据
        每次最多返回1000条，当数量小于1000条时为最新逐笔大单成交数据
    返回：
        code - 股票代码
        count - 大单个数
        big_orders - 大单列表
            大单列表的time为tuple(pack_id, buy_order_id, sell_order_id, buy_price, sell_price, buy_vol, sell_vol) 
    '''
    def _parse_big_order(self, msg):
        list_start = 56
        count, code, name = struct.unpack('1I16s32s', msg[4:list_start])

        big_orders = []
        for i in range(count):
            big_order = struct.unpack('5I1i1I', msg[list_start: list_start + 28])
            big_orders.append(big_order)
            list_start += 28
        print 'big order', code, count, big_orders[-5:]
        return code, count, big_orders


    def test_loop(self):
        self._recv_run_flag = True
        self._recv_loop()

    def _recv_loop(self):
        while self._recv_run_flag:
            msg = self._socket.recv()
            self._parse(msg)


if __name__ == '__main__':
    ths_market = ThsMarket()
    ths_market.connect()
    ths_market.subscribe(['SZ00001', 'SZ000002', 'SH601515'])
    try:
        ths_market.test_loop()
    finally:
        ths_market.disconnect()
        del ths_market

