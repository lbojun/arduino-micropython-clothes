import urequests
import network
from time import sleep
import dht
from machine import Pin
import machine
import ssd1306
import DS1302 #导入ds1302库
import time
import _thread
from machine import Timer
import json
import socket

DEVICE_ID ='725502274'  #用于调用数据的API接口
API_KEY='LYksYknE6lR291bcNZQPoTFN0dA='

SSID="Honor10"  #用于连接wifi的热点信息
PASSWORD="liubojun"
wlan = None
s= None

sensor = dht.DHT11(Pin(13))  #定义温湿度传感器
ds = DS1302.DS1302(Pin(5),Pin(4),Pin(2))#clk data ret #设置端口
ds.start() #初始化
i2c = machine.SoftI2C(scl=machine.Pin(22), sda=machine.Pin(21))
oled = ssd1306.SSD1306_I2C(128, 64, i2c)

d = [00,00,00,00,00,00]  #每次向onenet传送6个温度历史数据
h = [00,00,00,00,00,00]  #每次向onenet传送6个湿度历史数据

def dht_func(d,h):  #打印温湿度数据
    sensor.measure()
    d.append(sensor.temperature())
    h.append(sensor.humidity())
    d[:] = d[-6:]
    h[:] = h[-6:]
    print('Temperature:' ,d)
    print('Humidity:' ,h)
    
def ssd_func():  #在oled上显示当前温湿度和当前时间
    year_now, month_now, day_now = ds.DateTime()[0], ds.DateTime()[1], ds.DateTime()[2]
    hour_now, minute_now, second_now = ds.DateTime()[4], ds.DateTime()[5], ds.DateTime()[6]
    now_time = str(month_now) +'.'+str(day_now)+'.'+str(hour_now)+'.'+str(minute_now)+'.'+str(second_now)
    oled.fill(0)
    hum = 'hum:' + str(h[-1])
    tem = 'tem:'  + str(d[-1])
    oled.text(now_time, 0, 0)
    oled.text(hum, 0, 20)
    oled.text(tem,60,20)
    oled.show()
        
tim0 = Timer(0)  #定义timer，使其并行
tim0.init(period=2000,mode=Timer.PERIODIC,callback=lambda t:dht_func(d,h))

tim1 = Timer(1)  #定义timer，使其并行
tim1.init(period=50,mode=Timer.PERIODIC,callback=lambda t:ssd_func())

def connectWifi(ssid,password):  #连接WIFI
    global wlan
    wlan = network.WLAN(network.STA_IF)
    wlan.active(True)
    wlan.disconnect()
    wlan.connect(ssid,password)
    while(wlan.ifconfig()[0]=='0.0.0.0'):
        time.sleep(1)
    return True

def http_put_data(data):   #向onenet发送数据
    url='http://api.heclouds.com/devices/'+DEVICE_ID+'/datapoints'
    values={'datastreams':[{"id":"dht","datapoints":[{"value":data}]}]}
    jdata = json.dumps(values)
    r = urequests.post(url,data=jdata,headers={"api-key":API_KEY})
    return r

#ds.DateTime([2021, 6, 13, 2, 15, 17, 10]) #用于修正时间
connectWifi(SSID,PASSWORD)
while True:
    sleep(1)
    rsp = http_put_data(str(d)+str(h)) #每秒向onenet传送一次数据
    print(rsp.json())
    



