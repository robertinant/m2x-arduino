#include "M2XStreamClient.h"

const char* M2XStreamClient::kDefaultM2XHost = "api-m2x.att.com";

M2XStreamClient::M2XStreamClient(Client* client,
                                 const char* key,
                                 const char* host,
                                 int port) : _client(client),
                                             _key(key),
                                             _host(host),
                                             _port(port) {
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          double value) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif
    _client->print("PUT /v1/feeds/");
    _client->print(feedId);
    _client->print("/streams/");
    _client->print(streamName);
    _client->println(" HTTP/1.0");

    _client->print("X-M2X-KEY: ");
    _client->println(_key);
    _client->print("Host: ");
    _client->print(_host);
    if (_port != kDefaultM2XPort) {
      _client->print(":");
      _client->print(_port);
    }
    _client->println();
    _client->println("Content-Type: application/x-www-form-urlencoded");
    _client->println();

    _client->print("value=");
    _client->print(value);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }

  return readStatusCode();
}

int M2XStreamClient::readStatusCode() {
  static const char* kHeaderText = "HTTP/*.* ";
  const int kHeaderLen = 9;
  int headerIndex = 0;
  int responseCode = 0;

  while (true) {
    while (_client->available()) {
      char c = _client->read();
#ifdef DEBUG
      Serial.print(c);
#endif
      if (headerIndex < kHeaderLen) {
        if ((c == kHeaderText[headerIndex]) ||
            (kHeaderText[headerIndex] == '*')) headerIndex++;
      } else {
        // Here we use headerIndex instead of creating yet another variable on
        // the precious memory of Arduino. For the first time we reached here,
        // headerIndex should be 9, considering response code contains 3
        // characters, headerIndex should be 12 when we gathered enough data.
        headerIndex++;
        responseCode = responseCode * 10 + (c - '0');
        if (headerIndex == 12) {
          closeCurrentConnection();
          return responseCode;
        }
      }
    }

    if (!_client->connected()) {
#ifdef DEBUG
      Serial.println("ERROR: The client is disconnected from the server!");
#endif
      closeCurrentConnection();
      return E_DISCONNECTED;
    }

    delay(1000);
  }

  // never reached here
  return E_NOTREACHABLE;
}

void M2XStreamClient::closeCurrentConnection() {
  // Eats up buffered data before closing
  _client->flush();
  _client->stop();
}
