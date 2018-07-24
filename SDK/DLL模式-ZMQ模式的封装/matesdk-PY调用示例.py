import ctypes
from ctypes import *
import json
import os
import time


jbuff = create_string_buffer(1024*1024)


import datetime
import csv
import os

dll_quote = ctypes.CDLL(r'MateSdk_x64.dll') 

#print r'connect to zqm quote server result:' + connect_result
channel = create_string_buffer(b"WDPK")
dll_quote.Subscribe(channel)

while (True):
    
    jbuff = create_string_buffer(1024*1024*10)
    value = dll_quote.GetData(byref(jbuff), 1024*1024*10)
    addr = addressof(jbuff)

#    temps = string_at(addressof(jbuff))
    to = type(jbuff)
   # val = jbuff.value
    jss = jbuff.value
    #string_at(addressof(jbuff))
    #time.sleep(1)
    
    jsobject = json.loads(jss, encoding='utf-8')
    if jsobject[0]['QuoteType']=="WDPK":
        aaa = jsobject[0]['Stocks'][0]['StockCode']
        print(aaa)
