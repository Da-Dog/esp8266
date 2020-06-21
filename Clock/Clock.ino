#include <TimeLib.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiUdp.h>
#include <SPI.h>
#include <U8g2lib.h>

U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

const char *ssid = "ATT9eg4Iys";
const char *password = "a5dvbgn3db5n";
static const char ntpServerName[] = "time1.google.com";
const int timeZone = -7;

unsigned int localPort = 8888;
const int slaveSelect = 5;
const int scanLimit = 7;
String result;
float temp;


WiFiClient client;
WiFiUDP Udp;


void setup()
{
  u8g2.begin();
  u8g2.enableUTF8Print();
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_unifont_t_chinese2);
  u8g2.setCursor(0, 14);
  u8g2.print("Waiting for WiFi");
  u8g2.setCursor(0, 30);
  u8g2.print("connection...");
  u8g2.sendBuffer();
  Serial.println("OLED Ready");
  Serial.begin(9600);
  Serial.println("");
	Serial.print("Connecting...");
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED) {
		delay(1000);
		Serial.print(".");
	}
  Serial.println("");
	Serial.println("WiFi connected.");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
  Udp.begin(localPort);
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
}

time_t prevDisplay = 0;

void loop() {
  if (timeStatus() != timeNotSet)
  {
    if (now() != prevDisplay)
    {
      prevDisplay = now();
      oledClockDisplay();
    }
  }
}

void oledClockDisplay() {
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
  u8g2.print("Current Time");
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
}


// NTP 
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

time_t getNtpTime()
{
    IPAddress ntpServerIP;

    while (Udp.parsePacket() > 0);
    WiFi.hostByName(ntpServerName, ntpServerIP);
    sendNTPpacket(ntpServerIP);
    uint32_t beginWait = millis();
    while (millis() - beginWait < 1500){
        int size = Udp.parsePacket();
        if (size >= NTP_PACKET_SIZE){
            Udp.read(packetBuffer, NTP_PACKET_SIZE);
            unsigned long secsSince1900;
            secsSince1900 = (unsigned long)packetBuffer[40] << 24;
            secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
            secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
            secsSince1900 |= (unsigned long)packetBuffer[43];
            return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
        }
    }
    return 0;
}

void sendNTPpacket(IPAddress &address)
{
    memset(packetBuffer, 0, NTP_PACKET_SIZE);
    packetBuffer[0] = 0b11100011;
    packetBuffer[1] = 0;
    packetBuffer[2] = 6;
    packetBuffer[3] = 0xEC;
    packetBuffer[12] = 49;
    packetBuffer[13] = 0x4E;
    packetBuffer[14] = 49;
    packetBuffer[15] = 52;
    Udp.beginPacket(address, 123);
    Udp.write(packetBuffer, NTP_PACKET_SIZE);
    Udp.endPacket();
}
