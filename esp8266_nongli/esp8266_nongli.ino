 //leftmonitor.ino
//这是在面包板上全都正常最终版本2021.5.2日完成,用 TX和RX做I2C通讯。用0脚做温度检测。
#include <Arduino.h>//<>为系统头文件，一般在库内。
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>//网络服务器用
#include <ESP8266HTTPClient.h>
#include <stdio.h>
#include <stdlib.h>

#include <FS.h>  
 
#include <DNSServer.h>//密码直连将其三个库注释
#include <ESP8266WebServer.h>
//#include <CustomWiFiManager.h>  //  这个库找不到，换成了WiFiManager.h也可以
#include <WiFiManager.h>           //  WiFiManager.h 需要安装

#include <OneWire.h>             //  OneWire.h需要安装
#include <DallasTemperature.h>  //  DallasTemperature.h需要安装
#include <Wire.h>

#include <sys/time.h>
#include <coredecls.h>
#include <U8g2lib.h>   // U8g2lib.h需要安装
#include <U8x8lib.h>
  
#ifdef U8X8_HAVE_HW_SPI  
#include <SPI.h>  
#endif  
#ifdef U8X8_HAVE_HW_I2C  
#include <Wire.h>  
#endif  

#include "lunarday.h"

#define ONE_WIRE_BUS 0  // DS18B20 pin 0 是mini的D4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);

ESP8266WebServer esp8266_server(80);    // 网络服务器用建立网络服务器对象，该对象用于响应HTTP请求。监听端口（80）
 

int temperature;
float temp;
int year1; 
int month1;
int day1;
int hour1;

int clock2=22; //息屏时间
int clock1=7;   // 亮屏时间
int minlight=20; //最小亮度
int maxlight=1000; // 最大亮度

const int led = D4; // 板载指示灯，晚上10点后也要熄灭
const int blak = D2; //控制晚上10点后息屏的和第二天早上7点亮屏的gpio口
const String WDAY_NAMES[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};  //星期

U8G2_ST7920_128X64_F_SW_SPI u8g2(U8G2_R0, /* clock=*/ D5, /* data=*/ D7, /* CS=*/ D8, /* reset=*/ D0);   //适用于2021.12.23的板子，8266的地线要和转接板接通，原设计漏掉了

 
//const char* WIFI_SSID = "";  //填写你的WIFI名称及密码
//const char* WIFI_PWD = "";



//以下为中间大字字库字宽x字高 16 x 32
const unsigned char zero[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xE0,0x0F,0xF0,
 0x1C,0x38,0x38,0x38,0x38,0x1C,0x30,0x1C,0x70,0x0C,0x70,0x0C,0x70,0x0C,0x70,0x0C,
 0x70,0x0C,0x70,0x0C,0x70,0x0C,0x70,0x0C,0x70,0x0C,0x30,0x1C,0x38,0x1C,0x38,0x18,
 0x1C,0x38,0x1F,0xF0,0x07,0xE0,0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"0",0*/
};

const unsigned char one[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0xC0,0x01,0xC0,
 0x03,0xC0,0x07,0xC0,0x1D,0xC0,0x19,0xC0,0x11,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,
 0x01,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,0x01,0xC0,
 0x01,0xC0,0x01,0xC0,0x01,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"1",1*/
};
const unsigned char two[] U8X8_PROGMEM ={

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xE0,0x1F,0xF8,
 0x3C,0x38,0x38,0x1C,0x70,0x1C,0x00,0x1C,0x00,0x1C,0x00,0x18,0x00,0x38,0x00,0x38,
 0x00,0x70,0x00,0xE0,0x00,0xC0,0x01,0xC0,0x03,0x80,0x07,0x00,0x0E,0x00,0x1C,0x00,
 0x3C,0x00,0x3F,0xFC,0x3F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"2",2*/

};

const unsigned char three[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x07,0xE0,0x0F,0xF8,
 0x1C,0x38,0x38,0x1C,0x30,0x1C,0x10,0x1C,0x00,0x1C,0x00,0x18,0x00,0x78,0x03,0xE0,
 0x03,0xF0,0x00,0x78,0x00,0x1C,0x00,0x1C,0x00,0x1C,0x10,0x0C,0x70,0x1C,0x38,0x1C,
 0x3C,0x38,0x1F,0xF8,0x0F,0xE0,0x01,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"3",3*/
};

const unsigned char four[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x30,0x00,0x70,
 0x00,0x70,0x00,0xF0,0x01,0xF0,0x01,0xB0,0x03,0xB0,0x07,0x30,0x0E,0x30,0x0C,0x30,
 0x1C,0x30,0x38,0x30,0x30,0x30,0x70,0x30,0xFF,0xFE,0xFF,0xFE,0xFF,0xFE,0x00,0x30,
 0x00,0x30,0x00,0x30,0x00,0x30,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"4",4*/

};

const unsigned char five[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x1F,0xF8,0x1F,0xF8,
0x18,0x00,0x18,0x00,0x18,0x00,0x38,0x00,0x30,0x00,0x37,0xC0,0x3F,0xF0,0x78,0x78,
0x70,0x38,0x00,0x1C,0x00,0x1C,0x00,0x0C,0x00,0x0C,0x20,0x1C,0x60,0x1C,0x70,0x1C,
0x70,0x78,0x3F,0xF0,0x1F,0xE0,0x07,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"5",5*/
};

const unsigned char six[] U8X8_PROGMEM ={

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xE0,0x00,0xE0,
0x01,0xC0,0x03,0x80,0x03,0x80,0x07,0x00,0x0E,0x00,0x0E,0x00,0x1F,0xF0,0x1F,0xF8,
0x3C,0x1C,0x38,0x0E,0x70,0x0E,0x70,0x0E,0x70,0x0E,0x70,0x0E,0x70,0x0E,0x38,0x0E,
0x3C,0x1C,0x1F,0xF8,0x0F,0xF0,0x01,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"6",6*/

};
const unsigned char seven[] U8X8_PROGMEM ={

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x7F,0xFE,0x7F,0xFE,
0x00,0x0E,0x00,0x1C,0x00,0x18,0x00,0x38,0x00,0x30,0x00,0x70,0x00,0x60,0x00,0xE0,
0x00,0xC0,0x01,0xC0,0x01,0xC0,0x01,0x80,0x03,0x80,0x03,0x80,0x03,0x00,0x07,0x00,
0x07,0x00,0x06,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"7",7*/

};
const unsigned char eight[] U8X8_PROGMEM ={

0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xE0,0x1F,0xF0,
0x38,0x38,0x30,0x1C,0x30,0x1C,0x30,0x1C,0x30,0x1C,0x38,0x38,0x3C,0x78,0x0F,0xE0,
0x1F,0xF0,0x3C,0x78,0x78,0x1C,0x70,0x1C,0x60,0x0C,0x60,0x0C,0x70,0x0C,0x70,0x1C,
0x78,0x3C,0x3F,0xF8,0x1F,0xF0,0x03,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"8",8*/

};

const unsigned char nine[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0xE0,0x1F,0xF0,
0x3C,0x78,0x70,0x3C,0x70,0x1C,0x60,0x1C,0x60,0x1C,0x60,0x1C,0x60,0x1C,0x70,0x38,
0x78,0x78,0x3F,0xF8,0x1F,0xF0,0x00,0xE0,0x00,0xE0,0x01,0xC0,0x01,0xC0,0x03,0x80,
0x03,0x80,0x07,0x00,0x06,0x00,0x0E,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"9",9*/
};


const unsigned char maohao[] U8X8_PROGMEM = {
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x01,0x80,0x03,0xC0,0x03,0xC0,0x01,0x80,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x80,0x03,0xC0,0x03,0xC0,0x01,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*":",0*/
};

const unsigned char* mart[]={zero,one,two,three,four,five,six,seven,eight,nine};//显示中间大字时间做准备,

//以下为摄氏温度符号字库字宽x字高 16 x 16
const unsigned char du[] U8X8_PROGMEM ={
0x60,0x00,0x91,0xF4,0x96,0x0C,0x6C,0x04,0x08,0x04,0x18,0x00,0x18,0x00,0x18,0x00,
0x18,0x00,0x18,0x00,0x18,0x00,0x08,0x00,0x0C,0x04,0x06,0x08,0x01,0xF0,0x00,0x00,/*"℃",0*/

};

//以下为农历字库字宽x字高 16 x 16
const unsigned char zheng[] U8X8_PROGMEM =
{
0x00,0x00,0x7F,0xFC,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x11,0x00,0x11,0xF8,
0x11,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0x11,0x00,0xFF,0xFE,0x00,0x00,/*"正",0*/
};
const unsigned char yi[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,/*"一",15*/
};
const unsigned char er[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x00,0x00,0x00,0x00,0x00,0x00,/*"二",1*/
};

const unsigned char san[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x7F,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3F,0xF8,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,0xFE,0x00,0x00,0x00,0x00,/*"三",2*/
};

const unsigned char si[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x00,0x7F,0xFC,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,0x44,
0x48,0x44,0x48,0x3C,0x50,0x04,0x60,0x04,0x40,0x04,0x7F,0xFC,0x40,0x04,0x00,0x00,/*"四",3*/
};

const unsigned char wu[] U8X8_PROGMEM ={
0x00,0x00,0x7F,0xFC,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x3F,0xF0,0x04,0x10,
0x04,0x10,0x04,0x10,0x04,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0xFF,0xFE,0x00,0x00,/*"五",4*/
};

const unsigned char liu[] U8X8_PROGMEM ={
0x02,0x00,0x01,0x00,0x00,0x80,0x00,0x80,0x00,0x00,0xFF,0xFE,0x00,0x00,0x00,0x00,
0x04,0x40,0x04,0x20,0x08,0x10,0x08,0x08,0x10,0x08,0x20,0x04,0x40,0x04,0x00,0x00,/*"六",5*/
};

const unsigned char qi[] U8X8_PROGMEM ={
0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x3C,0x07,0xC0,0xFA,0x00,
0x02,0x00,0x02,0x00,0x02,0x00,0x02,0x04,0x02,0x04,0x02,0x04,0x01,0xFC,0x00,0x00,/*"七",6*/
};

const unsigned char ba[] U8X8_PROGMEM ={
0x00,0x00,0x00,0x40,0x04,0x40,0x04,0x40,0x04,0x40,0x04,0x40,0x04,0x40,0x04,0x20,
0x08,0x20,0x08,0x20,0x08,0x10,0x10,0x10,0x10,0x08,0x20,0x08,0x20,0x04,0x40,0x02,/*"八",7*/
};

const unsigned char jiu[] U8X8_PROGMEM ={
0x04,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x7F,0xE0,0x04,0x20,0x04,0x20,0x04,0x20,
0x08,0x20,0x08,0x20,0x08,0x20,0x10,0x22,0x10,0x22,0x20,0x22,0x40,0x1E,0x80,0x00,/*"九",8*/
};

const unsigned char shi[] U8X8_PROGMEM ={
0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0xFF,0xFE,0x01,0x00,
0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,0x01,0x00,/*"十",9*/
};
const unsigned char dong[] U8X8_PROGMEM ={
0x04,0x00,0x04,0x00,0x0F,0xF0,0x10,0x10,0x28,0x20,0x44,0x40,0x03,0x80,0x0C,0x60,
0x30,0x18,0xC0,0x06,0x07,0x00,0x00,0xC0,0x00,0x20,0x0E,0x00,0x01,0x80,0x00,0x40,/*"冬",10*/
};

const unsigned char la[] U8X8_PROGMEM ={
0x00,0x88,0x78,0x88,0x48,0x88,0x4B,0xFE,0x48,0x88,0x78,0x88,0x4F,0xFE,0x48,0x00,
0x49,0xFC,0x79,0x04,0x49,0x04,0x49,0xFC,0x49,0x04,0x49,0x04,0x49,0xFC,0x99,0x04,/*"腊",11*/
};

const unsigned char chu[] U8X8_PROGMEM ={
0x20,0x00,0x10,0x00,0x01,0xFC,0xFC,0x44,0x08,0x44,0x10,0x44,0x10,0x44,0x34,0x44,
0x58,0x44,0x94,0x44,0x14,0x44,0x10,0x84,0x10,0x84,0x11,0x04,0x12,0x28,0x14,0x10,/*"初",12*/
};

const unsigned char nian[] U8X8_PROGMEM ={
0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0xFF,0xFE,0x08,0x10,0x08,0x10,
0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x08,0x10,0x0F,0xF0,0x08,0x10,/*"廿",13*/
};

const unsigned char yue[] U8X8_PROGMEM ={
0x00,0x00,0x1F,0xF8,0x10,0x08,0x10,0x08,0x10,0x08,0x1F,0xF8,0x10,0x08,0x10,0x08,
0x10,0x08,0x1F,0xF8,0x10,0x08,0x10,0x08,0x20,0x08,0x20,0x08,0x40,0x28,0x80,0x10,/*"月",14*/
};

const unsigned char ri[] U8X8_PROGMEM ={
0x00,0x00,0x1F,0xF0,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0xF0,
0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x1F,0xF0,0x10,0x10,/*"日",16*/
};
#define TZ              8      // 中国时区为8
#define DST_MN          0      // 默认为0
 
#define TZ_MN           ((TZ)*60)   //时间换算
#define TZ_SEC          ((TZ)*3600)
#define DST_SEC         ((DST_MN)*60)
 
time_t now; //实例化时间

long timeSinceLastWUpdate = 0;    //上次更新后的时间
const int UPDATE_INTERVAL_SECS = 5 * 60; // 5分钟更新一次

//#include "WeatherStationFonts.h"
 
//
//void wificonnect() {  // WIFI密码连接
//  WiFi.begin(WIFI_SSID, WIFI_PWD);
//  while (WiFi.status() != WL_CONNECTED) {
//    Serial.print('.');
//    delay(80);
//    display.clear();
//    display.setFont(ArialMT_Plain_24);
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(10, 18, "wait");
//    display.display();
//    delay(80);
//    display.clear();
//    display.setFont(ArialMT_Plain_24);
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(10, 18, "wait.");
//    display.display();
//    delay(80);
//    display.clear();
//    display.setFont(ArialMT_Plain_24);
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(10, 18, "wait..");
//    display.display();
//    delay(80);
//    display.clear();
//    display.setFont(ArialMT_Plain_24);
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(10, 18, "wait...");
//    display.display();
//    delay(80);
//    display.clear();
//    display.setFont(ArialMT_Plain_24);
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(10, 18, "wait....");
//    display.display();
//    delay(80);
//    display.clear();
//    display.setFont(ArialMT_Plain_24);
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(10, 18, "wait.....");
//    display.display();
//    delay(80);
//    display.clear();
//    display.setFont(ArialMT_Plain_24);
//    display.setTextAlignment(TEXT_ALIGN_LEFT);
//    display.drawString(10, 18, "wait......");
//    display.display();
//    delay(80);
//    
//  }
//  Serial.println("");
//  delay(500);
//}






 
//void changedate() {
//  using namespace std;
//  using namespace lunar;
//  
//  int16_t x;
//  int16_t y;
//  now = time(nullptr);
//  struct tm* timeInfo;
//  timeInfo = localtime(&now);
//  char buff[16];
// 
//  time_t current_time;
//  time(&current_time);
//  tm *current_tm = localtime(&current_time);
// 
//  int year = current_tm->tm_year + 1900;
//  int month = current_tm->tm_mon + 1;
//  int day = current_tm->tm_mday;
//  Date lunar_date = LuanrDate(year, month, day);
//  
//  display.setFont(ArialMT_Plain_24);
//  display.drawString(60 + x, 40 + y, String(buff));
//  display.setTextAlignment(TEXT_ALIGN_LEFT);
//  
//  display.drawXbm(20, 40, 16,16, yi);
//  display.drawXbm(20, 56, 16,16, er);
// 
//}

void webconnect() {  //Web配网，密码直连将其注释
  
   u8g2.setFont(u8g2_font_wqy15_t_chinese2);  // use chinese2 for all the glyphs of "我是单片机菜鸟"  
   u8g2.setFontDirection(0);  
   u8g2.clearBuffer(); 
   u8g2.setCursor(16, 31); 
//                X列   字的最后一行的Y坐标
//                0-127  15--63
   u8g2.print("wait...");

   u8g2.sendBuffer();
  
  WiFiManager wifiManager;  //实例化WiFiManager
  wifiManager.setDebugOutput(false); //关闭Debug
  
  //wifiManager.setConnectTimeout(10); //设置超时
  //wifiManager.setHeadImgBase64(FPSTR(Icon)); //设置图标
 //wifiManager.setPageTitle("欢迎来到WiFi配置页");  //设置页标题
 
  if (!wifiManager.autoConnect("我的万年历 ")) {  //AP模式
    //Serial.println("连接失败并超时");
    //重新设置并再试一次，或者让它进入深度睡眠状态
    ESP.restart();
    // delay(10);
  }
  //Serial.println("connected...^_^");
  yield(); //
}

void webconnect1(){
  
   Serial.begin(9600);  //自动配网需要9600
   WiFiManager wifiManager;  //
   wifiManager.autoConnect("AutoConnectAP"); //连接AutoConnectAP这个wifi时无密码 
     
  }




 

void drawDateTime() {  //显示时间函数
  
  using namespace std;
  using namespace lunar;
  
  int16_t x;
  int16_t y;
  now = time(nullptr);
  struct tm* timeInfo;
  timeInfo = localtime(&now);
  char buff[16];
    
   String date = WDAY_NAMES[timeInfo->tm_wday];
   sprintf_P(buff, PSTR("%4d-%02d-%02d       "), timeInfo->tm_year+1900,timeInfo->tm_mon + 1, timeInfo->tm_mday);
  //display.drawString(40 + x, 0 + y, String(buff));
    
  //以下显示公历日期2021年03月05日  
    char yy[5];
    yy[0] = buff[0];
    yy[1] = buff[1];
    yy[2] = buff[2];
    yy[3] = buff[3];
    yy[4] = '\0';
    year1 = (atol)(yy);

    char mm[3];
    mm[0] = buff[5];
    mm[1] = buff[6];
    mm[2] = '\0';
    month1 = (atol)(mm);
     char dd[3];
    dd[0] = buff[8];
    dd[1] = buff[9];
    dd[2] = '\0';
    day1 = (atol)(dd);

   u8g2.setFont(u8g2_font_wqy15_t_chinese2);  // use chinese2 for all the glyphs of "我是单片机菜鸟"  
   u8g2.setFontDirection(0);  
   u8g2.clearBuffer();  
//u8g2.setCursor(14, 15); 
//             X列   字的最后一行的Y坐标
//            0-127  15--63
//u8g2.print("2022年01月01日");  
//以下显示第一行公历年月日

u8g2.setFont(u8g2_font_wqy16_t_gb2312a);//有了这一句才可以输出变量
u8g2.setCursor(10,15);
u8g2.print(year1);
u8g2.setCursor(42,15);
u8g2.print("年");
if (month1<10){
  u8g2.setCursor(58,15);
  u8g2.print("0");
  u8g2.setCursor(66,15);
  u8g2.print(month1);
  u8g2.setCursor(74,15);
  u8g2.print("月");
  }
else{
  u8g2.setCursor(58,15);
  u8g2.print(month1);
  u8g2.setCursor(74,15);
  u8g2.print("月");
  
  }

  if (day1<10){
  u8g2.setCursor(90,15);
  u8g2.print("0");
  u8g2.setCursor(98,15);
  u8g2.print(day1);
  u8g2.setCursor(106,15);
  u8g2.print("日");
  }
else{
  u8g2.setCursor(90,15);
  u8g2.print(day1);
  u8g2.setCursor(106,15);
  u8g2.print("日");
  }

//以下显示第二行大字时分秒 

    sprintf_P(buff, PSTR("%02d:%02d:%02d"), timeInfo->tm_hour, timeInfo->tm_min, timeInfo->tm_sec);//显示时间

   
  //显示16x32字库字体 
   for(int i = 0;i<8;i++){
    char n = buff[i];
    if(n==':'){
       u8g2.drawBitmap((i)*16, 16, 2,32, maohao);
             }
      else{
        n -=48;
        u8g2.drawBitmap(i*16, 16, 2,32, mart[n]);
         } 
    }
    

 //以下为夜间息屏程序
/*
int clock2=22; //息屏时间
int clock1=7;   // 亮屏时间
int minlight=20; //最小亮度
int maxlight=1000; // 最大亮度       
*/        
    char hh[3];
    hh[0] = buff[0];
    hh[1] = buff[1];
    hh[3] = '\0';

        hour1 = (atol)(hh);
       if(hour1>=clock2 || hour1<clock1){
          //digitalWrite(blak,LOW);
          analogWrite(blak,minlight);
          digitalWrite(led,HIGH);  // 高电平时反而是暗的
   };
      if(hour1>=clock1 and hour1<clock2){
          //digitalWrite(blak,HIGH);
          analogWrite(blak,maxlight);
          digitalWrite(led,LOW);  // 低电平时反而是亮的
          
   };

    
  
//显示农历日期

 
  int year = timeInfo->tm_year + 1900;
  int month = timeInfo->tm_mon + 1;
  int day = timeInfo->tm_mday;

  int c=2;
  int d=16;
  
  Date lunar_date = LuanrDate(year, month, day); //公历转你农历函数，参数为当前的公历日期 
//显示农历月份
 if (lunar_date.month == 1 && lunar_date.day==0)
  {    
    u8g2.drawBitmap(0, 48, c, d, la);    
  }
  else if (lunar_date.month==1 && lunar_date.day!=0 )
 {
    u8g2.drawBitmap(0, 48, c,d, zheng);
  }
  else if  (lunar_date.month==2)
  {
    u8g2.drawBitmap(0, 48, c,d, er);
  }
  else if  (lunar_date.month==3)
  {
    u8g2.drawBitmap(0, 48, c,d, san); 
  }
  else if  (lunar_date.month==4)
  {
    u8g2.drawBitmap(0, 48, c,d, si); 
  }
  else if  (lunar_date.month==5)
  {
    u8g2.drawBitmap(0, 48, c,d, wu); 
  }
  else if  (lunar_date.month==6)
  {
    u8g2.drawBitmap(0, 48, c,d, liu); 
  }
  else if  (lunar_date.month==7)
  {
    u8g2.drawBitmap(0, 48, c,d, qi);  
  }
  else if  (lunar_date.month==8)
  {
    u8g2.drawBitmap(0, 48, c,d, ba);  
  }
  else if  (lunar_date.month==9)
  {
    u8g2.drawBitmap(0, 48, c,d, jiu);  
  }
  else if  (lunar_date.month==10)
  {
    u8g2.drawBitmap(0, 48, c,d, shi);  
  }
  else if  (lunar_date.month==11)
  {
    u8g2.drawBitmap(0, 48, c,d, dong);  
  }
  else if (lunar_date.month==12)
  {
    u8g2.drawBitmap(0, 48, c,d, la);  
 
  }
   u8g2.drawBitmap(16, 48, c,d, yue);
 

  
  
    
  if (lunar_date.day==11)      
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, yi); 
  }
  else if (lunar_date.day==12)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, er); 
  }
  else if (lunar_date.day==13)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, san); 
  }
  else if (lunar_date.day==14)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, si); 
  }
  else if (lunar_date.day==15)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, wu); 
  }
  else if (lunar_date.day==16)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, liu); 
  }
  else if (lunar_date.day==17)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, qi); 
  }
  else if (lunar_date.day==18)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, ba); 
  }
  else if (lunar_date.day==19)
  {
    u8g2.drawBitmap(32, 48, c,d, shi); 
    u8g2.drawBitmap(48, 48, c,d, jiu); 
  }
  else if (lunar_date.day==20)
  {
    u8g2.drawBitmap(32, 48, c,d, er); 
    u8g2.drawBitmap(48, 48, c,d, shi); 
  }
  else if (lunar_date.day==21)      
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, yi); 
  }
  else if (lunar_date.day==22)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, er); 
  }
  else if (lunar_date.day==23)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, san); 
  }
  else if (lunar_date.day==24)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, si); 
  }
  else if (lunar_date.day==25)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, wu); 
  }
  else if (lunar_date.day==26)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, liu); 
  }
  else if (lunar_date.day==27)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, qi); 
  }
  else if (lunar_date.day==28)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, ba); 
  }
  else if (lunar_date.day==29)
  {
    u8g2.drawBitmap(32, 48, c,d, nian); 
    u8g2.drawBitmap(48, 48, c,d, jiu); 
  }
  else if (lunar_date.day==30)
  {
    u8g2.drawBitmap(32, 48, c,d, san); 
    u8g2.drawBitmap(48, 48, c,d, shi); 
  }
  else if (lunar_date.day==1)      
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, yi); 
  }
  else if (lunar_date.day==2)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, er); 
  }
  else if (lunar_date.day==3)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, san); 
  }
  else if (lunar_date.day==4)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, si); 
  }
  else if (lunar_date.day==5)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, wu); 
  }
  else if (lunar_date.day==6)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, liu); 
  }
  else if (lunar_date.day==7)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, qi); 
  }
  else if (lunar_date.day==8)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, ba); 
  }
  else if (lunar_date.day==9)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, jiu); 
  }
  else if (lunar_date.day==10)
  {
    u8g2.drawBitmap(32, 48, c,d, chu); 
    u8g2.drawBitmap(48, 48, c,d, shi); 
  }
  else if (lunar_date.day==0 )
     {
     day=day-1;
    //Serial.print("阳历日：");
    //Serial.println(day);
    Date lunar_date = LuanrDate(year, month, day);
    //Serial.print("日期为：");
    //Serial.println(lunar_date.day);
        if (lunar_date.day==29){
            u8g2.drawBitmap(32, 48, c,d, san); 
            u8g2.drawBitmap(48, 48, c,d, shi);
            }
         else if(lunar_date.day==28){
           u8g2.drawBitmap(32, 48, c,d, nian); 
           u8g2.drawBitmap(48, 48, c,d, jiu);
           }
      }

// 以下显示星期
   int a = 72; //显示星期的x坐标
   int b = 48; //显示星期的y坐标
 
  if (WDAY_NAMES[timeInfo->tm_wday]=="Mon")
    u8g2.drawBitmap(a, b, 2,16, yi);
  else if (WDAY_NAMES[timeInfo->tm_wday]=="Tue")
    u8g2.drawBitmap(a, b, 2,16, er);
  else if (WDAY_NAMES[timeInfo->tm_wday]=="Wed")
    u8g2.drawBitmap(a, b, 2,16, san);
  else if (WDAY_NAMES[timeInfo->tm_wday]=="Thu")
    u8g2.drawBitmap(a, b, 2,16, si);
  else if (WDAY_NAMES[timeInfo->tm_wday]=="Fri")
    u8g2.drawBitmap(a, b, 2,16, wu);
  else if (WDAY_NAMES[timeInfo->tm_wday]=="Sat")
    u8g2.drawBitmap(a, b, 2,16, liu);
  else if (WDAY_NAMES[timeInfo->tm_wday]=="Sun")
    u8g2.drawBitmap(a, b, 2,16, ri);


 //以下显示温度的值
   u8g2.setFont(u8g2_font_wqy16_t_gb2312a);//有了这一句才可以输出变量
   u8g2.setCursor(96,63);
   u8g2.print(temperature);
//以下显示温度的符号
   u8g2.drawBitmap(112,48,2,16,du);

   u8g2.sendBuffer();
   
   delay(100); 

}


void setup() {

    Serial.begin(9600);
    //Serial.begin(115200);
//  Serial.println();//println换行输出
//  Serial.println();
    u8g2.begin();  
    u8g2.enableUTF8Print();   // enable UTF8 support for the Arduino print() function  

    pinMode(blak,OUTPUT);
    pinMode(led,OUTPUT);

   // digitalWrite(blak,HIGH);
     analogWrite(blak,maxlight);
     digitalWrite(led,LOW);
 
  // 用固定密码连接
  //wificonnect();
  //web配网
  //webconnect1(); // 联网过程中 黑屏  AP名称  AutoConnectAP
    
    webconnect();  // 联网过程中 显示WAIT WIFI AP名称 我的万年历
 //以下为服务器程序 
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());              // 通过串口监视器输出连接的WiFi名称
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());           // 通过串口监视器输出ESP8266-NodeMCU的IP

 
  // 启动闪存文件系统

  if(SPIFFS.begin()){                       
    Serial.println("SPIFFS Started.");
  } else {
    Serial.println("SPIFFS Failed to Start.");
  }
  
  esp8266_server.on("/LED-Control", handleLEDControl);   
  esp8266_server.onNotFound(handleUserRequet);      // 告知系统如何处理用户请求

  esp8266_server.begin();                           // 启动网站服务
  
  Serial.println("HTTP server started");               
//////////////
   
   configTime(TZ_SEC, DST_SEC, "ntp.ntsc.ac.cn", "ntp1.aliyun.com"); //ntp获取时间，你也可用其他"pool.ntp.org","0.cn.pool.ntp.org","1.cn.pool.ntp.org","ntp1.aliyun.com"
  
   
}
   

void loop() {

    esp8266_server.handleClient();  // 处理用户请求

//以下6行为温度测量程序
    
    DS18B20.requestTemperatures();    //测温请求
    temp = DS18B20.getTempCByIndex(0); // 测温结果赋给temp，数据类型为浮点型，有一位小数
    //Serial.print("Temperature: ");//向串口传送Temperature:
    temperature=temp; //把浮点型转换成整型，这样温度数值中，就没有小数点了
    temperature=temperature-2; //测量结果高2度，减去2度
    
    //Serial.println(temperature); //向串口传送温度数据
    
    drawDateTime();  //液晶屏绘制时间
 
   
 // changedate();
   
  
 
  if (millis() - timeSinceLastWUpdate > (1000L * UPDATE_INTERVAL_SECS)) { //屏幕刷新
    timeSinceLastWUpdate = millis();
    
  }
}

void handleLEDControl(){
  // 从浏览器发送的信息中获取PWM控制数值（字符串格式）
  //String ledPwm = esp8266_server.arg("ledPwm");
  String value1 = esp8266_server.arg("value1"); 
  String value2 = esp8266_server.arg("value2");
  String value3 = esp8266_server.arg("value3"); 
  String value4 = esp8266_server.arg("value4"); 

  // 将字符串格式的PWM控制数值转换为整数
  //int ledPwmVal = ledPwm.toInt();
       minlight = value1.toInt();
       maxlight = value2.toInt();
       clock1 = value3.toInt();
       clock2 = value4.toInt();
       

  // 实施引脚PWM设置
  //  analogWrite(blak, minlight);

  //  delay(2000);

  // 建立基本网页信息显示当前数值以及返回链接
  String httpBody = "minlight: " + value1 + "<br> maxlight: " + value2 + "<br> clock1: " + value3 + "<br> clock2: " + value4+"<p><a href=\"/LED.html\"><-LED Page</a></p>";           
  esp8266_server.send(200, "text/html", httpBody);
}




// 处理用户浏览器的HTTP访问

void handleUserRequet() {         
     
 // 获取用户请求资源(Request Resource）
  String reqResource = esp8266_server.uri();  //用户浏览器请求的资源名称
  Serial.print("reqResource: ");
  Serial.println(reqResource);
  // 通过handleFileRead函数处理用户访问
  bool fileReadOK = handleFileRead(reqResource);

  // 如果在SPIFFS无法找到用户访问的资源，则回复404 (Not Found)
  if (!fileReadOK){                                                 
    esp8266_server.send(404, "text/plain", "404 Not Found"); 
  }
}

bool handleFileRead(String resource) {            //处理浏览器HTTP访问

  if (resource.endsWith("/")) {                   // 如果访问地址以"/"为结尾
    resource = "/index.html";                     // 则将访问地址修改为/index.html便于SPIFFS访问
  } 
  
  String contentType = getContentType(resource);  // 获取文件类型
  
  if (SPIFFS.exists(resource)) {                     // 如果访问的文件可以在SPIFFS中找到
    File file = SPIFFS.open(resource, "r");          // 则尝试打开该文件
    esp8266_server.streamFile(file, contentType);// 并且将该文件返回给浏览器
    file.close();                                // 并且关闭文件
    return true;                                 // 返回true
  }
  return false;                                  // 如果文件未找到，则返回false
}

// 获取文件类型
String getContentType(String filename){
  if(filename.endsWith(".htm")) return "text/html";
  else if(filename.endsWith(".html")) return "text/html";
  else if(filename.endsWith(".css")) return "text/css";
  else if(filename.endsWith(".js")) return "application/javascript";
  else if(filename.endsWith(".png")) return "image/png";
  else if(filename.endsWith(".gif")) return "image/gif";
  else if(filename.endsWith(".jpg")) return "image/jpeg";
  else if(filename.endsWith(".ico")) return "image/x-icon";
  else if(filename.endsWith(".xml")) return "text/xml";
  else if(filename.endsWith(".pdf")) return "application/x-pdf";
  else if(filename.endsWith(".zip")) return "application/x-zip";
  else if(filename.endsWith(".gz")) return "application/x-gzip";
  return "text/plain";
}
