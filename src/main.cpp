#include "soc/efuse_reg.h"
#include <App.hpp>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <Esp.h>
#include <DisplayHandler.hpp>
#include <OtaHandler.hpp>
#include <SPIFFS.h>
#include <WiFi.h>
#include <WeatherData.hpp>

byte mac[6];
uint64_t chipid;

volatile SemaphoreHandle_t mutex;

static int counter;
static time_t lastTimestamp = 0;
static time_t lastDisplayUpdateTimestamp = 0;
static const char *rollingChars = "|/-\\";

struct tm timeinfo;
time_t startTime;

int getChipRevision()
{
  return ((REG_READ(EFUSE_BLK0_RDATA3_REG) >> EFUSE_RD_CHIP_VER_REV1_S) &&
          EFUSE_RD_CHIP_VER_REV1_V);
}

void printAsDouble(const char *label, uint32_t value, double divisor,
                   const char *unit)
{
  Serial.print(label);
  double dvalue = (double)value;
  dvalue /= divisor;
  Serial.print(dvalue);
  Serial.println(unit);
}

void secondTask(void *parameter)
{
  xSemaphoreTake(mutex, portMAX_DELAY);
  Serial.printf(PSTR("2nd Task core       : %d\n"), xPortGetCoreID());
  xSemaphoreGive(mutex);
  vTaskDelete(NULL);
}

void connectWiFi()
{
  Serial.print(PSTR("\nConnecting to WiFi network "));

  WiFi.begin();
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(OTA_HOSTNAME);
  WiFi.begin(WIFI_SSID, WIFI_PASS);

  while (WiFi.status() != WL_CONNECTED)
  {
    digitalWrite(BUILTIN_LED, 1 ^ digitalRead(BUILTIN_LED));
    delay(500);
    Serial.print(".");
  }

  Serial.println(PSTR(" connected.\n"));
  digitalWrite(BUILTIN_LED, BOARD_LED_ON);

  WiFi.macAddress(mac);
  Serial.printf(PSTR("WiFi MAC Address    : %02X:%02X:%02X:%02X:%02X:%02X\n"), mac[0],
                mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.printf(PSTR("WiFi Hostname       : %s\n"), WiFi.getHostname());
  Serial.print(PSTR("WiFi IP-Address     : "));
  Serial.println(WiFi.localIP());
  Serial.print(PSTR("WiFi Gateway-IP     : "));
  Serial.println(WiFi.gatewayIP());
  Serial.print(PSTR("WiFi Subnetmask     : "));
  Serial.println(WiFi.subnetMask());
  Serial.print(PSTR("WiFi DNS Server     : "));
  Serial.println(WiFi.dnsIP());
  Serial.println();
}

void setup()
{
  pinMode(BUILTIN_LED, OUTPUT);
  digitalWrite(BUILTIN_LED, BOARD_LED_OFF);
  Serial.begin(115200);
  mutex = xSemaphoreCreateMutex();
  delay(3000); // wait for serial monitor
  Serial.println(PSTR("\n\n\n" APP_NAME " - Version " APP_VERSION
                 " by Dr. Thorsten Ludewig"));
  Serial.println(PSTR("Build date: " __DATE__ " " __TIME__ "\n"));

  if (!SPIFFS.begin(true))
  {
    Serial.println(PSTR("SPIFFS Mount Failed... reboot system"));
    delay(5000);
    ESP.restart();
    delay(10000);
  }

  Serial.printf(PSTR("Chip Revision (ESP) : %d\n"), ESP.getChipRevision());
  Serial.printf(PSTR("Chip Revision (REG) : %d\n"), getChipRevision());
  Serial.println();
  Serial.printf(PSTR("CPU Frequency       : %dMHz\n"), ESP.getCpuFreqMHz());
  Serial.println();
  printAsDouble(PSTR("Flash Chip          : "), ESP.getFlashChipSize(), 1048576,
                "MB");
  printAsDouble(PSTR("Flash Chip Speed    : "), ESP.getFlashChipSpeed(), 1000000,
                "MHz");
  Serial.println();
  printAsDouble(PSTR("Heap Size           : "), ESP.getHeapSize(), 1024, "KB");
  printAsDouble(PSTR("Free Heap           : "), ESP.getFreeHeap(), 1024, "KB");
  printAsDouble(PSTR("Sketch Size         : "), ESP.getSketchSize(), 1024, "KB");
  printAsDouble(PSTR("Free Sketch Space   : "), ESP.getFreeSketchSpace(), 1024, "KB");
  printAsDouble(PSTR("SPIFFS total bytes  : "), SPIFFS.totalBytes(), 1024, "KB");
  printAsDouble(PSTR("SPIFFS used bytes   : "), SPIFFS.usedBytes(), 1024, "KB");
  SPIFFS.end();

  Serial.printf(PSTR("\nCycle Count         : %u\n"), ESP.getCycleCount());
  Serial.println();
  xTaskCreatePinnedToCore(&secondTask, "secondTask", 10000, NULL, 1, NULL, 0);
  delay(50);
  xSemaphoreTake(mutex, portMAX_DELAY);
  Serial.printf(PSTR("Running core        : %d\n"), xPortGetCoreID());
  xSemaphoreGive(mutex);

  Serial.println();
  Serial.printf(PSTR("SDK Version         : %s\n"), ESP.getSdkVersion());
  Serial.printf(PSTR("PIO Environment     : %s\n"), PIOENV);
  Serial.printf(PSTR("PIO Platform        : %s\n"), PIOPLATFORM);
  Serial.printf(PSTR("PIO Framework       : %s\n"), PIOFRAMEWORK);
  Serial.printf(PSTR("Arduino Board       : %s\n"), ARDUINO_BOARD);
  Serial.println();

  WiFi.persistent(false);
  WiFi.disconnect(true);
  delay(200);

  connectWiFi();

  counter = 0;

  configTzTime(TIMEZONE, NTP_SERVER1, NTP_SERVER2, NTP_SERVER3);

  if (getLocalTime(&timeinfo, 10000)) // wait up to 10sec to sync
  {
    Serial.println(&timeinfo, PSTR("Time set            : %A %d %B %Y %H:%M:%S"));
    Serial.print(PSTR("Timezone            : "));
    Serial.println(getenv("TZ"));
  }
  else
  {
    Serial.println(PSTR("Time not set"));
  }

  startTime = time(nullptr);
  getLocalTime(&timeinfo);
  Serial.println(&timeinfo, PSTR("Time                : %Y-%m-%d %H:%M:%S"));
  Serial.println();

  delay(1000);
  displayHandler.setup();
  displayHandler.update();
  lastDisplayUpdateTimestamp = millis();

  InitializeOTA();

  Serial.println("Setup done.\n");
  Serial.flush();
}

void loop()
{
  time_t currentTimestamp = millis();
  getLocalTime( &timeinfo );

  if ((currentTimestamp - lastTimestamp >= 1000))
  {
    bool wifiIsConnected = WiFi.isConnected();
    Serial.printf("\r[%d] Running: ", xPortGetCoreID() );
    Serial.print(rollingChars[counter]);
    Serial.printf(" and WiFi is%s connected  ", wifiIsConnected ? "" : " NOT");
    counter++;
    counter %= 4;
    lastTimestamp = currentTimestamp;

    digitalWrite( BUILTIN_LED, 1 ^ digitalRead(BUILTIN_LED));

    if (wifiIsConnected == false)
    {
      connectWiFi();
    }

    if (( timeinfo.tm_min == 0 || timeinfo.tm_min == 30 ) && 
         currentTimestamp > ( lastDisplayUpdateTimestamp + 120000 ))
    {
      displayHandler.update();
      lastDisplayUpdateTimestamp = millis();
    }
  }

  ArduinoOTA.handle();
  delay( 25 );
}
