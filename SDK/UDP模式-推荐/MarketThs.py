#!/usr/bin/env python
# -*- coding: utf-8 -*-

import time, datetime
import struct
import threading
import urllib2
import random
import socket

class MarketThs():
    UDP_HOST = '127.0.0.1'
    UDP_PORT = 12425
    SUBSCRIBE_QUEUE_URI = 'http://localhost:10010/requestqx?list='
    SUBSCRIBE_L2_URI = 'http://localhost:10010/requestl2?list='


    def __init__(self):
        self._context = None
        self._socket = None
        self._recv_thread = None
        self._recv_run_flag = False
        self._message_buffer = []
        self._parse_thread = None
        self._parse_run_flag = False

        self.parse_buy_1_queue_only = True

        self.order_queue_callback = None
        self.quotes_callback = None
        self.last_print_time = None

    def __del__(self):
        self.disconnect()


    def connect(self):
        self.disconnect()

        self._recv_run_flag = True
        self._recv_thread = threading.Thread(target=self._recv_loop)
        self._recv_thread.start()

        self._parse_run_flag = True
        self._parse_thread = threading.Thread(target=self._parse_loop)
        self._parse_thread.start()

        self._socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        
        self._socket.bind((self.UDP_HOST, self.UDP_PORT))
        
         # 设置接收缓存为5M
        self._socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 5*1024*1024)
        print u'接收缓存大小：',self._socket.getsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF)
        self._socket.settimeout(1)
  
        print u'开始接收行情数据...'


    # 断开连接
    def disconnect(self):
        if self._socket is not None:
            self._socket.close()
            self._context = None

        if self._recv_thread is not None:
            self._recv_run_flag = False
            if self._recv_thread.is_alive():
                self._recv_thread.join()
            self._recv_thread = None

        if self._parse_thread is not None:
            self._parse_run_flag = False
            if self._parse_thread.is_alive():
                self._parse_thread.join()
            self._parse_thread = None


        self._message_buffer = []


    # 订阅传入的股票，股票代码格式为SZ000001, SH600858..etc.
    def subscribe_queue(self, security_list):
        send_headers = {
            'Cache-Control': 'no-cache'
        }

        param = ','.join(security_list)
        url = self.SUBSCRIBE_QUEUE_URI + param
        url += '&rnd=' + str(int(random.random() * 1000000000))
        req = urllib2.Request(url, headers=send_headers)
        try:
            response = urllib2.urlopen(req, timeout=10)
            response.read()
            return True
        except:
            return False

    def subscribe_l2(self, security_list):
        send_headers = {
            'Cache-Control': 'no-cache'
        }

        param = ','.join(security_list)
        url = self.SUBSCRIBE_L2_URI + param
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
        # 5档盘口
        if msg_type == 0:
            pass
        # 10档盘口(废弃）
        elif msg_type == 1:
            pass
        # 逐笔成交数据
        elif msg_type == 4:
            #self._parse_transaction(msg)
            pass
        # 逐笔委托数据(全息盘口)
        elif msg_type == 5:
            self._parse_order_queue(msg)
            pass
        # 高五档盘口数据
        elif msg_type == 6:
            pass
        # 百档盘口
        elif msg_type == 7:
            self._parse_quotes(msg)
            pass
        # 逐笔大单数据
        elif msg_type == 8:
            #self._parse_big_order(msg)
            pass
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
        #for test
        #if code[:8] != 'SZ000001':
        #    return
            
        buy_queue = []
        sell_queue = []
        read_ints = 0

        while read_ints < count_of_ints:
            # buy 16384, sell -32768
            price, buy_or_sell, vol_num = struct.unpack('1H1h1I', msg[list_start:list_start + 8])
            list_start += 8
            read_ints += 2 + vol_num
            # 忽略队列数量为0或50的无效数据
            if vol_num == 0 or vol_num == 50:
                return

            list_end = list_start + vol_num * 4
            vol_list = struct.unpack(str(vol_num) + 'I', msg[list_start: list_end])

            list_start = list_end

            if buy_or_sell == 16384:
                buy_queue.append((price, vol_num, vol_list,))
            else:
                sell_queue.append((price, vol_num, vol_list,))
  
            if self.parse_buy_1_queue_only:
                # for test
                print 'order queue', datetime.datetime.now(), code, vol_num, price, vol_list[:5]
                #if code[:8] == 'SZ000001':
                #    print 'order queue', datetime.datetime.now(), code, vol_num, price, vol_list[:5]
                break

        if self.order_queue_callback is not None:
            self.order_queue_callback((code[2:8], buy_queue, sell_queue,))
        #return code, buy_queue, sell_queue

    '''
    解析百档盘口
    返回:
        code - 股票代码, SZ000001, SH600858..etc.
        buy_quotes - 买档数据, 数组
        sell_quotes - 卖档数据， 数组
            买卖档数据item为tuple类型(level_num, price, volume, order_num)
                level_num - 档位，正数表示卖档，负数表示买档(奇怪的表示方式，注意不要搞反）
                price - 委托价格（单位：分）
                volume - 委托数量（单位：手）
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
                sell_quotes.append(quote)
            else:
                buy_quotes.append(quote)
                if self.parse_buy_1_queue_only:
                    if code[:8] == 'SZ000001':
                        print 'buy sell quotes', datetime.datetime.now(), code, buy_quotes[0]
                    break
            list_start += 16

        if self.quotes_callback:
            self.quotes_callback((code[2:8], buy_quotes, sell_quotes,))
        return code, buy_quotes, sell_quotes

    '''
    解析逐笔成交
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
        # if code[:8] == 'SZ000001':
        #     print 'detail transaction', code, count, pack_index, transactions[-3:]
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

    def _recv_loop(self):
        while self._recv_run_flag:
            try:
                msg, addr = self._socket.recvfrom(65535)
                self._message_buffer.append(msg)
            except:
                pass

    def _parse_loop(self):
        while self._parse_run_flag:
            l = len(self._message_buffer)
            if l > 200 and (self.last_print_time is None or (datetime.datetime.now()-self.last_print_time).seconds > 1):
                self.last_print_time = datetime.datetime.now()
                print 'OVERLOAD ' + str(l)
            if len(self._message_buffer) > 0:
                msg = self._message_buffer.pop(0)
                self._parse(msg)


if __name__ == '__main__':
    ths_market = MarketThs()
    try:
        ths_market.connect()
        #ths_market.subscribe_queue(['SZ000757'])
        while True:
            time.sleep(0.1)
    finally:
        ths_market.disconnect()
        del ths_market

