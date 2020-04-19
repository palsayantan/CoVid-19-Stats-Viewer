/*
  CoVid-19 Stats Viewer Using ESP32 and 1.8' TFT LED display
  API used in this project  -
  Adhikansh Mittal | for COVID19 India stat API - https://github.com/HrithikMittal/COVID19-India-API
  Coronatab | for COVID19 World stat API - https://github.com/PotentialWeb/CoronaTab/wiki/RESTful-API-documentation
*/

#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiUdp.h>
#include <NTPClient.h>             // include NTPClient library
#include <TimeLib.h>               // include Arduino time library
#include <SPI.h>
#include <TFT_eSPI.h>              // https://github.com/Bodmer/TFT_eSPI
#include <TJpg_Decoder.h>          // https://github.com/Bodmer/TJpg_Decoder
#include "bg.h"                    //Background image
#include <ArduinoJson.h>

const char* ssid = "SSID";
const char* password = "Password";

WiFiUDP ntpUDP;
// 'time.nist.gov' is used (default server) with +1 hour offset (3600 seconds) 60 seconds (60000 milliseconds) update interval
NTPClient timeClient(ntpUDP, "time.nist.gov", 19800, 60000);

TFT_eSPI tft = TFT_eSPI();         // Invoke custom library

bool tft_output(int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t* bitmap)
{
  if ( y >= tft.height() ) return 0;
  tft.pushImage(x, y, w, h, bitmap);
  return 1;
}

//counting days
int world_dd = 30;
int world_mm = 01;
int Ind_dd = 24;
int Ind_mm = 03;
int total_days;

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  tft.begin();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
  TJpgDec.setSwapBytes(true);
  TJpgDec.setCallback(tft_output);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print("Connecting...");
    tft.drawString("Connecting...", 45, 60, 1);
  }
  timeClient.begin();

  TJpgDec.drawJpg(0, 0, bg, sizeof(bg));
  delay(1000);

  tft.setTextColor(0xFFE0);
  tft.drawString("CoVid-19 ", 28, 10, 4);
  tft.drawString("Statistics", 28, 35, 4);
  delay(500);
  tft.setTextColor(0x7FF);
  tft.drawString("Stay Home", 20, 75, 4);
  delay(1000);
  tft.drawString("Stay Safe", 30, 100, 4);
  delay(2000);

  TJpgDec.drawJpg(0, 0, bg, sizeof(bg));
  tft.setTextColor(0xF81F, 0x0000);
  tft.drawString("CoVid-19 : ", 25, 2, 2);
  tft.setTextColor(0x7FF, 0x7BEF);
  tft.drawString("Crisis", 100, 24, 1);
  tft.drawString("Days: ", 100, 33, 1);
  tft.setTextColor(0xF800, 0x0000);
  tft.drawString("ACTIVE   :", 5, 47, 2);
  tft.setTextColor(0x7E0, 0x0000);
  tft.drawString("RECOVER :", 5, 67, 2);
  tft.setTextColor(0xFFFF, 0x0000);
  tft.drawString("DEATH   :", 5, 87, 2);
  tft.setTextColor(0x1F, 0x0000);
  tft.drawString("TOTAL   :", 5, 107, 2);
}

char Time[] = "  :  :  ";
char Date[] = "  -  -20  ";
byte last_second, last_minute, second_, minute_, hour_, wday, day_, month_, year_;

void loop()
{
  date();
  world_stat();
  IN_stat();
  WB_stat();
}

void resetStat() {
  tft.fillRect(80, 47, 60, 16, 0x0000);
  tft.fillRect(80, 67, 60, 16, 0x0000);
  tft.fillRect(80, 87, 60, 16, 0x0000);
  tft.fillRect(80, 107, 60, 16, 0x0000);
}

void date() {
  if (WiFi.status() == WL_CONNECTED)  // check WiFi connection status
  {
    timeClient.update();
    unsigned long unix_epoch = timeClient.getEpochTime();   // get UNIX Epoch time

    second_ = second(unix_epoch);        // get seconds from the UNIX Epoch time
    minute_ = minute(unix_epoch);      // get minutes (0 - 59)
    if (last_second != second_)          // update time & date every 1 min
    {
      hour_   = hour(unix_epoch);        // get hours   (0 - 23)
      wday    = weekday(unix_epoch);     // get minutes (1 - 7 with Sunday is day 1)
      day_    = day(unix_epoch);         // get month day (1 - 31, depends on month)
      month_  = month(unix_epoch);       // get month (1 - 12 with Jan is month 1)
      year_   = year(unix_epoch) - 2000; // get year with 4 digits - 2000 results 2 digits year (ex: 2018 --> 18)

      Time[7] = second_ % 10 + '0';
      Time[6] = second_ / 10 + '0';
      Time[4] = minute_ % 10 + '0';
      Time[3] = minute_ / 10 + '0';
      Time[1] = hour_   % 10 + '0';
      Time[0] = hour_   / 10 + '0';
      Date[9] = year_   % 10 + '0';
      Date[8] = year_   / 10 + '0';
      Date[4] = month_  % 10 + '0';
      Date[3] = month_  / 10 + '0';
      Date[1] = day_    % 10 + '0';
      Date[0] = day_    / 10 + '0';


      tft.setTextColor(0xFFE0);
      tft.setCursor(5, 20, 1);
      tft.println(Date);
      tft.fillRect(5, 29, 64, 8, 0x7BEF);
      tft.setCursor(5, 29, 1);
      tft.println(Time);
      tft.setCursor(5, 38, 1);
      display_wday();
      last_second = second_;
    }
  }
}

void display_wday() {
  switch (wday) {
    case 1:  tft.println("SUNDAY    "); break;
    case 2:  tft.println("MONDAY    "); break;
    case 3:  tft.println("TUESDAY   "); break;
    case 4:  tft.println("WEDNESDAY "); break;
    case 5:  tft.println("THURSDAY  "); break;
    case 6:  tft.println("FRIDAY    "); break;
    default: tft.println("SATURDAY  ");
  }
}

void world_stat() {
  //world
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("https://api.coronatab.app/places/earth");
    int httpCode = http.GET();
    if (httpCode > 0) {
      const size_t capacity = JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(4) + JSON_OBJECT_SIZE(5) + 120;
      DynamicJsonDocument doc(capacity);

      String json = http.getString();

      deserializeJson(doc, json);

      JsonObject data = doc["data"];
      JsonObject data_latestData = data["latestData"];
      const char* data_latestData_date = data_latestData["date"]; // "2020-04-14"
      long data_latestData_cases = data_latestData["cases"]; // 1909366
      long data_latestData_deaths = data_latestData["deaths"]; // 123182
      long data_latestData_recovered = data_latestData["recovered"]; // 424105

      tft.fillCircle(150, 49, 3, 0xFFFF);
      tft.fillCircle(150, 64, 2, 0x0000);
      tft.fillCircle(150, 79, 2, 0x0000);
      tft.fillRect(95, 2, 42, 16, 0x0000);
      tft.setTextColor(0xF800, 0x0000);
      tft.setCursor(95, 2, 2);
      tft.println("WORLD");

      tft.fillRect(128, 33, 16, 8, 0x7BEF);
      tft.setTextColor(0x7FF);
      tft.setCursor(132, 33, 1);
      if (((month_ - world_mm - 1) * 30 + (31 - world_dd) + day_) > 0) {
        total_days = (month_ - world_mm - 1) * 30 + (31 - world_dd) + day_;
        tft.println(total_days);
      }

      resetStat();
      tft.setTextColor(0xF800, 0x0000);
      tft.setCursor(80, 47, 2);
      tft.println(data_latestData_cases);
      tft.setTextColor(0x7E0, 0x0000);
      tft.setCursor(80, 67, 2);
      tft.println(data_latestData_recovered);
      tft.setTextColor(0xFFFF, 0x0000);
      tft.setCursor(80, 87, 2);
      tft.println(data_latestData_deaths);
      tft.setTextColor(0x1F, 0x0000);
      tft.setCursor(80, 107, 2);
      tft.println(data_latestData_cases + data_latestData_recovered + data_latestData_deaths);

      delay(3000);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      world_stat();
    }
    http.end();
    delay(500);
  }
}
void IN_stat() {
  //India
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://covid19-india-adhikansh.herokuapp.com/summary");
    int httpCode = http.GET();
    if (httpCode > 0) {
      const size_t capacity = JSON_OBJECT_SIZE(4) + 80;
      DynamicJsonDocument doc(capacity);

      String json = http.getString();

      deserializeJson(doc, json);

      int Total_Confirmed_cases = doc["Total Confirmed cases"];
      int Cured_Discharged_Migrated = doc["Cured/Discharged/Migrated"];
      int Death = doc["Death"];
      int Total_Cases = doc["Total Cases"];

      tft.fillCircle(150, 49, 2, 0x0000);
      tft.fillCircle(150, 64, 3, 0xFFFF);
      tft.fillCircle(150, 79, 2, 0x0000);
      tft.fillRect(95, 2, 42, 16, 0x0000);
      tft.setTextColor(0xFBE0, 0x0000);
      tft.setCursor(95, 2, 2);
      tft.println("IN");
      tft.setTextColor(0xFFFF, 0x0000);
      tft.setCursor(108, 2, 2);
      tft.println("D");
      tft.setTextColor(0x7E0, 0x0000);
      tft.setCursor(117, 2, 2);
      tft.println("IA");

      tft.fillRect(128, 33, 16, 8, 0x7BEF);
      tft.setTextColor(0x7FF);
      tft.setCursor(132, 33, 1);
      tft.println((month_ - Ind_mm - 1) * 30 + (31 - Ind_dd) + day_);

      resetStat();
      tft.setTextColor(0xF800, 0x0000);
      tft.setCursor(80, 47, 2);
      tft.println(Total_Confirmed_cases);
      tft.setTextColor(0x7E0, 0x0000);
      tft.setCursor(80, 67, 2);
      tft.println(Cured_Discharged_Migrated);
      tft.setTextColor(0xFFFF, 0x0000);
      tft.setCursor(80, 87, 2);
      tft.println(Death);
      tft.setTextColor(0x1F, 0x0000);
      tft.setCursor(80, 107, 2);
      tft.println(Total_Cases);

      delay(3000);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      IN_stat();
    }
    http.end();
    delay(500);
  }
}

void WB_stat() {
  //WB
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin("http://covid19-india-adhikansh.herokuapp.com/state/West%20Bengal");
    int httpCode = http.GET();
    if (httpCode > 0) {
      const size_t capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + JSON_OBJECT_SIZE(6) + 90;
      DynamicJsonDocument doc(capacity);

      String json = http.getString();

      deserializeJson(doc, json);

      JsonObject data_0 = doc["data"][0];
      const char* Sname = data_0["name"]; // "West Bengal"
      int confirmed = data_0["confirmed"]; // 152
      int cured = data_0["cured"]; // 29
      int death = data_0["death"]; // 7
      int total = data_0["total"]; // 188

      tft.fillCircle(150, 49, 2, 0x0000);
      tft.fillCircle(150, 64, 2, 0x0000);
      tft.fillCircle(150, 79, 3, 0xFFFF);
      tft.fillRect(95, 2, 42, 16, 0x0000);
      tft.setTextColor(0x1F, 0x0000);
      tft.setCursor(96, 2, 1);
      tft.println("WEST");
      tft.setTextColor(0xFFFF, 0x0000);
      tft.setCursor(96, 10, 1);
      tft.println("BENGAL");

      tft.setTextColor(0x7FF);
      tft.setCursor(132, 33, 1);
      tft.println((month_ - Ind_mm - 1) * 30 + (31 - Ind_dd) + day_);

      resetStat();
      tft.setTextColor(0xF800, 0x0000);
      tft.setCursor(80, 47, 2);
      tft.println(confirmed);
      tft.setTextColor(0x7E0, 0x0000);
      tft.setCursor(80, 67, 2);
      tft.println(cured);
      tft.setTextColor(0xFFFF, 0x0000);
      tft.setCursor(80, 87, 2);
      tft.println(death);
      tft.setTextColor(0x1F, 0x0000);
      tft.setCursor(80, 107, 2);
      tft.println(total);

      delay(3000);
    }
    else {
      Serial.println(httpCode);
      delay(1000);
      WB_stat();
    }
    http.end();
    delay(500);
  }
}
