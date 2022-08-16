#include <vfs_api.h>
#include <FSImpl.h>
#include <FS.h>
#define CONFIGFILE "/main.ini"
#include "genral_exec.h"
//#define DEBUG_PLUS
//#define DEBUG_MINUS
//#define DEBUG_GPS
//#define NOGPSDEBUG
//#define DEBUG_WATER
#define GPSTRACKER
#define NOGPS_DATE 0,9,9,16,07,2022
#ifdef NOGPSDEBUG
unsigned nogpsdate[][6] = {{23,57,9,15,07,2022},{23,57,10,15,07,2022},{23,59,9,15,07,2022},{0,1,9,16,07,2022},{0,3,9,16,07,2022},{0,5,9,16,07,2022},{0,7,9,16,07,2022},{0,9,9,16,07,2022}};
int nogps_i=0;
#endif
#ifdef NOGPSDEBUG
#define TIME_UPDATE 3600
#else
#define TIME_UPDATE 3600
#endif 
#define water_btn 21
#define orange_btn 13
#define yellow_btn 39
#define green_btn 34
#define blue_btn 35
#define white_btn 32
#define violet_btn 33

#define lightgreen_rel 5
#define brown_rel 15
#define orange_rel 18
#define yellow_rel 4
#define green_rel 19
#define blue_rel 14
#define white_rel 27
#define violet_rel 26
#define NOFRELAYS sizeof(relays)/sizeof(relays[0])
int relays[]={violet_rel,white_rel,blue_rel,green_rel,yellow_rel,orange_rel,brown_rel,lightgreen_rel};
#define NOFBTNS sizeof(btns)/sizeof(btns[0])
int btns[]={violet_btn,white_btn,blue_btn,green_btn,yellow_btn,orange_btn/*,water_btn*/};
float sensors[3];
String sensorsname[3]={"Volume","Temperature","Humidity"};
#define Vol 0
#define Tem 1
#define Hum 2
#define NOFSENSORS sizeof(sensors)/sizeof(sensors[0])
#include <DHT.h>      // подключаем библиотеку для датчика
DHT dht(22, DHT11);

#include "BluetoothSerial.h"

BluetoothSerial SerialBT;

#include <Time.h>
//#include <TimeLib.h>
#include <TimeAlarms.h>

//#include <TinyGPS.h>
#include "TinyGPS++.h"
TinyGPSPlus gps;
//TinyGPS gps; 
const int offset = 3;
#define GPS_PIN_TX 17
#define GPS_PIN_RX 16
time_t prevDisplay = 0;

AlarmId id;

#include <CircularBuffer.h>

CircularBuffer<char, 1200> buffer;

volatile unsigned long pulseCount;

void IRAM_ATTR pulseCounter()
{
  pulseCount++;
}
void dprint(String s,byte log=1)
{ char bb[50];
  Serial.print(s);
  SerialBT.print(s);
//  sprintf(bb,"%s",s);
 if(log==1)
  {
  s.toCharArray(bb, 50); 
  for(int i=0;i<s.length();i++)
   {
     buffer.push(bb[i]);
   }
  }
}
void dprintln(String s,byte log=1)
{
  char bb[50];
  Serial.println(s);
  SerialBT.println(s);
//   sprintf(bb,"%s\n",s); 
 if(log==1)
  {
  s.toCharArray(bb, 50); 
  for(int i=0;i<s.length();i++)
   {
    buffer.push(bb[i]);
   }
    buffer.push('\n');
  }
}
void dprint(long s,byte log=1)
{
  dprint(String(s),log);
  /*char bb[50];
  Serial.print(s);
  SerialBT.print(s);*/
/*  sprintf(bb,"%s",s); 
  for(int i=0;i<String(s).length()+1;i++)
   {
    buffer.push(bb[i]);
   }*/
}
void dprintln(long s,byte log=1)
{
  dprintln(String(s),log);
 /* char bb[50];
  Serial.println(s);
  SerialBT.println(s);
  sprintf(bb,"%d\n",s); */
 /*   for(int i=0;i<String(s).length()+1;i++)
   {
    buffer.push(bb[i]);
   }*/
}
void print_sensors()
{
  for(int i=0;i<NOFSENSORS;i++)
   {
    dprint(sensorsname[i]);dprint(":");
    dprintln(String(sensors[i]));
   }
}
void printDigits(int digits,byte log=1)
{
  // utility function for digital clock display: prints preceding colon and leading 0
  if(digits < 10)
    dprint("0",log);
  dprint(digits,log);
}



void digitalClockDisplay(){
  // digital clock display of the time
//  dprint("time ajust to: ");
  dprint(hour());
  dprint(":");
  printDigits(minute());
  dprint(":");
  printDigits(second());
  dprint(" ");
  dprint(day());
  dprint(".");
  dprint(month());
  dprint(".");
  dprint(year()); 
  dprint(" ");
  dprint(weekday()); 
  dprintln(""); 
}

void digitalClockDisplay(time_t t,byte log=1){

  dprint(hour(t),log);  dprint(":",log);
  printDigits(minute(t),log);  dprint(":",log);
  printDigits(second(t),log);
  dprint(" ",log);
  printDigits(day(t),log);
  dprint(".",log);
  printDigits(month(t),log);
  dprint(".",log);
  dprint(year(t),log); 
  dprint(" ",log);
  dprint(weekday(t),log); 
  dprintln("",log); 
}
 

//---------------------------------------------------------------------------
scheduler::scheduler(String _taskname,int8_t _StHour,int8_t _StMin,int8_t _StDayofWeek,unsigned long _duration,float _startV,float _stopV,byte _orderV,float _startT,float _stopT,byte _orderT,float _startH,float _stopH,byte _orderH)
   :taskname(_taskname),StHour(_StHour),StMin(_StMin),StDayofWeek(_StDayofWeek),duration(_duration)
{
startD[0]=_startV;
stopD[0]=_stopV;
orderD[0]=_orderV;
startD[1]=_startT;
stopD[1]=_stopT;
orderD[1]=_orderT;
startD[2]=_startH;
stopD[2]=_stopH;
orderD[2]=_orderH;
aID=-1; 
finish=0;
}

#define NOFALARMS 5

#define v1_3bochki 1000000
#define v1_2bochki 1500000
#define v2_3bochki 2000000
#define SCHEDULER_L sizeof(scheduler_arr)/sizeof(scheduler)
#define TASKS_L sizeof(tasks_arr)/sizeof(task)
#ifdef GPSTRACKER
#include "buildTime.h"
#define baseH 19
#define baseM 00
scheduler scheduler_arr[]={
  {"Violet task",BUILD_HOUR,BUILD_MIN+6, 3,40000,0,2,1,10,300,1,0,110,1},
  {"White task",BUILD_HOUR,BUILD_MIN+2,  3,30000,0,2,1,10,300,1,0,110,1},
  {"blue task",BUILD_HOUR,BUILD_MIN+3,  -1,15000,0,5,1,10,300,1,0,110,1},
  {"green task",BUILD_HOUR,BUILD_MIN+4,  3,15000,0,3,1,10,300,1,0,110,1},
  {"yellow task",BUILD_HOUR,BUILD_MIN+5,-1,15000,0,5,1,10,300,1,0,110,1},
   {"tempr task",-1,-1,-1,                  5000,1,2,1,10,300,1,0,110,1}

 /* {"Orange task",-1,-1,-1,0,-1,0}/*,
  {"Water task",water_btn,orange_rel,-1,-1,-1,0,0,&stopInit,&stopStart,&stopExec,&stopFin}*/
  };
 
#else
scheduler scheduler_arr[]={
  {"Violet task",10,0,6,v1_2bochki,0,100,1,10,300,1,0,110,1},
  {"White task",10,0,7,v1_2bochki,0,100,1,10,300,1,0,110,1},
  {"blue task",18,30,-1,v1_3bochki,0,60,1,10,300,1,0,110,1},
  {"green task",10,0,1,v1_2bochki,0,100,1,10,300,1,0,110,1},
  {"yellow task",19,05,-1,v1_3bochki,0,60,1,10,300,1,0,110,1}
 /* {"Orange task",-1,-1,-1,0,-1,0}/*,
  {"Water task",water_btn,orange_rel,-1,-1,-1,0,0,&stopInit,&stopStart,&stopExec,&stopFin}*/
  };
#endif




//---------------------------------------------------------------------------


class general_init: public general_do
{
  public:

 
   virtual void handle(task *x) {
#ifdef DEBUG_PLUS
    digitalClockDisplay();
    dprint (x->taskname);dprintln(" init");
#endif
for(int i=0;i<NOFRELAYS;i++)
 {
   if(x->relays[i])
    pinMode(relays[i], OUTPUT);
    digitalWrite(relays[i], HIGH);
 }
    pinMode(x->btn, INPUT);
//    digitalWrite(x->relay, HIGH);
    x->stat=0;
/*    //todo добавление в расписание
    if(x->StDayofWeek>0)
     {
     x->aID=Alarm.alarmRepeat((timeDayOfWeek_t)x->StDayofWeek,x->StHour,x->StMin,0,doalarm);
     }
    else
     {
     if(x->StHour>0)
      {
       x->aID=Alarm.alarmRepeat(x->StHour,x->StMin,0,doalarm);   
      }
     } */

   }
} generalInit;

class general_start: public general_do
{
  public:
   virtual void handle(task *x) {
    digitalClockDisplay();
    dprint(x->taskname);dprintln(" started");
    if(scheduler_arr[x->schedulerID].StMin>0)  //если задача перманентная то задержка 49 дней (раньше сработает штатная перезагрузка) 
     {
        x->finish=scheduler_arr[x->schedulerID].duration+millis();
        pulseCount=0;
     if(Alarm.count()<NOFALARMS)
      {
      x->aID=-1;
      scheduler_arr[x->schedulerID].aID=-1;
      x->schedulerID=-1;
      bindShedulertoAlarm(getnextscheduler());
      }
     }
       else
        {
        x->finish=4294967290;
        }
    for(int i=0;i<NOFRELAYS;i++)
     {
      if(x->relays[i])
        digitalWrite(relays[i],LOW);
     }
  //  digitalWrite(x->relay,LOW);
 //   digitalWrite(lightgreen_rel,LOW);
  //  digitalWrite(brown_rel,LOW);
    x->stat=1;
   }
} generalStart;

class general_exec: public general_do
{
  public:
   virtual void handle(task *x) {

bool sens=true;
    for(int i=0;i<NOFSENSORS;i++)
     {
      if(sensors[i]<-10000)
       sens*=1;
      else 
       sens*=(scheduler_arr[x->schedulerID].startD[i]<=sensors[i] && sensors[i]<=scheduler_arr[x->schedulerID].stopD[i])?scheduler_arr[x->schedulerID].orderD[i]:(!scheduler_arr[x->schedulerID].orderD[i]);
#ifdef DEBUG_MINUS   
     dprint(x->taskname);dprint(" sens ");dprint(i); dprint(" value");dprint(sens);dprint(" sensor value  ");dprintln(sensors[i]);
#endif   
     }
    if(sens)
     {
     for(int i=0;i<NOFRELAYS;i++)
     {
      if(x->relays[i])
        digitalWrite(relays[i],LOW);
     }
     if(x->interruped!=0)
      {
       digitalClockDisplay();
       dprint(x->taskname);dprintln(" resume"); 
       dprint("sensors: ");
       print_sensors();
      x->interruped=0;
      }
     }
    else
     {
      if(scheduler_arr[x->schedulerID].StMin<0) //если это перманентная задача
       {
        if(scheduler_arr[x->schedulerID].finish==0 && (!x->interruped))
         {
          scheduler_arr[x->schedulerID].finish=millis()+scheduler_arr[x->schedulerID].duration;  //то duration это задержка отключения после выхода датчика из диапазона
         }
       }

     //  Serial.println(scheduler_arr[x->schedulerID].finish-millis());
      if( (scheduler_arr[x->schedulerID].finish<millis() && scheduler_arr[x->schedulerID].StMin<0)|| scheduler_arr[x->schedulerID].StMin>=0) // если вышло время задержки или задача не перманентная - выключаем реле
       {    
       for(int i=0;i<NOFRELAYS;i++)
        {
        if(x->relays[i])
        digitalWrite(relays[i],HIGH);
        }
       if(!x->interruped)
        {
       digitalClockDisplay();
       dprint(x->taskname);dprintln(" interruped"); 
       dprint("sensors: ");
       print_sensors();
      x->interruped=1;
       }
       if(scheduler_arr[x->schedulerID].StMin<0) // Если задача перманентная сбрасываем задержку
        {
           scheduler_arr[x->schedulerID].finish=0;
        }
      }
     }
   }
} generalExec;

class general_fin: public general_do
{
  public:
   virtual void handle(task *x) {
    if(x->stat!=0)
     {
    digitalClockDisplay();
    dprint(x->taskname);dprintln(" stoped");
    print_sensors();
    for(int i=0;i<NOFRELAYS;i++)
     {
      if(x->relays[i])
        digitalWrite(relays[i],HIGH);
     }
    x->stat=0;
     }
   }
} generalFin;

//------------------------------------------------------------------------------
class stop_init: public general_do
{
  public:
   virtual void handle(task *x) {
   pinMode(x->btn, INPUT);    
   }
} stopInit;

class stop_start: public general_do
{
  public:
   virtual void handle(task *x) {
    digitalClockDisplay();
    dprint(x->taskname);dprintln(" emergancy STOP");
    emergency_stop();
   }
} stopStart;

class stop_exec: public general_do
{
  public:
   virtual void handle(task *x) {

   }
} stopExec;

class stop_fin: public general_do
{
  public:
   virtual void handle(task *x) {

   }
} stopFin;




//----------------------------------------------------------------------------







task::task(String _taskname,int _btn,general_do * _init,general_do * _start,general_do * _exec,general_do * _fin,byte _r0,byte _r1,byte _r2,byte _r3,byte _r4,byte _r5,byte _r6,byte _r7)
   :taskname(_taskname),btn(_btn),init(_init),start(_start),exec(_exec),fin(_fin)
{
#ifdef DEBUG_PLUS
dprintln("--------------------init--------------"); 
#endif
aID=-1;
schedulerID=-1;
interruped=0;
relays[0]=_r0;relays[1]=_r1;relays[2]=_r2;relays[3]=_r3;relays[4]=_r4;relays[5]=_r5;relays[6]=_r6;relays[7]=_r7;
}

void task::check()
{
  if(digitalRead(btn)==HIGH)
   {
    start->handle(this);
   }
  else
   {
    if(finish>millis())
     {
#ifdef DEBUG_MINUS
dprintln(finish-millis());
#endif
      exec->handle(this);
 
     }
    else
     {
      fin->handle(this);
     }
   }
}
//int relays[]={violet_rel,white_rel,blue_rel,green_rel,yellow_rel,orange_rel,brown_rel,lightgreen_rel};


task tasks_arr[]={
  {"Violet task",violet_btn,&generalInit,&generalStart,&generalExec,&generalFin,1,0,0,0,0,0,1,1},
  {"White task",white_btn,&generalInit,&generalStart,&generalExec,&generalFin,0,1,0,0,0,0,1,1},
  {"blue task",blue_btn,&generalInit,&generalStart,&generalExec,&generalFin,0,0,1,0,0,0,1,1},
  {"green task",green_btn,&generalInit,&generalStart,&generalExec,&generalFin,0,0,0,1,0,0,1,1},
  {"yellow task",yellow_btn,&generalInit,&generalStart,&generalExec,&generalFin,0,0,0,0,1,0,1,1},
  {"Orange task",orange_btn,&stopInit,&stopStart,&stopExec,&stopFin,0,0,0,0,0,0,0,0}
  ,
  {"tempr task",orange_btn,&generalInit,&generalStart,&generalExec,&generalFin,0,0,0,0,0,0,0,0}
  };
 
int getnextscheduler()
{
  
  //time_t value = (DOW-1) * SECS_PER_DAY + AlarmHMS(H,M,S);
  //nextTrigger = value + nextSunday(time);
  //nextTrigger = value + previousMidnight(time);
int nexttask=-1;
int j=0;
time_t ntime = now();
time_t tt;
time_t ct=now()+8*SECS_PER_DAY;
  for (int i=0;i<SCHEDULER_L;i++) 
  {
  
  for (j=0;j<TASKS_L;j++)
   {
    if(tasks_arr[j].taskname.equals(scheduler_arr[i].taskname)) 
     break;
   }
   if(scheduler_arr[i].StMin>=0) //Только если не перманентная задача
    {
    if((scheduler_arr[i].aID<0) && (tasks_arr[j].aID<0))
     {
      if(scheduler_arr[i].StDayofWeek>0)
       {
        tt=previousSunday(ntime)+(scheduler_arr[i].StDayofWeek-1) * SECS_PER_DAY + AlarmHMS(scheduler_arr[i].StHour,scheduler_arr[i].StMin,0);
        if(tt<=ntime)
         {
          tt=nextSunday(ntime)+(scheduler_arr[i].StDayofWeek-1) * SECS_PER_DAY + AlarmHMS(scheduler_arr[i].StHour,scheduler_arr[i].StMin,0);
         }
       }
      else
       {
        tt=previousMidnight(ntime)+AlarmHMS(scheduler_arr[i].StHour,scheduler_arr[i].StMin,0);
                if(tt<=ntime)
         {
           tt=nextMidnight(ntime)+AlarmHMS(scheduler_arr[i].StHour,scheduler_arr[i].StMin,0);
         }
       }
      if(tt<ct)
       {
        ct=tt;
        nexttask=i;
       }
     }
    }
  }
#ifdef DEBUG_PLUS
dprint("got next scheduler");
dprintln(nexttask);
#endif    
  return(nexttask);
}

int bindShedulertoAlarm(int id)
{
  int aID=-1;
   for (task & ntask : tasks_arr)
    {
      if(ntask.aID<0)
       {
   
        if(ntask.taskname.equals(scheduler_arr[id].taskname))
         {
          if(scheduler_arr[id].StDayofWeek>0)
           {
            aID=Alarm.alarmOnce((timeDayOfWeek_t)scheduler_arr[id].StDayofWeek,scheduler_arr[id].StHour,scheduler_arr[id].StMin,0,doalarm);
          }
          else
          {
          aID=Alarm.alarmOnce(scheduler_arr[id].StHour,scheduler_arr[id].StMin,0,doalarm);
          }
#ifdef DEBUG_PLUS
dprint("Set ");
dprint(ntask.taskname);
dprint(" on:");
digitalClockDisplay(Alarm.getNextTrigger(aID));
#endif     
        scheduler_arr[id].aID=aID;
        ntask.aID=aID;
        ntask.schedulerID=id;
        break;
         }
        }
    }
   return(aID);
}

#define USE_SPECIALIST_METHODS
void emergency_stop()
{
    for (task & ntask : tasks_arr) 
     {
     ntask.finish=0;
     ntask.fin->handle(&ntask);
     }

    for (task & ntask : tasks_arr) 
     {
     if(ntask.aID>=0)
      {
     dprint(ntask.aID);dprint(" "); dprint(ntask.taskname);dprint(" ");dprint(Alarm.read(ntask.aID));dprint(" ");
     digitalClockDisplay(Alarm.read(ntask.aID),1);
      }
     }
  setTimefromGPS(60000);
}

void doalarm()
    {
     int id;
      id=Alarm.getTriggeredAlarmId();
       for (task & ntask : tasks_arr) 
        {
          if(ntask.aID==id)
           {
            ntask.start->handle(&ntask);
           }
           
        }
    }
byte pushbuffer=0;

void btCallback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param)
{
  if(event == ESP_SPP_SRV_OPEN_EVT){
    Serial.println("Client Connected!");
    pushbuffer=1;
  }
}

void refresh_sensors()
{
  sensors[Vol]=pulseCount*2.25/1000.;
  float hp = dht.readHumidity();
  float tp = dht.readTemperature();
  if(!isnan(hp)) sensors[Hum]=hp;
  if(!isnan(tp)) sensors[Tem]=tp;
}


void setup() {
Serial.begin(115200);
/*File file = SPIFFS.open(CONFIGFILE, FILE_READ);
if(file)
{
  
}*/


SerialBT.begin("poliv");
dht.begin();
SerialBT.register_callback(btCallback);
int i;
for (i=0;i<NOFRELAYS;i++)
 {
  pinMode(relays[i], OUTPUT);
  digitalWrite(relays[i], HIGH);
 }
for(i=0;i<NOFBTNS;i++)
 {
  pinMode(btns[i], INPUT);
 }

   Serial1.begin(9600, SERIAL_8N1, GPS_PIN_TX, GPS_PIN_RX);
   Serial1.setRxBufferSize(1024);
   Alarm.delay(500);  
   dprintln("Search GPS..");
   Alarm.delay(1000);
#ifdef GPSTRACKER
// Если для отладки использую плату трекера
 pinMode(23, OUTPUT);
 digitalWrite(GPIO_NUM_23, HIGH);
#endif

refresh_sensors();
setTime(setTimefromGPS(60*30*1000));
setSyncProvider(CsetTimefromGPS);
setSyncInterval(TIME_UPDATE);
// Иницализируем задачи
for (task & ntask : tasks_arr) ntask.init->handle(&ntask);
// Устанавливаем таймеры
for(int i=0;i<NOFALARMS;i++)
 {
  bindShedulertoAlarm(getnextscheduler());
 }
for (i=0;i<SCHEDULER_L;i++) 
 {
  if(scheduler_arr[i].StMin<0)  // ищем и запускаем перманентные задачи
   {
    for (task & ntask : tasks_arr) 
     {
      if(ntask.taskname.equals(scheduler_arr[i].taskname))
       {
        ntask.schedulerID=i;
        ntask.start->handle(&ntask); 
        scheduler_arr[i].finish=0; 
       }
     }
   }
 }
//Подключаем расходомер
pinMode(water_btn, INPUT_PULLUP);
attachInterrupt(digitalPinToInterrupt(water_btn), pulseCounter, FALLING);
//Сбрасываем значения датчиков в неопределено 
 for(int i=0;i<NOFSENSORS;i++)
  {
    sensors[i]=-10001;
  }
}


time_t CsetTimefromGPS()
{
  return(setTimefromGPS(20000));
}


 time_t setTimefromGPS(unsigned long dt)
{
unsigned long st=millis();
 tmElements_t tm;
 time_t tt;
 print_sensors();
/*if(gps.date.year()>=2022)
 {
 dprint("Now is: ");
 digitalClockDisplay();
 }*/
#ifdef NOGPSDEBUG
//setTime(NOGPS_DATE);
 tm.Year=nogpsdate[nogps_i][5]-1970;
 tm.Month=nogpsdate[nogps_i][4];
 tm.Day=nogpsdate[nogps_i][3];
 tm.Hour=nogpsdate[nogps_i][0];
 tm.Minute=nogpsdate[nogps_i][1];
 tm.Second=nogpsdate[nogps_i][2];
 

  tt=makeTime(tm)+(offset * SECS_PER_HOUR);
  nogps_i++;
 //setTime(tt+offset * SECS_PER_HOUR);
// Alarm.delay(1000);
 dprint("Set DEBUG Time ");

#else
   do
  {
     while (Serial1.available()) {
      char g;
      g = Serial1.read();
#ifdef    DEBUG_GPS
    Serial.print(g);
#endif
      gps.encode(g);
      }
   if((st+dt)<millis())
      return(0);
  }
 while (((!gps.location.isValid()) || (gps.date.year()<2022) || (gps.location.age()>1000)));
 //unsigned long age;
 //int Year;
 //byte Month, Day, Hour, Minute, Second;

/* Year=gps.date.year();
 Month=gps.date.month();
 Day=gps.date.day();
 Hour=gps.time.hour();
 Minute=gps.time.minute();
 Second=gps.time.second();*/
 tm.Year=gps.date.year()-1970;
 tm.Month=gps.date.month();
 tm.Day=gps.date.day();
 tm.Hour=gps.time.hour();
 tm.Minute=gps.time.minute();
 tm.Second=gps.time.second();
 //age=gps.time.age();
 tt=makeTime(tm)+offset * SECS_PER_HOUR;;

 //setTime(t);
 // Alarm.delay(1000);
 dprint("Got time from GPS ");
 #endif
  digitalClockDisplay(tt);
return(tt);

}

 
void loop() {
  int i;
  Alarm.delay(10);
  if (Serial1.available()) {  //непрерывно читаем GPS 
   char g;
   g = Serial1.read();
#ifdef    DEBUG_GPS
    Serial.print(g);
#endif
   gps.encode(g);
   }
 refresh_sensors();
 for (task & ntask : tasks_arr) ntask.check();  // непрерывно запускаем check по всем задачам
 if(pushbuffer==1)   //при подключении bluetooth Serial выводим в порт log, расписание и т.п.
  {
  pushbuffer=0;
  using index_t = decltype(buffer)::index_t;
  for (index_t i = 0; i < buffer.size()-2; i++) {
     SerialBT.print((char)buffer[i]);
     }
   for (int j = 0; j < 6; j++) {
    int k=0;
    for (task & ntask : tasks_arr) 
     {
      if(ntask.aID==j)
       {
        dprint(ntask.aID);dprint(" "); dprint(ntask.taskname);dprint(" ");digitalClockDisplay(Alarm.getNextTrigger(j),0);dprint(" ");
        k=1;
        break;
       }
     }
    if(k==0)
      {
       dprint(j);dprint(" ");digitalClockDisplay(Alarm.getNextTrigger(j),0);
      }
      k=0;
     }
//   float hp = dht.readHumidity();
//   float tp = dht.readTemperature();
   dprint("Volume:");  dprint(String(sensors[Vol]));
   dprint("Humidity:");  dprint(String(sensors[Hum]));
   dprint("Temperature:");  dprintln(String(sensors[Tem]));
  }
if(millis()>4000000000) 
 {
  byte rst=1;
   for (task & ntask : tasks_arr) 
    {
    if(ntask.finish>millis()) rst=0;   
    }
  if(rst) ESP.restart();  //если вдруг все долго работает без перезагрузок, и нет запущенных задач то лучше бы перезагрузиться
 }
}
