//file:lunarday.h
 
 
namespace lunar
{
     //����һ��ũ������
 
     typedef struct T_Date
     {
     //��
 
     int year;
     //��
 
     int month;
     //��
 
     int day;
     //�Ƿ�����
 
     bool leap;    
     } Date;
     
     //����תũ��
 
     Date LuanrDate(int solar_year,int solar_month,int solar_day);
}
