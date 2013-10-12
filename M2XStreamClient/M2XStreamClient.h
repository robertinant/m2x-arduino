#ifndef M2XStreamClient_h
#define M2XStreamClient_h

#include "Arduino.h"
#include "Client.h"

#include "Arduino.h"

static const int E_OK = 0;
static const int E_NOCONNECTION = -1;
static const int E_DISCONNECTED = -2;
static const int E_NOTREACHABLE = -3;
static const int E_INVALID = -4;
static const int E_JSON_INVALID = -5;

typedef void (*stream_value_read_callback)(const char* at, const char* value, int index, void* context);

class M2XStreamClient {
public:
  static const char* kDefaultM2XHost;
  static const int kDefaultM2XPort = 80;

  M2XStreamClient(Client* client,
                  const char* key,
                  const char* host = kDefaultM2XHost,
                  int port = kDefaultM2XPort);
  int send(const char* feedId, const char* streamName, double value);
  int send(const char* feedId, const char* streamName, long value);
  int send(const char* feedId, const char* streamName, int value);
  int send(const char* feedId, const char* streamName, const char* value);
  int receive(const char* feedId, const char* streamName);

  int readStreamValue(stream_value_read_callback callback, void* context);
  void close();
private:
  Client* _client;
  const char* _key;
  const char* _host;
  int _port;

  void writeSendHeader(const char* feedId, const char* streamName);
  int readContentLength();
  int skipHttpHeader();
  int readStatusCode();
  int waitForString(const char* str);
  void printEncodedString(const char* str);
  void putStream(const char*, const char*, String);
};

#endif  /* M2XStreamClient_h */
