#include <Arduino.h>
#include <DHT.h>
#include <Adafruit_Sensor.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFi.h>
#include <SPIFFS.h> 

#define LDRPIN 35
#define LEDPIN 13
#define DHTPIN 17
#define ledTempPin 25 
#define SOILMOISTURE 34     
#define SoilLedpin 16
#define DHTTYPE DHT11

int threshold = 40;
bool isManualMode = false;

DHT dht(DHTPIN, DHTTYPE);
AsyncWebServer server(80);

void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("SPIFFS Mount Failed");
    return;
  }
  Serial.println("SPIFFS mounted successfully");
  
  File root = SPIFFS.open("/");
  File file = root.openNextFile();
  
  Serial.println("SPIFFS Files:");
  while (file) {
    Serial.print("FILE: ");
    Serial.println(file.name());
    file = root.openNextFile();
  }
}

void initServer() {
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });
  // Other routes...
  
  server.on("/sensorData", HTTP_GET, [](AsyncWebServerRequest *request){
    int ldrValue = analogRead(LDRPIN);
    float temperature = dht.readTemperature();
    int soilMoistureValue = analogRead(SOILMOISTURE);
    int moisturePercent = map(soilMoistureValue, 0, 4095, 100, 0);
    
    String json = "{\"light\":" + String(ldrValue) + 
                  ",\"temperature\":" + String(temperature) +
                  ",\"soilMoisture\":" + String(moisturePercent) + "}";
    
    request->send(200, "application/json", json);
  });
  
  server.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/style.css", "text/css");
  });

  server.on("/script.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/script.js", "application/javascript");
  });
  //turn on/off the fan based on route and manual mode status 
  server.on("/ledTemp/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isManualMode) digitalWrite(ledTempPin, HIGH);
    request->send(200, "text/plain", "ledTemp is On");
  });
 
  server.on("/ledTemp/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isManualMode) digitalWrite(ledTempPin, LOW);
    request->send(200, "text/plain", "ledTemp is Off");
  });
   //turn on/off the light strip based on route and manual mode status 
  server.on("/LED/on", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isManualMode) digitalWrite(LEDPIN, HIGH);
    request->send(200, "text/plain", "LED is On");
  });

  server.on("/LED/off", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (isManualMode) digitalWrite(LEDPIN, LOW);
    request->send(200, "text/plain", "LED is Off");
  });
  //Mode control route
  server.on("/mode", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("mode")) {
      String mode = request->getParam("mode")->value();
      isManualMode = (mode == "manual");
      request->send(200, "text/plain", "Mode changed to " + mode);
    } else {
      request->send(400, "text/plain", "Invalid mode parameter");
    }
  });

  server.begin();
}

void setup() {
  Serial.begin(115200);
  initSPIFFS();
  
  const char* ssid = "ESP32_Hotspot";
  const char* password = "12345678";
  IPAddress local_IP(192, 168, 4, 1);
  IPAddress gateway(192, 168, 4, 1);
  IPAddress subnet(255, 255, 255, 0);

  WiFi.softAP(ssid, password);
  WiFi.softAPConfig(local_IP, gateway, subnet);

  Serial.println("Hotspot started");
  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());
  
  dht.begin();
  
  pinMode(LEDPIN, OUTPUT);
  pinMode(ledTempPin, OUTPUT);
  pinMode(SoilLedpin, OUTPUT);
  digitalWrite(LEDPIN, LOW);
  digitalWrite(ledTempPin, LOW);
  digitalWrite(SoilLedpin, LOW);
  
  initServer();
}

void loop() {
  int ldrValue = analogRead(LDRPIN);
  float temperature = dht.readTemperature();
  int soilMoistureValue = analogRead(SOILMOISTURE);
  int moisturePercent = map(soilMoistureValue, 0, 4095, 100, 0);

  // Print sensor values to the Serial Monitor
  Serial.print("Light Value: ");
  Serial.println(ldrValue);
  Serial.print("Temperature: ");
  Serial.println(temperature);
  Serial.print("Soil Moisture level: ");
  Serial.println(soilMoistureValue);
  Serial.print("Soil Moisture: ");
  Serial.print(moisturePercent);
  Serial.println("%");
  Serial.println("---------------------------------------");

  // Control actuators based on automatic/manual mode
  if (!isManualMode) {
    digitalWrite(LEDPIN, ldrValue > 2300 ? HIGH : LOW);
    digitalWrite(ledTempPin, temperature >= 30 ? HIGH : LOW);
    digitalWrite(SoilLedpin, soilMoistureValue > 3000 ? HIGH : LOW);
  }
  
  delay(1000);
}
