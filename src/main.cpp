/*********
  Rui Santos & Sara Santos - Random Nerd Tutorials
  Complete instructions at https://RandomNerdTutorials.com/esp32-wi-fi-manager-asyncwebserver/
  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files.
  The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
  This controls neopixel leds ona fish tank, switchiung betwen white and blue leds

*********/
#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include <ESP32Time.h>
#include "LittleFS.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
ESP32Time rtc(3600);
// Search for parameter in HTTP POST request for wifi manager
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";

const char* PARAM_WHITE = "whiteBrightness";
const char* PARAM_BLUE = "blueBrightness";
const char* PARAM_CURRENT_HOUR = "currentHour";
const char* PARAM_CURRENT_MINS = "currentMins";
const char* PARAM_TIME_ON = "onHour";
const char* PARAM_TIME_OFF = "offHour";

const char* hostname = "andyFishTank";
//Variables to save values from HTML form
String ssid;
String pass;
String ip;
String gateway;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";



IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);

// Timer variables

const long wifiinterval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)

int whiteBrightness;
int blueBrightness;
int currentHour = 0;;
int currentMins = 0;
int onHour;
int offHour;

int time_hour;
// Replaces placeholder with LED
const int blueLedpin = 12;
const int whiteLedpin = 13;
unsigned long previousMillis = 0;
const long interval = 5 * 60 * 1000; //5 minute delay in milli-secs



// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Replaces placeholder with stored values
String processor(const String& var){
  //Serial.println(var);
  String tempdata;

  if(var == "whiteBrightness"){
    tempdata = readFile(LittleFS, "/whiteBrightness.txt");
     whiteBrightness = tempdata.toInt();
    return tempdata;
  }
  else if(var == "blueBrightness"){
    tempdata =  readFile(LittleFS, "/blueBrightness.txt");
    blueBrightness = tempdata.toInt();
    return tempdata;
  }
  else if(var == "onHour"){
    tempdata =  readFile(LittleFS, "/onHour.txt");
    onHour = tempdata.toInt();
    return tempdata;
  }
  else if(var == "offHour"){
    tempdata =  readFile(LittleFS, "/offHour.txt");
    offHour = tempdata.toInt();
    return tempdata;
  }
  else if(var == "currentHour"){
    tempdata =  String(currentHour);
    return tempdata;
  }
  else if(var == "currentMins"){
    tempdata =  String(currentMins);
    return tempdata;
  }
  return String();
}

bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());


  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  
 /*
 if (!WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE, INADDR_NONE)){
    Serial.println("STA Failed to configure");
    return false;
  }
  */

  if (!WiFi.setHostname(hostname )){
    Serial.println("failed top set hostname");
    return false;
  }

  WiFi.begin(ssid.c_str(), pass.c_str());
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= wifiinterval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }

  Serial.println(WiFi.localIP());
  Serial.println(WiFi.getHostname());

  return true;
}

void notFound(AsyncWebServerRequest *request) {
  request->send(404, "text/plain", "Not found");
}


void processTime(){
  time_hour = rtc.getHour(true);
  Serial.print(" ");
  Serial.print(rtc.getAmPm());
  Serial.print(" ");
  Serial.print(rtc.getAmPm(true));
  Serial.print(" ");
  Serial.print(onHour);
  Serial.print(" ");
  Serial.print( time_hour);
  Serial.print(" ");
  Serial.println(offHour);
    if ((onHour <= time_hour)  && (time_hour < offHour) ) {
      digitalWrite(whiteLedpin, HIGH);
      digitalWrite(blueLedpin, LOW);
      Serial.println("WHITE");
    }
    else{
     digitalWrite(whiteLedpin, LOW);
    digitalWrite(blueLedpin, HIGH); 
    Serial.println("BLUE ");
    }
}

void setup() {
  // Serial port for debugging purposes
  Serial.begin(115200);

  initLittleFS();

  pinMode(blueLedpin, OUTPUT);
  pinMode(whiteLedpin, OUTPUT);
  
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(ip);
  Serial.println(gateway);

  if(initWiFi()) {
    // Route for root / web pagetime_hour
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(LittleFS, "/index.html", "text/html", false, processor);
    });
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      String inputMessage;
      String inputParam;
      // GET white value on <ESP_IP>/get?input1=<inputMessage>
    if (request->hasParam(PARAM_WHITE)) {
      inputMessage = request->getParam(PARAM_WHITE)->value();
      inputParam = PARAM_WHITE;
      writeFile(LittleFS, "/whiteBrightness.txt", inputMessage.c_str());
    }
    // GET input2 value on <ESP_IP>/get?input2=<inputMessage>
    else if (request->hasParam(PARAM_BLUE)) {
      inputMessage = request->getParam(PARAM_BLUE)->value();
      inputParam = PARAM_BLUE;
      writeFile(LittleFS, "/blueBrightness.txt", inputMessage.c_str());
    }
    // GET input3 value on <ESP_IP>/get?input3=<inputMessage>
    else if (request->hasParam(PARAM_CURRENT_HOUR)) {
      inputMessage = request->getParam(PARAM_CURRENT_HOUR)->value();
      inputParam = PARAM_CURRENT_HOUR;
      currentHour = inputMessage.toInt();
      rtc.setTime(0, currentMins, currentHour, 1, 1, 2024);
    }
    else if (request->hasParam(PARAM_CURRENT_MINS)) {
      inputMessage = request->getParam(PARAM_CURRENT_MINS)->value();
      inputParam = PARAM_CURRENT_MINS;
      currentMins = inputMessage.toInt();
      rtc.setTime(0, currentMins, currentHour, 1, 1, 2024);
    }
    else if (request->hasParam(PARAM_TIME_ON)) {
      inputMessage = request->getParam(PARAM_TIME_ON)->value();
      inputParam = PARAM_TIME_ON;
      writeFile(LittleFS, "/onHour.txt", inputMessage.c_str());
    }
    else if (request->hasParam(PARAM_TIME_OFF)) {
      inputMessage = request->getParam(PARAM_TIME_OFF)->value();
      inputParam = PARAM_TIME_OFF;
      writeFile(LittleFS, "/offHour.txt", inputMessage.c_str());
    }
    else {
      inputMessage = "No message sent";
      inputParam = "none";
    }
    Serial.println(inputMessage);
    Serial.println(inputParam);
    request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") with value: " + inputMessage +
                                     "<br><a href=\"/\">Return to Home Page</a>");
  });
    server.onNotFound(notFound);
    server.begin();
    
    
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("ESP-WIFI-MANAGER", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(LittleFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", LittleFS, "/");
    
    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(LittleFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(LittleFS, passPath, pass.c_str());
          }
          // HTTP POST ip value
          if (p->name() == PARAM_INPUT_3) {
            ip = p->value().c_str();
            Serial.print("IP Address set to: ");
            Serial.println(ip);
            // Write file to save value
            writeFile(LittleFS, ipPath, ip.c_str());
          }
          // HTTP POST gateway value
          if (p->name() == PARAM_INPUT_4) {
            gateway = p->value().c_str();
            Serial.print("Gateway set to: ");
            Serial.println(gateway);
            // Write file to save value
            writeFile(LittleFS, gatewayPath, gateway.c_str());
          }
          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip);
      delay(3000);
      ESP.restart();
    });
    server.begin();
  }
  
  previousMillis = 0;
  processTime();
}

void loop() {
unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval){
    previousMillis = currentMillis;
    processTime();
  }
}