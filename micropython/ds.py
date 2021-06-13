from machine import Pin
import DS1302 #导入ds1302库
from time import sleep
ds = DS1302.DS1302(Pin(5),Pin(4),Pin(2))#clk data ret #设置端口
ds.start() #初始化
while True:
    print(ds.DateTime()) #打印当前时间
    print(ds.Hour())  #打印当前小时数
    sleep(2) #隔两秒
