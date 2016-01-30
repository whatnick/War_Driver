/*
 This example prints the Wifi MAC address, and
 scans for available Wifi networks.
 Every ten seconds, it scans again. It doesn't actually
 connect to any network, so no encryption scheme is specified.
 
 Circuit:
 * CC3200 WiFi LaunchPad or CC3100 WiFi BoosterPack
 with TM4C or MSP430 LaunchPad
 
 created 13 July 2010
 by dlf (Metodo2 srl)
 modified 21 Junn 2012
 by Tom Igoe and Jaymes Dec
 */
#include "SPI.h" 
#include "pfatfs.h"

#define cs_pin  12

unsigned short int bw=0;
char buffer[512];
int rc;
uint16_t block_size=64;
uint32_t AccStringLength = 0;

#include <WiFi.h>
// For scans that happen every 10 seconds
unsigned long last = 0UL;

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define OLED_RESET 4
Adafruit_SSD1306 display(OLED_RESET);

#include <TinyGPS++.h>
TinyGPSPlus gps;

void die (int pff_err) 		/* Stop with dying message */
/* FatFs return value */
{
  Serial.println();
  Serial.print("Failed with rc=");
  Serial.print(pff_err,DEC);
  for (;;) ;
}

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);

  WiFi.init();
  Serial.println(WiFi.firmwareVersion());

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(2000);

  // Clear the buffer.
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  delay(2000);
  // Print WiFi MAC address:
  printMacAddress();

  // scan for existing networks:
  Serial.println("Scanning available networks...");
  listNetworks();

  // initialize FatFS library (cspin, divider, module)
  FatFs.begin(cs_pin,2);       
}

void loop() {
  //Eat GPS characters
  while (Serial.available() > 0)
  {
    gps.encode(Serial.read());
  }
  //every 10s scan
  if (millis() - last > 10000)
  {
    display.clearDisplay();
    // scan for existing networks:
    Serial.println("Scanning available networks...");
    listNetworks();
    printDetailGPS();
    summaryGPS();
    LogScan();
    display.display();
    last = millis();   
  }
}

void LogScan()
{
  rc = FatFs.open("LOG.txt");//, FA_READ | FA_WRITE);
  if (rc) die(rc);
  rc = FatFs.lseek(  AccStringLength );
  if (rc) die(rc);
  AccStringLength =  AccStringLength + 512;
  rc = FatFs.write(buffer, block_size, &bw);
  if (rc) die(rc);
  rc = FatFs.write(0, 0, &bw);  //Finalize write
  if (rc) die(rc);
  rc = FatFs.close();  //Close file
  if (rc) die(rc);
}

void summaryGPS()
{
  display.setCursor(64,0);
  display.print("Num Sat:");
  display.print(gps.satellites.value());
  if (gps.satellites.isUpdated())
  {
    Serial.print(F("SATELLITES Fix Age="));
    Serial.print(gps.satellites.age());
    Serial.print(F("ms Value="));
    Serial.println(gps.satellites.value());
  }
}
void printDetailGPS()
{
  if (gps.location.isUpdated())
  {
    Serial.print(F("LOCATION   Fix Age="));
    Serial.print(gps.location.age());
    Serial.print(F("ms Raw Lat="));
    Serial.print(gps.location.rawLat().negative ? "-" : "+");
    Serial.print(gps.location.rawLat().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLat().billionths);
    Serial.print(F(" billionths],  Raw Long="));
    Serial.print(gps.location.rawLng().negative ? "-" : "+");
    Serial.print(gps.location.rawLng().deg);
    Serial.print("[+");
    Serial.print(gps.location.rawLng().billionths);
    Serial.print(F(" billionths],  Lat="));
    Serial.print(gps.location.lat(), 6);
    Serial.print(F(" Long="));
    Serial.println(gps.location.lng(), 6);
  }

  else if (gps.date.isUpdated())
  {
    Serial.print(F("DATE       Fix Age="));
    Serial.print(gps.date.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.date.value());
    Serial.print(F(" Year="));
    Serial.print(gps.date.year());
    Serial.print(F(" Month="));
    Serial.print(gps.date.month());
    Serial.print(F(" Day="));
    Serial.println(gps.date.day());
  }

  else if (gps.time.isUpdated())
  {
    Serial.print(F("TIME       Fix Age="));
    Serial.print(gps.time.age());
    Serial.print(F("ms Raw="));
    Serial.print(gps.time.value());
    Serial.print(F(" Hour="));
    Serial.print(gps.time.hour());
    Serial.print(F(" Minute="));
    Serial.print(gps.time.minute());
    Serial.print(F(" Second="));
    Serial.print(gps.time.second());
    Serial.print(F(" Hundredths="));
    Serial.println(gps.time.centisecond());
  }
}

void printMacAddress() {
  // the MAC address of your Wifi
  byte mac[6];

  // print your MAC address:
  WiFi.macAddress(mac);
  Serial.print("MAC: ");
  Serial.print(mac[5], HEX);
  Serial.print(":");
  Serial.print(mac[4], HEX);
  Serial.print(":");
  Serial.print(mac[3], HEX);
  Serial.print(":");
  Serial.print(mac[2], HEX);
  Serial.print(":");
  Serial.print(mac[1], HEX);
  Serial.print(":");
  Serial.println(mac[0], HEX);
}

void listNetworks() {
  // scan for nearby networks:
  Serial.println("** Scan Networks **");
  int numSsid = WiFi.scanNetworks();
  if (numSsid == -1)
  {
    Serial.println("Couldn't get a wifi connection");
    while (true);
  }

  // print the list of networks seen:
  Serial.print("number of available networks:");
  Serial.println(numSsid);
  display.setCursor(0,0);
  display.print("Num Net:");
  display.print(numSsid);

  // print the network number and name for each network found:
  int cur_off = 0;
  for (int thisNet = 0; thisNet < numSsid; thisNet++) {
    Serial.print(thisNet);
    Serial.print(") ");
    Serial.print(WiFi.SSID(thisNet));
    Serial.print("\tSignal: ");
    Serial.print(WiFi.RSSI(thisNet));
    Serial.print(" dBm");
    Serial.print("\tEncryption: ");
    printEncryptionType(WiFi.encryptionType(thisNet));

    //Do some warping since we have only 128x64 space
    display.setCursor(64*(thisNet/5),10+10*(thisNet%5));
    display.print(String(WiFi.SSID(thisNet)).substring(0,6));
    display.print(":");
    display.print(WiFi.RSSI(thisNet));

    //Write data to buffer for logging to SD card
    int n = sprintf(buffer+cur_off,"%s,%d,%d\n",WiFi.SSID(thisNet),WiFi.RSSI(thisNet),WiFi.encryptionType(thisNet));
    cur_off += n;
  }
}

void printEncryptionType(int thisType) {
  // read the encryption type and print out the name:
  switch (thisType) {
  case ENC_TYPE_WEP:
    Serial.println("WEP");
    break;
  case ENC_TYPE_TKIP:
    Serial.println("WPA");
    break;
  case ENC_TYPE_CCMP:
    Serial.println("WPA2");
    break;
  case ENC_TYPE_NONE:
    Serial.println("None");
    break;
  case ENC_TYPE_AUTO:
    Serial.println("Auto");
    break;
  }
}






