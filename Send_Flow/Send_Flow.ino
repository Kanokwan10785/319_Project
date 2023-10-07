#define BLYNK_TEMPLATE_ID "TMPL6tH3kk0yR"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN "MtkLG9b6nK8Za7ijMr2vZHSEcIpEi3HS"

//https://script.google.com/macros/s/AKfycbwOyoX42twQ39hJRGqrYLzgRCBiZvz0GZJeMaV1koz6w3v3wx2Mtq6GZ5_F6TfnnwDo/exec

#include <SimpleTimer.h>
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <TimeLib.h>
#include <WidgetRTC.h>

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

WidgetRTC server_rtc;
SimpleTimer timer;

#define BLYNK_VALVE_1           V1
#define BLYNK_TIMER_1           V2
#define BLYNK_VALVE_2           V3
#define BLYNK_TIMER_2           V4

#define VALVE_ON                0
#define VALVE_OFF               1

unsigned char start_time_hour_1;
unsigned char start_time_min_1;
unsigned char stop_time_hour_1;
unsigned char stop_time_min_1;
unsigned char day_timer_1;
bool flag_timer1_en;
bool flag_timer_on_valve_1;
bool flag_valve_1_set;
bool flag_valve_1_status;
bool flag_blynk_valve_1_update;

unsigned char start_time_hour_2;
unsigned char start_time_min_2;
unsigned char stop_time_hour_2;
unsigned char stop_time_min_2;
unsigned char day_timer_2;
bool flag_timer2_en;
bool flag_timer_on_valve_2;
bool flag_valve_2_set;
bool flag_valve_2_status;
bool flag_blynk_valve_2_update;


//char ssid[] = "Sopida_2.4G";
//char pass[] = "0835074547";


char ssid[] = "243-iot";
char pass[] = "987654321";

const char TimeZone[] = "Asia/Bangkok";  

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------

WiFiClientSecure client; //--> Create a WiFiClientSecure object.

long this_time;
long lastMeasure = 0;
String GAS_ID = "AKfycbwOyoX42twQ39hJRGqrYLzgRCBiZvz0GZJeMaV1koz6w3v3wx2Mtq6GZ5_F6TfnnwDo";

long rtc_sec_server;
unsigned char weekday_server;
int Relay1 = D5;
int Relay2 = D6;
int X;
int Y;
float TIME = 0;
float FREQUENCY = 0;
float WATER = 0;
float TOTAL = 0;
float LS = 0;
int Flow = D0; // D0

BLYNK_CONNECTED() {
    Blynk.syncAll();
    server_rtc.begin();
  }

void setup()
{
    Serial.begin(115200);
    pinMode(Relay1, OUTPUT);
    pinMode(Relay2, OUTPUT);
    pinMode(Flow,INPUT_PULLUP);
    timer.setInterval(100L,FlowSensor);
    Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

    WiFi.begin(ssid, pass); //--> Connect to your WiFi router
    Serial.println("");

  //----------------------------------------Wait for connection
    Serial.print("Connecting");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(100);
  }
  //----------------------------------------If successfully connected to the wifi router, the IP Address that will be visited is displayed in the serial monitor
  Serial.println("");
  Serial.print("Successfully connected to : ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.println();
  //----------------------------------------

  client.setInsecure();
}

void loop()
{
    Blynk.run();
    timer.run();
    delay(100);
    RTC_mng();
    fn_valve_mng();
    update_blynk_data();
    this_time = millis();
    if (this_time - lastMeasure > 3000 && flag_valve_1_status && flag_valve_2_status){
        lastMeasure = this_time;
        sendData(WATER);
    }
  }

void FlowSensor(){

  X = pulseIn(Flow, HIGH);
  Y = pulseIn(Flow, LOW);
  TIME = X + Y;
  FREQUENCY = 1000000/TIME;
  WATER = FREQUENCY/7.5;
  LS = WATER/60;
  if(FREQUENCY >= 0)
   Serial.print("WATER : ");
   Serial.println(WATER);
   Blynk.virtualWrite(V9,WATER);
  {
  if(isinf(FREQUENCY))
  {
  Serial.print("TIME : ");
  Serial.println(TIME);
  Serial.print("TOTAL11 : ");
  Serial.println(TOTAL);
  Blynk.virtualWrite(V11,TOTAL);
  }
  else
  {
  TOTAL = TOTAL + LS;
  Serial.print("TOTAL10 : ");
  Serial.println(TOTAL);
  Blynk.virtualWrite(V10,TOTAL);
  }
  }

  if (WATER>3)
  {
    Blynk.logEvent("flow_noti","Maybe your pipe got a problem.");
    }
  }

// ######################################################################

BLYNK_WRITE (BLYNK_VALVE_1)
{
  int val = param.asInt();  // assigning incomming value from pin to a var

  if ( flag_timer_on_valve_1 == 0 )
    flag_valve_1_set = val;
  else
    flag_blynk_valve_1_update = 1;
  
  Serial.print("Valve 1 Set: ");
  Serial.println(val);
}

// ######################################################################
BLYNK_WRITE (BLYNK_VALVE_2)
{
  int val = param.asInt();  // assigning incomming value from pin to a var

  if ( flag_timer_on_valve_2 == 0)
    flag_valve_2_set = val;
  else
    flag_blynk_valve_2_update = 1;
  
  Serial.print("Valve 2 Set: ");
  Serial.println(val);
}


// ######################################################################
BLYNK_WRITE(BLYNK_TIMER_1)
{
  unsigned char week_day;
  
  TimeInputParam  t(param);
  
  if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
  {
     start_time_hour_1 = t.getStartHour();
     start_time_min_1 = t.getStartMinute();
     Serial.println(String("Time1 Start: ") +
                     start_time_hour_1 + ":" +
                     start_time_min_1);
    
     stop_time_hour_1 = t.getStopHour();
     stop_time_min_1 = t.getStopMinute();
     Serial.println(String("Time1 Stop: ") +
                     stop_time_hour_1 + ":" +
                     stop_time_min_1);
    
     for (int i = 1; i <= 7; i++)
     {
       if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
       {
         day_timer_1 |= (0x01 << (i-1));
       }
       else
         day_timer_1 &= (~(0x01 << (i-1)));
     }
    
     Serial.print("Time1 Selected Days: ");
     Serial.println(day_timer_1, HEX);
     flag_timer1_en = 1;
  }
  else
  {
    flag_timer1_en = 0;
    Serial.println("Disabled Timer 1");
  }
}

// ######################################################################
BLYNK_WRITE(BLYNK_TIMER_2)
{
  unsigned char week_day;
  
  TimeInputParam  t(param);
  
  if (t.hasStartTime() && t.hasStopTime() && t.getStartSecond()==0 && t.getStopSecond()==0 )
  {
     start_time_hour_2 = t.getStartHour();
     start_time_min_2 = t.getStartMinute();
     Serial.println(String("Time2 Start: ") +
                     start_time_hour_2 + ":" +
                     start_time_min_2);
    
     stop_time_hour_2 = t.getStopHour();
     stop_time_min_2 = t.getStopMinute();
     Serial.println(String("Time2 Stop: ") +
                     stop_time_hour_2 + ":" +
                     stop_time_min_2);
    
     for (int i = 1; i <= 7; i++)
     {
       if (t.isWeekdaySelected(i))  // will be "TRUE" if nothing selected as well
       {
         day_timer_2 |= (0x01 << (i-1));
       }
       else
         day_timer_2 &= (~(0x01 << (i-1)));
     }
    
     Serial.print("Time1 Selected Days: ");
     Serial.println(day_timer_2, HEX);
     flag_timer2_en = 1;
  }
  else
  {
    flag_timer2_en = 0;
    Serial.println("Disabled Timer 1");
  }
}


// ######################################################################
void fn_valve_mng (void)
{
  bool time_set_overflow;
  long start_timer_sec;
  long stop_timer_sec;
  bool flag_timer_on_1_buf = flag_timer_on_valve_1;
  bool flag_timer_on_2_buf = flag_timer_on_valve_2;

  // VALVE 1
  time_set_overflow = 0;
  start_timer_sec = start_time_hour_1*3600 + start_time_min_1*60;
  stop_timer_sec = stop_time_hour_1*3600 + stop_time_min_1*60;

  if ( stop_timer_sec < start_timer_sec ) time_set_overflow = 1;
  
  if ( flag_timer1_en && (((time_set_overflow == 0 && (rtc_sec_server >= start_timer_sec) && (rtc_sec_server < stop_timer_sec)) ||
        (time_set_overflow  && ((rtc_sec_server >= start_timer_sec) || (rtc_sec_server < stop_timer_sec)))) && 
        (day_timer_1 == 0x00 || (day_timer_1 & (0x01 << (weekday_server - 1) )))) )
  {
    flag_timer_on_valve_1 = 1;
  }
  else
    flag_timer_on_valve_1 = 0;

  // VALVE 2
  time_set_overflow = 0;
  start_timer_sec = start_time_hour_2*3600 + start_time_min_2*60;
  stop_timer_sec = stop_time_hour_2*3600 + stop_time_min_2*60;

  if ( stop_timer_sec < start_timer_sec ) time_set_overflow = 1;
  
  if ( flag_timer2_en && (((time_set_overflow == 0 && (rtc_sec_server >= start_timer_sec) && (rtc_sec_server < stop_timer_sec)) ||
        (time_set_overflow  && ((rtc_sec_server >= start_timer_sec) || (rtc_sec_server < stop_timer_sec)))) && 
        (day_timer_2 == 0x00 || (day_timer_2 & (0x01 << (weekday_server - 1) )))) )
  {
    flag_timer_on_valve_2 = 1;
  }
  else
    flag_timer_on_valve_2 = 0;


  // VALVE 1
  if ( flag_timer_on_valve_1 )
  {
    flag_valve_1_status = 1;
    flag_valve_1_set = 0;
  }
  else
  {
    flag_valve_1_status = flag_valve_1_set;
  }

  // VALVE 2
  if ( flag_timer_on_valve_2 )
  {
    flag_valve_2_status = 1;
    flag_valve_2_set = 0;
  }
  else
  {
    flag_valve_2_status = flag_valve_2_set;
  }

  if ( flag_timer_on_1_buf != flag_timer_on_valve_1 )
    flag_blynk_valve_1_update = 1;

  if ( flag_timer_on_2_buf != flag_timer_on_valve_2 )
    flag_blynk_valve_2_update = 1;

  // HARDWARE CONTROL
  digitalWrite(Relay1, !flag_valve_1_status);  // Relay active LOW
  digitalWrite(Relay2, !flag_valve_2_status);  // Relay active LOW
}
// ######################################################################
void RTC_mng(void)
{
  int year_server;
  String currentTime = String(hour()) + ":" + minute() + ":" + second();
  String currentDate = String(day()) + " " + month() + " " + year();
  
  weekday_server = weekday();
  
  if ( weekday_server == 1 )
    weekday_server = 7;
  else
    weekday_server -= 1;

  year_server = year();
  
  rtc_sec_server = (hour()*60*60) + (minute()*60) + second();

#if 0
  Serial.print("Current server time: ");
  Serial.print(currentTime);
  Serial.print(" ");
  Serial.print(currentDate);
  Serial.println();
  Serial.print("RTC Server in Seconds : ");
  Serial.println(rtc_sec_server);
  Serial.print("Weekday: ");
  Serial.println(weekday_server);

  Serial.print("Year Server: ");
  Serial.println(year_server);
#endif
}

// ######################################################################
void update_blynk_data(void)
{
  if ( flag_blynk_valve_1_update )
  {
    flag_blynk_valve_1_update = 0;
    Blynk.virtualWrite(BLYNK_VALVE_1, flag_valve_1_status);
  }

  if ( flag_blynk_valve_2_update )
  {
    flag_blynk_valve_2_update = 0;
    Blynk.virtualWrite(BLYNK_VALVE_2, flag_valve_2_status);
  }
}

void sendData(float value) {
  Serial.println("==========");
  Serial.print("connecting to ");
  Serial.println(host);
  
  //----------------------------------------Connect to Google host
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }
  //----------------------------------------

  //----------------------------------------Proses dan kirim data  

  float string_moist = value; 
  String url = "/macros/s/" + GAS_ID + "/exec?flow=" + string_moist; //  2 variables 
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  //----------------------------------------

  //---------------------------------------
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    if (line == "\r") {
      Serial.println("headers received");
      break;
    }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
    Serial.println("esp8266/Arduino CI successfull!");
  } else {
    Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.print("reply was : ");
  Serial.println(line);
  Serial.println("closing connection");
  Serial.println("==========");
  Serial.println();
  //----------------------------------------
}
