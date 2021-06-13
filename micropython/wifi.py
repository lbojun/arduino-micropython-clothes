import network
wlan = network.WLAN(network.STA_IF)
wlan.active(True) #wifi激活
wlan.connect("Honor10","liubojun")#wifi连接
while not wlan.isconnected():
    pass
print("wifi已连接")
print(wlan.ifconfig())
