#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SD.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <Wire.h>
#include <Adafruit_BME280.h>
#include <NtpClientLib.h>

#define MSECOND  1000
#define MMINUTE  60*MSECOND
#define MHOUR    60*MMINUTE
#define MDAY     24*MHOUR

// attention D0 = GPIO16 = pulldown
#define WRITEPIN D0

// configuration en EEPROM
struct EEconf {
  char ssid[32];
  char password[64];
  char myhostname[32];
};
EEconf readconf;

// mot de passe pour l'OTA
const char* otapass = "123456";
// gestion du temps pour calcul de la durée de la MaJ
unsigned long otamillis;

unsigned long previousMillis = 0;
unsigned long previousMillisSDU = 0;

boolean sdinact = 0;

ESP8266WebServer server(80);
Adafruit_BME280 bme;

void confOTA() {
  // Port 8266 (défaut)
  ArduinoOTA.setPort(8266);

  // Hostname défaut : esp8266-[ChipID]
  ArduinoOTA.setHostname(readconf.myhostname);

  // mot de passe pour OTA
  ArduinoOTA.setPassword(otapass);

  ArduinoOTA.onStart([]() {
    Serial.println("/!\\ Maj OTA");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\n/!\\ MaJ terminee");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progression: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
}

void testSDU() {
  if(digitalRead(WRITEPIN) != sdinact) {
    sdinact=digitalRead(WRITEPIN);
    if(sdinact) {
      server.stop();
      SD.end();
      digitalWrite(LED_BUILTIN, HIGH);
      Serial.println("SD & serveur inactifs");
    } else {
      if (!SD.begin(D8)) {
        Serial.println("Erreur initialization SD !");
      } else {
        Serial.println("Initialization SD ok.");
        server.begin();
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("SD & serveur actifs");
      }
    }
  }  
}

void adddata() {
  if(sdinact) {
    Serial.println("SD désactivée");
    return;
  }
  // seulement si on a une sync NTP
  if(NTP.getLastNTPSync()) {
    char strbuffer[64];
    snprintf(strbuffer, 64, "\"%s\";%.2f;%.2f;%.2f\n",
      NTP.getTimeDateString().c_str(),
      bme.readTemperature(),
      bme.readHumidity(),
      bme.readPressure()/100.0);
    Serial.println(strbuffer);
    File f = SD.open("/data.csv", FILE_WRITE);
    if (!f) {
      Serial.println("erreur ouverture fichier!");
    } else {
      f.print(strbuffer);
      f.close();
    }
  }
}

void getdatacsv() {
  if(SD.exists("/data.csv")) {
    File f = SD.open("/data.csv");
    if(!f)
      return;
    if(server.streamFile(f, "text/plain") != f.size()) {
      Serial.println("Erreur streamFile!");
    }
    f.close();
  }
}

void setup() {
  pinMode(WRITEPIN, INPUT_PULLDOWN_16);
  pinMode(LED_BUILTIN, OUTPUT);
  // led off
  digitalWrite(LED_BUILTIN, HIGH);
  
  // Moniteur série
  Serial.begin(115200);
  Serial.println();

  if(!SPIFFS.begin()) {
    Serial.println("Erreur initialisation SPIFFS");
  }

  if (!SD.begin(D8)) {
    Serial.println("SD initialization failed!");
  } else {
    Serial.println("SD initialization done.");
    digitalWrite(LED_BUILTIN, LOW);
  }

  // Lecture configuration Wifi
  EEPROM.begin(sizeof(readconf));
  EEPROM.get(0, readconf);

  // Connexion au Wifi
  Serial.print(F("Connexion Wifi AP"));
  WiFi.mode(WIFI_STA);
  WiFi.begin(readconf.ssid, readconf.password);
  while(WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(F("\r\nWiFi connecté"));
  Serial.print(F("Mon adresse IP: "));
  Serial.println(WiFi.localIP());

  server.serveStatic("/", SPIFFS, "/index.html");
  server.serveStatic("/index.html", SPIFFS, "/index.html");
  server.serveStatic("/indexminmax.html", SPIFFS, "/indexminmax.html");
  server.serveStatic("/Chart.bundle.min.js", SPIFFS, "/Chart.bundle.min.js");
  server.serveStatic("/mytooltips.js", SPIFFS, "/mytooltips.js");
  server.serveStatic("/papaparse.min.js", SPIFFS, "/papaparse.min.js");
  server.serveStatic("/utils.js", SPIFFS, "/utils.js");
  server.serveStatic("/moment.js", SPIFFS, "/moment.js");
  server.on("/data.csv", HTTP_GET, getdatacsv);
  server.begin();

  Wire.begin(D2, D1);
  if (!bme.begin(0x76)){
    Serial.println(F("Erreur BME280"));
  }

  Serial.print(F("Température: "));
  Serial.println(bme.readTemperature());
  Serial.print(F("Pression: "));
  Serial.println(bme.readPressure()/100.0);
  Serial.print(F("Humidité: "));
  Serial.println(bme.readHumidity());

  // configuration NTP
  NTP.begin("europe.pool.ntp.org", 1, true, 0);

  // configuration OTA
  confOTA();

  // test si SD est utilisable ou inactive
  testSDU();
  adddata();
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillisSDU >= 100) {
    // test si SD est utilisable ou inactive
    testSDU();
    previousMillisSDU = currentMillis;
  }
  
  if (currentMillis - previousMillis >= MMINUTE*30) {
    previousMillis = currentMillis;
    adddata();
  } 
  // gestion OTA
  ArduinoOTA.handle();
  // gestion serveur HTTP
  server.handleClient();
}
