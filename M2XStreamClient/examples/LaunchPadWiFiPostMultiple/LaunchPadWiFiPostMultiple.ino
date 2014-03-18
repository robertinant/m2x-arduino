#include <jsonlite.h>
#include <SPI.h>
#include <WiFi.h>

#include "M2XStreamClient.h"

char ssid[] = "<ssid>"; //  your network SSID (name)
char pass[] = "<WPA password>";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;

char feedId[] = "<feed id>"; // Feed you want to post to
char m2xKey[] = "<M2X access key>"; // Your M2X access key

const char *streamNames[] = { "temperature", "humidity" };
int counts[] = { 2, 1 };
const char *ats[] = { "2013-09-09T19:15:00Z", NULL, NULL };
double values[] = { 10.0, 20.0, 7.5 };

WiFiClient client;
M2XStreamClient m2xClient(&client, m2xKey);

void setup() {
  Serial.begin(9600);

  // Set communication pins for CC3000
  WiFi.setCSpin(18);  // 18: P2_2 @ F5529, PE_0 @ LM4F/TM4C
  WiFi.setENpin(2);   //  2: P6_5 @ F5529, PB_5 @ LM4F/TM4C
  WiFi.setIRQpin(19); // 19: P2_0 @ F5529, PB_2 @ LM4F/TM4C

  while ( status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();
}

void loop() {
  int response = m2xClient.postMultiple(feedId, 2, streamNames,
                                        counts, ats, values);
  Serial.print("M2x client response code: ");
  Serial.println(response);

  if (response == -1) while(1) ;

  delay(5000);
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
}

float getVoltage(int pin)
{
  return (analogRead(pin) * 0.004882814);
}
