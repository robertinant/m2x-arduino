#include "M2XStreamClient.h"

#define HEX(t_) (((t_) > 9) ? ((t_) - 10 + 'A') : ((t_) + '0'))

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
    printEncodedString(feedId);
    _client->print("/streams/");
    printEncodedString(streamName);
    _client->println(" HTTP/1.0");

    _client->print("X-M2X-KEY: ");
    _client->println(_key);
    _client->print("Host: ");
    printEncodedString(_host);
    if (_port != kDefaultM2XPort) {
      _client->print(":");
      // port is an integer, does not need encoding
      _client->print(_port);
    }
    _client->println();
    _client->println("Content-Type: application/x-www-form-urlencoded");
    _client->println();

    _client->print("value=");
    // value is a double, does not need encoding, either
    _client->print(value);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }

  return readStatusCode();
}

int M2XStreamClient::waitForString(const char* str) {
  int currentIndex = 0;
  if (str[currentIndex] == '\0') return E_OK;

  while (true) {
    while (_client->available()) {
      char c = _client->read();
#ifdef DEBUG
      Serial.print(c);
#endif

      if ((str[currentIndex] == '*') ||
          (c == str[currentIndex])) {
        currentIndex++;
        if (str[currentIndex] == '\0') {
          return E_OK;
        }
      } else {
        // start from the beginning
        currentIndex = 0;
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

int M2XStreamClient::readStatusCode() {
  int responseCode = 0;
  int ret = waitForString("HTTP/*.* ");
  if (ret != E_OK) {
    return ret;
  }

  // ret is not needed from here(since it must be E_OK), so we can use it
  // as a regular variable now.
  ret = 0;
  while (true) {
    while (_client->available()) {
      char c = _client->read();
#ifdef DEBUG
      Serial.print(c);
#endif
      responseCode = responseCode * 10 + (c - '0');
      ret++;
      if (ret == 3) {
        return responseCode;
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

int M2XStreamClient::skipHttpHeader() {
  return waitForString("\r\n\r\n");
}

void M2XStreamClient::closeCurrentConnection() {
  // Eats up buffered data before closing
  _client->flush();
  _client->stop();
}


void M2XStreamClient::printEncodedString(const char* str) {
  for (int i = 0; str[i] != 0; i++) {
    if (((str[i] >= 'A') && (str[i] <= 'Z')) ||
        ((str[i] >= 'a') && (str[i] <= 'z')) ||
        ((str[i] >= '0') && (str[i] <= '9')) ||
        (str[i] == '-') || (str[i] == '_') ||
        (str[i] == '.') || (str[i] == '~')) {
      _client->print(str[i]);
    } else {
      // Encode all other characters
      _client->print('%');
      _client->print(HEX(str[i] / 16));
      _client->print(HEX(str[i] % 16));
    }
  }
}
