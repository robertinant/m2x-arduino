#include <SPI.h>
#include <Ethernet.h>

#include "M2XStreamClient.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
// Set the static IP address to use if the DHCP fails to assign
IPAddress ip(192,168,1,17);

char server[] = "192.168.1.4";    // M2X API server
int port = 9393;

char feedId[] = "37a1813b03d46e672b9040de6abf0f73"; // Feed you want to post to
char streamName[] = "temperature"; // Stream you want to post to
char m2xKey[] = "1e56f68dfd3fc3ad4f129facb6b831b4"; // Your M2X access key

EthernetClient client;
M2XStreamClient m2xClient(&client, m2xKey, server, port);

void on_data_point_found(const char* at, const char* value, int index, void* context) {
  Serial.print("Found a data point, index:");
  Serial.println(index);
  Serial.print("At:");
  Serial.println(at);
  Serial.print("Value:");
  Serial.println(value);
}

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection:
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip);
  }

  // wait 10 seconds for connection:
  delay(10000);

  Serial.println("Connected to Ethernet");
  printEthernetStatus();
}

void loop() {
  int response = m2xClient.receive(feedId, streamName);
  Serial.print("M2x client response code: ");
  Serial.println(response);

  int val = m2xClient.readStreamValue(on_data_point_found, NULL);
  Serial.print("M2x stream read returns: ");
  Serial.println(val);

  m2xClient.close();

  if (response == -1) while(1) ;

  delay(5000);
}

void printEthernetStatus() {
  // print local IP
  Serial.print("Local IP: ");
  Serial.println(Ethernet.localIP());

  // print subnet mask
  Serial.print("Subnet Mask: ");
  Serial.println(Ethernet.subnetMask());

  // print gateway IP
  Serial.print("Gateway IP: ");
  Serial.println(Ethernet.gatewayIP());

  // print dns IP
  Serial.print("DNS IP: ");
  Serial.println(Ethernet.dnsServerIP());
}
