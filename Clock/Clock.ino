#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/U8X8_PIN_NONE);

// put in wifi SSID
const char ssid[] = "";
// put wifi password here
const char pass[] = "";
static const char ntpServerName[] = "time.google.com";
const int timeZone = -7;

WiFiClient client;
WiFiUDP Udp;
unsigned int localPort = 8888;

int i = 151;
String result;
float temp;

boolean isNTPConnected = false;

void setup()
{
  u8g2.begin();
  u8g2.enableUTF8Print();

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  isNTPConnected = true;
}

void loop() {
  if (timeStatus() != timeNotSet)
  {
    oledClockDisplay();
  }
}

void oledClockDisplay() {
  if (i > 150) {
    if (client.connect("api.openweathermap.org", 80))
    {
      // change this link to the city that you want the temperature to be
      client.println("GET /data/2.5/weather?q=San%20Francisco,US&APPID=9c01fd6f5fb3e80415a3009bab170f4b");
      client.println("Host: api.openweathermap.org");
      client.println("User-Agent: ArduinoWiFi/1.1");
      client.println("Connection: close");
      client.println();
    }
    else
    {
      return;
    }
    while (client.connected() && !client.available())
      delay(1);
    while (client.connected() || client.available())
    {
      char c = client.read();
      result = result + c;
    }
    client.stop();
    result.replace('[', ' ');
    result.replace(']', ' ');
    char jsonArray[result.length() + 1];
    result.toCharArray(jsonArray, sizeof(jsonArray));
    jsonArray[result.length() + 1] = '\0';
    StaticJsonBuffer<1024> json_buf;
    JsonObject &root = json_buf.parseObject(jsonArray);
    if (!root.success())
    {
      return;
    }
    float temp1 = root["main"]["temp"];
    float temp2 = temp1 - 273.15;
    temp = temp2 * 1.8000 + 32.00;
    i = 0;
  }
  int years, months, days, hours, minutes, seconds, weekdays;
  years = year();
  months = month();
  days = day();
  hours = hour();
  minutes = minute();
  seconds = second();
  weekdays = weekday();
  u8g2.clearBuffer();
  u8g2.setCursor(0, 14);
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.print("Temp: ");
  u8g2.print(temp);
  String currentTime = "";
  if (hours < 10)
    currentTime += 0;
  currentTime += hours;
  currentTime += ":";
  if (minutes < 10)
    currentTime += 0;
  currentTime += minutes;
  currentTime += ":";
  if (seconds < 10)
    currentTime += 0;
  currentTime += seconds;
  String currentDay = "";
  currentDay += years;
  currentDay += "/";
  if (months < 10)
    currentDay += 0;
  currentDay += months;
  currentDay += "/";
  if (days < 10)
    currentDay += 0;
  currentDay += days;

  u8g2.setFont(u8g2_font_logisoso24_tr);
  u8g2.setCursor(0, 44);
  u8g2.print(currentTime);
  u8g2.setCursor(0, 61);
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.print(currentDay);
  if (weekdays == 1)
    u8g2.print(" Sun");
  else if (weekdays == 2)
    u8g2.print(" Mon");
  else if (weekdays == 3)
    u8g2.print(" Tues");
  else if (weekdays == 4)
    u8g2.print(" Wed");
  else if (weekdays == 5)
    u8g2.print(" Thur");
  else if (weekdays == 6)
    u8g2.print(" Fri");
  else if (weekdays == 7)
    u8g2.print(" Sat");
  u8g2.sendBuffer();
  delay(500);
  i++;
}

const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];


time_t getNtpTime()
{
  IPAddress ntpServerIP;

  while (Udp.parsePacket() > 0);
  WiFi.hostByName(ntpServerName, ntpServerIP);
  sendNTPpacket(ntpServerIP);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500)
  {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE)
    {
      isNTPConnected = true;
      Udp.read(packetBuffer, NTP_PACKET_SIZE);
      unsigned long secsSince1900;
      secsSince1900 = (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
    }
  }
  isNTPConnected = false;
  return 0;
}


void sendNTPpacket(IPAddress& address)
{
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011; // LI, Version, Mode
  packetBuffer[1] = 0;          // Stratum, or type of clock
  packetBuffer[2] = 6;          // Polling Interval
  packetBuffer[3] = 0xEC;       // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;
  Udp.beginPacket(address, 123);
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
