#ifndef M2XStreamClient_h
#define M2XStreamClient_h

#include "Arduino.h"
#include "Client.h"

#include "Arduino.h"

#define DEBUG

static const int E_NOCONNECTION = -1;
static const int E_DISCONNECTED = -2;
static const int E_NOTREACHABLE = -3;

class M2XStreamClient {
public:
  static const char* kDefaultM2XHost;
  static const int kDefaultM2XPort = 80;

  M2XStreamClient(Client* client,
                  const char* key,
                  const char* host = kDefaultM2XHost,
                  int port = kDefaultM2XPort);
  int send(const char* feedId, const char* streamName, double value);
private:
  Client* _client;
  const char* _key;
  const char* _host;
  int _port;

  int readStatusCode();
  void closeCurrentConnection();
};

#endif  /* M2XStreamClient_h */
