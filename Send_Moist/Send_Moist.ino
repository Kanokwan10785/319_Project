//----------------------------------------Include the NodeMCU ESP8266 Library

#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#define soilMoisture A0

#define BLYNK_TEMPLATE_ID "TMPL6tH3kk0yR"
#define BLYNK_TEMPLATE_NAME "test"
#define BLYNK_AUTH_TOKEN "MtkLG9b6nK8Za7ijMr2vZHSEcIpEi3HS"

#include <BlynkSimpleEsp8266.h>

//----------------------------------------SSID dan Password wifi mu gan.
const char* ssid = "243-iot"; //--> Nama Wifi / SSID.
const char* password = "987654321"; //-->  Password wifi .
//----------------------------------------

//----------------------------------------Host & httpsPort
const char* host = "script.google.com";
const int httpsPort = 443;
//----------------------------------------


WiFiClientSecure client; //--> Create a WiFiClientSecure object.

// Timers auxiliar variables
long this_time = millis();
long lastMeasure = 0;

String GAS_ID = "AKfycby4T9-TMEw0wfZx9PTk-pUGblM-4CP_Qi_x8fQnUtAcI9cHlXpVL54bm1RsjrbgddrY"; //--> spreadsheet script ID

//============================================ void setup
void setup() {
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid,password);
  Serial.begin(9600);
  delay(100);

  WiFi.begin(ssid, password); //--> Connect to your WiFi router
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
//==============================================================================
//============================================================================== void loop
void loop() {
  Blynk.run();
  this_time = millis();
  // Publishes new temperature and humidity every 3 seconds
  int soilMoisture = analogRead(A0);
  int outsoilMoisture = map(soilMoisture,1024,0,0,100);
  Blynk.virtualWrite(V0, outsoilMoisture);
  if (this_time - lastMeasure > 3000) {
    lastMeasure = this_time;
    Serial.print("Moist: ");
    Serial.print(outsoilMoisture);
    sendData(outsoilMoisture);
    
  }

  
}
//*****
//==============================================================================

void sendData(int value) {
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
  String url = "/macros/s/" + GAS_ID + "/exec?moist=" + string_moist; //  2 variables 
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
