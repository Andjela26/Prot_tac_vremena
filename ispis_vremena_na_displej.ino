#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>
#include <WiFiUDP.h>
#include <LiquidCrystal.h>  // Dodajte biblioteku za LCD displej
//#include "TM1637.h"         //klasa za rad sa drajverom LED displeja
#include <TM1637Display.h>


const int BTN_PIN = 17;  // Pin na kojem je taster za slanje zahteva

const int CLK_PIN = 12;  //clk pin displeja
const int DIO_PIN = 13;  //dio pin displeja

char ssid[] = "AndroidAP514A";  // SSID vaše WiFi mreže
char password[] = "drug1234";   // Lozinka vaše WiFi mreže
unsigned int localPort = 2390;  // Lokalni port (za prijem odgovora)
unsigned int echoPort = 123;    // Odredišni port Echo paketa

const char *ntpServerName = "time.nist.gov";
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];

unsigned long TS;

unsigned char HH;
unsigned char MM;
unsigned char ss;

IPAddress timeServerIP(132, 163, 96, 6);

TM1637Display display(CLK_PIN, DIO_PIN);  //instanciranje objekta klase TM1637
WiFiUDP udp;

int T;
/*
void displayBinaryNumber(unsigned long num) {
  unsigned d;
  for (int i = 3; i >= 0; i--) {
    d = num % 10;
    tm1637.display(i, d);
    num /= 10;
  }
}
*/
void connectToWiFi() {
  Serial.println("Povezivanje na WiFi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Povezivanje...");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Povezan na WiFi");
  } else {
    Serial.println("Neuspešno povezivanje na WiFi");
  }
}

void setup() {
  //tm1637.init();               //inicijalizacija drajvera LED displeja
  //tm1637.set(BRIGHT_TYPICAL);  //podrazumevani nivo osvetljaja
  //tm1637.point(true);
  pinMode(BTN_PIN, INPUT_PULLUP);
  Serial.begin(115200);
  display.setBrightness(0x0f);
  connectToWiFi();
  udp.begin(localPort);
}


void loop() {

  if (digitalRead(BTN_PIN) == LOW) {
    WiFi.hostByName(ntpServerName, timeServerIP);

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("Slanje NTP zahteva...");
      memset(packetBuffer, 0, NTP_PACKET_SIZE);
      packetBuffer[0] = 0x1B;

      //WiFiUDP udp;

      udp.beginPacket(timeServerIP, echoPort);
      udp.write(packetBuffer, NTP_PACKET_SIZE);
      udp.endPacket();

      int packetSize = udp.parsePacket();

      if (packetSize >= NTP_PACKET_SIZE) {
        udp.read(packetBuffer, NTP_PACKET_SIZE);
        unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
        unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
        TS = highWord << 16 | lowWord;
        TS -= 2208988800UL;  // Oduzimanje 70 godina (1900-1970) u sekundama

        TS += 3600;
        int NS = 0;
        NS = TS % 86400;

        //Serial.println(NS);
        HH = NS / 3600;
        HH = (HH + 1) % 24;  //za nasu vremensku zonu
                             //      MM = (NS / 60) - (60 * HH);
        MM = (NS % 3600) / 60;
        ss = NS % 60;

     
        T = HH * 100 + MM;

        Serial.println("HH:MM:SS ");
        Serial.print(HH);
        Serial.print(":");
        Serial.print(MM);
        Serial.print(":");
        Serial.println(ss);
      }
    }
  }
  display.showNumberDecEx(T, 0b11100000, true);

}
