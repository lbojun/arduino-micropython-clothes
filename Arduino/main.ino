#include <WiFi.h>
#include "Thread.h"
#include <ThreeWire.h>  
#include <RtcDS1302.h>
#include "BluetoothSerial.h"
#include <Wire.h>
#include "SSD1306.h"
#include "DHT.h"  //温湿度传感器
#define DHTPIN 13    //设置数据端口为13，可以使用3，4，5，12，13，14
#define DHTTYPE DHT11   // DHT 11
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif


const char *ssid = "Honor10"; //你的网络名称
const char *password = "liubojun"; //你的网络密码
BluetoothSerial SerialBT;
ThreeWire myWire(4,5,2); // IO, SCLK, CE
RtcDS1302<ThreeWire> Rtc(myWire);
DHT dht(DHTPIN, DHTTYPE);   //温湿度传感器类
SSD1306  display(0x3c, 21, 22);
String bt_text="";
WiFiClient client;

Thread time_thread = Thread();
Thread bt_thread = Thread();
Thread dht_thread = Thread();
Thread oled_thread = Thread();

void time_print(){
    RtcDateTime now = Rtc.GetDateTime();
    //printDateTime(now);
    if (!now.IsValid())
       Serial.println("RTC lost confidence in the DateTime!");
}
String bt_print(){
  String a="";
  while(SerialBT.available())
      a += char(SerialBT.read());
  if(a!="")
      bt_text =a;
  return a;
}

void dht_print(){
  float h = dht.readHumidity();  //读湿度
  float t = dht.readTemperature();  //读温度，摄氏度
}

void oled_print(){
  float h = dht.readHumidity();  //读湿度
  float t = dht.readTemperature();  //读温度，摄氏度
  RtcDateTime now = Rtc.GetDateTime();//读时间
  display.clear();
  
  String dht_text = String("Hum:") + String(h) + String("%  Tem:") + String(t) + "°C  ";
  String time_now = String("Time:") + printDateTime(now);
  String bt = bt_print();
  if(bt=="")
     bt = bt_text;
  display.drawString(0,0,dht_text);
  display.drawString(0,20,time_now);
  display.drawString(0,40,bt);
  display.display();
}

const IPAddress serverIP(183,230,40,40); //欲访问的地址
uint16_t serverPort = 1811;
int t_str[6]={0,0,0,0,0,0};
int h_str[6]={0,0,0,0,0,0};

void setup () 
{

  
    Serial.begin(115200);
    WiFi.begin(ssid, password); //连接网络
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    
    int tmp;
    Serial.print("compiled: ");
    Serial.print(__DATE__);
    Serial.println(__TIME__);
    //屏幕初始化
    display.init();
    //连接蓝牙
    SerialBT.begin("ESP32test"); //Bluetooth device name
    Serial.println("The device started, now you can pair it with bluetooth!");
    //时钟初始化
    Rtc.Begin();
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    printDateTime(compiled);
    Serial.println();
    time_set(compiled);
    //dht温湿度
    Serial.println(F("DHT11 test!"));
    dht.begin();

    time_thread.onRun(time_print);
    time_thread.setInterval(10000);
    dht_thread.onRun(dht_print);
    dht_thread.setInterval(10000);
    oled_thread.onRun(oled_print);
    oled_thread.setInterval(20);
    if (client.connect(serverIP, serverPort)) //尝试访问目标地址
    {
        Serial.println("访问成功"); 
    }
    
   client.println("*428615#lbj#sample*");//发送报文。产品ID+设备鉴权碼+脚本名称。这里千万要注意是产品ID，如果成了设备ID是上传不了的。还有就是，一个设备一个脚本。
}

void loop () 
{
    float h = dht.readHumidity();  //读湿度
    float t = dht.readTemperature();  //读温度，摄氏度
    if(oled_thread.shouldRun())
         oled_thread.run();
    if(dht_thread.shouldRun())
         dht_thread.run();
    if(time_thread.shouldRun()){
         time_thread.run();
         Serial.println();
    }
    h = int(h);
    t = int(t);
    for(int i=0;i<6;i++)
    {
      if(i<5)
      {
        t_str[i]=t_str[i+1];
        h_str[i]=h_str[i+1];
        }
        else
        {
          t_str[i]=t;
          h_str[i]=h;
          }
      }
    
    client.print('['+String(t_str[0]) +','+String(t_str[1]) +','+String(t_str[2])+','+String(t_str[3]) +','+String(t_str[4]) +','+String(t_str[5]) +']'+ '['+String(h_str[0])+ ','+String(h_str[1])+ ','+String(h_str[2])+ ','+String(h_str[3])+','+ String(h_str[4])+','+ String(h_str[5])+']');    //向服务器发送数据
    delay(2000);

}

#define countof(a) (sizeof(a) / sizeof(a[0]))
String printDateTime(const RtcDateTime& dt)  //得到时间年月日
{
    char datestring[20];
    snprintf_P(datestring, 
            countof(datestring),
            PSTR("%02u/%02u/%04u %02u:%02u:%02u"),
            dt.Month(),
            dt.Day(),
            dt.Year(),
            dt.Hour(),
            dt.Minute(),
            dt.Second() );
    return datestring;
}

void time_set(RtcDateTime compiled)  //设定时间的情况处理
{
      if (!Rtc.IsDateTimeValid()) 
    {
        // Common Causes:
        //    1) first time you ran and the device wasn't running yet
        //    2) the battery on the device is low or even missing

        Serial.println("RTC lost confidence in the DateTime!");
        Rtc.SetDateTime(compiled);
    }

    if (Rtc.GetIsWriteProtected())
    {
        Serial.println("RTC was write protected, enabling writing now");
        Rtc.SetIsWriteProtected(false);
    }

    if (!Rtc.GetIsRunning())
    {
        Serial.println("RTC was not actively running, starting now");
        Rtc.SetIsRunning(true);
    }
    RtcDateTime now = Rtc.GetDateTime();
    if (now < compiled) 
    {
        Serial.println("RTC is older than compile time!  (Updating DateTime)");
        Rtc.SetDateTime(compiled);
    }
    else if (now > compiled) 
        Serial.println("RTC is newer than compile time. (this is expected)");
    else if (now == compiled) 
        Serial.println("RTC is the same as compile time! (not expected but all is fine)");
}
