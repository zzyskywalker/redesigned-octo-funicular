/************led******************/
#include <Adafruit_NeoPixel.h>//引用头文件
#define PIN 4   /*定义了控制LED的引脚，6表示Microduino的D6引脚，可通过Hub转接出来，用户可以更改 */
#define PIN_NUM 1 //定义允许接的led灯的个数
#define val_max 255
#define val_min 0   
Adafruit_NeoPixel strip = Adafruit_NeoPixel(PIN_NUM, PIN, NEO_GRB + NEO_KHZ800); //该函数第一个参数控制串联灯的个数，第二个是控制用哪个pin脚输出，第三个显示颜色和变化闪烁频率

//该函数第一个参数控制串联灯的个数，第二个是控制用哪个pin脚输出，第三个显示颜色和变化闪烁频率
/****************led*************/
/**********放歌*********************/
#include "audio.h"//控制audio模块文件
#include <SoftwareSerial.h>
#include <IRremote.h>
int RECV_PIN = 12;   //红外线接收器OUTPUT端接在pin 12
IRrecv irrecv(RECV_PIN);  //定义IRrecv对象来接收红外线信号
decode_results results;   //解码结果放在decode_results构造的对象results里
#define music_num_MAX 9        //歌曲最大数，可改
bool play=true;
int music_vol = 20; //初始音量0~30
int music_num = 1;  //初始化第一首歌，范围1-9
/************放歌*********************/
#include <Microduino_Tem_Hum.h>
Tem_Hum_S2  termo; //调用Sensor-Tem&Hum-S2传感器
#include <Wire.h>
#include "Rtc_Pcf8563.h"//使用rtc模块
#include <U8glib.h>//使用OLED需要包含这个头文件
#define INTERVAL_LCD 20 //定义OLED刷新时间间隔 
unsigned long lcd_time = millis(); //OLED刷新时间计时器
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE); //设置OLED型号 
//-------字体设置，大、中、小
#define setFont_M u8g.setFont(u8g_font_fixed_v0r)
Rtc_Pcf8563 rtc;//定义rtc
#define crash 10//按键的引脚
#define buzzer_pin 8 //定义蜂鸣器驱动引脚
#define buzzer_fre 600 //定义蜂鸣器输出频率
int buttonState;
void setup() {
Wire.begin();
Serial.begin(9600);  //设置串口通讯波特率
pinMode(crash,INPUT);//设置按键输入状态
pinMode(buzzer_pin, OUTPUT);
settime(17,12,16,6,21,06,59);//年，月，日，星期，时，分，秒设置时间
irrecv.enableIRIn(); // 启动红外解码
audio_pause();
audio_init(DEVICE_Flash, MODE_loopAll, music_vol); //初始化mp3模块
 Serial.print("Tem_Hum_S2:");
 Serial.print(termo.begin());  //初始化温湿度传感器，并串口打印传感器是否在线
 strip.begin();   //准备对灯珠进行数据发送
 strip.show(); //初始化LED为关的状态
}


void loop() {
if(termo.getTemperature()<30){   //气温过低，警报
  strip.setPixelColor(0, strip.Color(255, 0, 0));//红光
  strip.show();   //LED显示
  }

 coutOLED();//输出到OLED
if(rtc.getHour()==21&&rtc.getMinute()==6&&rtc.getSecond()==59)//设置响铃时间
{
 tone(buzzer_pin, buzzer_fre);   //驱动蜂鸣器
    delay(100);
  Serial.println("闹钟响了");}
  if(!digitalRead(crash))  
  { noTone(buzzer_pin);//蜂鸣器不响
    Serial.println("闹钟停");
    }
  
/**********************放歌******************************/
if (irrecv.decode(&results))
  {//解码成功，收到一组红外线信号
          if (results.value==33441975)
           {
             if(play==true)        
            { audio_play();Serial.println("kaishi");}
             else {audio_pause();Serial.println("tingzhi");}
             play=!play;
            }
          if(results.value==33464415)
           {
              music_vol++;                 //音量+1
             if(music_vol>30) music_vol=30;      //若音量大于30，则音量为30
             audio_vol(music_vol);
             Serial.println("++");       //Serial.println函数输出“+1”
                                   
          }
         if(results.value==33478695)
           {
             music_vol--;                   //音量减1
             if(music_vol<1) music_vol=1;   //如果音量小于1，音量为1
             audio_vol(music_vol);
             Serial.println("--");         //Serial.println函数输出“-1”
                       
           }
         if(results.value==33460335)
           {
            music_num++; //歌曲序号加
            if(music_num>music_num_MAX) //限制歌曲序号范围，如果歌曲序号大于9
            {
              music_num=1;   //歌曲序号返回1
            }
             audio_choose(music_num);
             audio_play();Serial.println("xiayishou");
            }
         if(results.value==33480735)
           {
             music_num--;  //歌曲序号减1
             if(music_num<1) //限制歌曲序号范围，如果歌曲序号小于1
               {
                  music_num=music_num_MAX;     //歌曲序号为最大（9）
               }
              audio_choose(music_num);       //音频选择歌曲序号
              audio_play();   Serial.println("shangyishou");               //音频工作
           }
          irrecv.resume(); //  接收下一个值 
  }
/**********************放歌*****************************/
}

void settime(int _year, int _month, int _day, int _week, int _hour, int _min, int _sec)
{
  //clear out the registers
  rtc.initClock();
  //set a time to start with.
  //day, weekday, month, century(1=1900, 0=2000), year(0-99)
  rtc.setDate(_day, _week, _month, 0, _year);
  //hr, min, sec
  rtc.setTime(_hour, _min, _sec);
}

void coutOLED()
{
      //both format functions call the internal getTime() so that the
  //formatted strings are at the current time/date.
  Serial.println("CODE_1:");
  Serial.print(rtc.formatDate());//获取日期
  Serial.print("    ");
  Serial.println(rtc.formatTime());//获取时间

  Serial.println("CODE_2:");
  Serial.print(rtc.getYear());//获取年
  Serial.print("/");
  Serial.print(rtc.getMonth());//获取月
  Serial.print("/");
  Serial.print(rtc.getDay());//获取日
  Serial.print("     ");
  Serial.print(rtc.getHour());//获取时
  Serial.print(":");
  Serial.print(rtc.getMinute());//获取分
  Serial.print(":");
  Serial.println(rtc.getSecond());//获取秒
   u8g.firstPage();
 do {
 setFont_M;
 u8g.setPrintPos(3, 10);
 u8g.print("Date:");
 u8g.print(rtc.formatDate());//打印日期
 u8g.setPrintPos(3, 20);
 u8g.print("Time:");
 u8g.print(rtc.formatTime());//打印时间
 u8g.setPrintPos(3, 30);
 u8g.print("Tem:");
 u8g.print(termo.getTemperature());//打印温度
 u8g.setPrintPos(3, 40);
 u8g.print("Hum:");
 u8g.print(termo.getHumidity());//打印湿度
  }while( u8g.nextPage() );

 // delay();
  Serial.print("\r\n");
}




