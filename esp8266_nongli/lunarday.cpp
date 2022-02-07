//lunarday.cpp
#include "lunarday.h"
#include <ctime>
#include <cassert>
#include <cstring>
#include <iostream>
 
using namespace std;
using lunar::Date;
 
 
//使用比特位记录每年的情况
 
//0~4 共5bit 春节日份
 
//5~6 共2bit 春节月份
 
//7~19 共13bit 13个月的大小月情况(如果无闰月，最后位无效)，大月为1,小月为0
 
//20~23 共4bit 记录闰月的月份，如果没有闰月为0
 
static const int BEGIN_YEAR = 1901;
static const int NUMBER_YEAR = 199;
static const unsigned int LUNAR_YEARS[199] = {
   0x03a953,0x0752c8,0x5b253d,0x0325d0,0x054dc4,
   0x4aab39,0x02b54d,0x05acc2,0x2ba936,0x03a94a, //1901-1910
 
   0x6d92be,0x0592d2,0x0525c6,0x5a55ba,0x0156ce,
   0x02b5c3,0x25b4b7,0x06d4cb,0x7ec941,0x074954, //1911-1920
 
   0x0692c8,0x5d26bc,0x052b50,0x015b45,0x4adab8,
   0x036a4d,0x0754c2,0x2f4937,0x07494a,0x66933e, //1921-1930
 
   0x0295d1,0x052bc6,0x595b3a,0x05ad4e,0x036a44,
   0x3764b8,0x03a4cb,0x7b49bf,0x0549d3,0x0295c8, //1931-1940 
 
   0x652dbb,0x0556cf,0x02b545,0x4daab9,0x05d24d,
   0x05a4c2,0x2d49b6,0x054aca,0x7a96bd,0x029b51, //1941-1950
 
   0x0556c6,0x5ad53b,0x02d94e,0x06d2c3,0x3ea538,
   0x06a54c,0x854abf,0x054bd2,0x02ab48,0x755abc, //1951-1960
 
   0x056acf,0x036945,0x4752b9,0x0752cd,0x032542,
   0x364bb5,0x054dc9,0x7aad3e,0x02b551,0x05b4c6, //1961-1970
 
   0x5ba93b,0x05a94f,0x0592c3,0x4b25b7,0x0525cb,
   0x8a55bf,0x0156d2,0x02b6c7,0x65b4bc,0x06d4d0, //1971-1980
 
   0x06c945,0x4e92b9,0x0692cd,0xad26c2,0x052b54,
   0x015b49,0x62dabd,0x036ad1,0x0754c6,0x5f493b, //1981-1990
 
   0x07494f,0x069344,0x352b37,0x052bca,0x8a6b3f,
   0x01ad53,0x036ac7,0x5b64bc,0x03a4d0,0x0349c5, //1991-2000
 
   0x4a95b8,0x0295CC,0x052dc1,0x2aad36,0x02b549,
   0x7daabd,0x05d252,0x05a4c7,0x5d49ba,0x054ace, //2001-2010
 
   0x0296C3,0x4556B7,0x055ACA,0x9AD53F,0x02E953,
   0x06D2C8,0x6ea53c,0x06a550,0x064AC5,0x4A9739, //2011-2020  
 
   0x02AB4C,0x055AC1,0x2AD936,0x03694A,0x6752BD,
   0x0392D1,0x0325C6,0x564BBA,0x0655CD,0x02AD43, //2021-2030
 
   0x356B37,0x05B4CB,0xBBA93F,0x05A953,0x0592C8,
   0x6D25BC,0x0525CF,0x0255C4,0x52ADB8,0x02D6CC, //2031-2040
 
   0x05B541,0x2DA936,0x06C94A,0x7E92BE,0x0692D1,
   0x052AC6,0x5A56BA,0x025B4E,0x02DAC2,0x36D537, //2041-2050
 
   0x0764CB,0x8F4941,0x074953,0x069348,0x652B3C,
   0x052BCF,0x026B44,0x436AB8,0x03AACC,0x03A4C2, //2051-2060
                                
   0x3749B5,0x0349C9,0x7A95BD,0x0295D1,0x052DC5,
   0x5AAD3A,0x02B54E,0x05B2C3,0x4BA537,0x05A54B, //2061-2070
                               
   0x8D4ABF,0x054AD3,0x0296C7,0x6556BB,0x055ACF,
   0x02D545,0x45D2B8,0x06D2CC,0x06A542,0x3E4AB6, //2071-2080
 
   0x064B49,0x7CA73D,0x02AB51,0x055AC6,0x5AD93A,
   0x03694E,0x0752C3,0x472538,0x0325ca,0x864BBE, //2081-2090
 
   0x0255D2,0x02ADC7,0x656B3B,0x05B54F,0x03A945,
   0x4B92B9,0x0592CC,0x0545C1,0x2A4DB5//2091-2099 
   
  //上表农历修正及校验完成。2021.3.3
 
};
//计算这个公历日期是一年中的第几天
 
static int DayOfSolarYear(int year, int month, int day )
{
     //为了提高效率，记录每月一日是一年中的第几天
 
     static const int NORMAL_YDAY[12] = {1,32,60,91,121,152,
                     182,213,244,274,305,335};
     //闰年的情况
 
     static const int LEAP_YDAY[12] = {1,32,61,92,122,153,
                 183,214,245,275,306,336};
     const int *t_year_yday_ = NORMAL_YDAY;
     
     //判断是否是公历闰年
 
     if( year % 4 ==0 )
     {
     if(year%100 != 0)    
     t_year_yday_ = LEAP_YDAY;
     if(year%400 == 0)
     t_year_yday_ = LEAP_YDAY;              
     }
     return t_year_yday_[month -1] + (day -1);
}
 
Date lunar::LuanrDate(int solar_year,int solar_month,int solar_day)
{
     Date luanr_date ;
     luanr_date.year = solar_year;
     luanr_date.month = 0;
     luanr_date.day = 0;
     luanr_date.leap = false;
     
     //越界检查，如果越界，返回无效日期
 
     if(solar_year <= BEGIN_YEAR || solar_year > BEGIN_YEAR + NUMBER_YEAR - 1 )
          return luanr_date;
          
     int year_index = solar_year - BEGIN_YEAR;
     
     //计算春节的公历日期
 
     int spring_ny_month = ( LUNAR_YEARS[year_index] & 0x60 ) >> 5;
     int spring_ny_day = ( LUNAR_YEARS[year_index] & 0x1f);
     
     //计算今天是公历年的第几天
 
     int today_solar_yd = DayOfSolarYear(solar_year,solar_month,solar_day);
     //计算春节是公历年的第几天
 
     int spring_ny_yd = DayOfSolarYear(solar_year,spring_ny_month,spring_ny_day);
     //计算今天是农历年的第几天
 
     int today_luanr_yd = today_solar_yd - spring_ny_yd + 1;
     //如果今天在春节的前面，重新计算today_luanr_yd
 
     if ( today_luanr_yd < 0)
     {
      //农历年比当前公历年小1
 
     year_index --;
     luanr_date.year --;
     //越界，返回无效日期
 
     if(year_index <0)
     return luanr_date;
     spring_ny_month = ( LUNAR_YEARS[year_index] & 0x60 ) >> 5;
     spring_ny_day = ( LUNAR_YEARS[year_index] & 0x1f);    
     spring_ny_yd = DayOfSolarYear(solar_year,spring_ny_month,spring_ny_day);
         
     int year_total_day = DayOfSolarYear(luanr_date.year,12,31);         
     today_luanr_yd = today_solar_yd + year_total_day - spring_ny_yd + 1;    
     }
     
     int luanr_month = 1;
     //计算月份和日期
 
     for(;luanr_month<=13;luanr_month++)
     {    
     int month_day = 29;    
     if( (LUNAR_YEARS[year_index] >> (6 + luanr_month)) & 0x1 )
     month_day = 30;    
     if( today_luanr_yd <= month_day )
     break;
     else
     today_luanr_yd -= month_day;    
     }
     luanr_date.day = today_luanr_yd;
     //处理闰月
 
     int leap_month = (LUNAR_YEARS[year_index] >>20) & 0xf;
     if(leap_month > 0 && leap_month < luanr_month )
     {    
     luanr_month --;
     //如果当前月为闰月，设置闰月标志
 
     if( luanr_month == leap_month )    
     luanr_date.leap = true;         
     }
     assert(leap_month <= 12);
     luanr_date.month = luanr_month;
     return luanr_date;
}

 



