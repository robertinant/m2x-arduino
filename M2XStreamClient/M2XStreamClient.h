#ifndef M2XStreamClient_h
#define M2XStreamClient_h

#define MIN(a, b) (((a) > (b))?(b):(a))

#include "Arduino.h"
#include "Client.h"
#include "NullPrint.h"

static const int E_OK = 0;
static const int E_NOCONNECTION = -1;
static const int E_DISCONNECTED = -2;
static const int E_NOTREACHABLE = -3;
static const int E_INVALID = -4;
static const int E_JSON_INVALID = -5;

typedef void (*stream_value_read_callback)(const char* at,
                                           const char* value,
                                           int index,
                                           void* context);

typedef void (*location_read_callback)(const char* name,
                                       double latitude,
                                       double longitude,
                                       double elevation,
                                       const char* timestamp,
                                       int index,
                                       void* context);

class M2XStreamClient {
public:
  static const char* kDefaultM2XHost;
  static const int kDefaultM2XPort = 80;

  M2XStreamClient(Client* client,
                  const char* key,
                  const char* host = kDefaultM2XHost,
                  int port = kDefaultM2XPort);

  // Update data stream
  int send(const char* feedId, const char* streamName, double value);
  int send(const char* feedId, const char* streamName, long value);
  int send(const char* feedId, const char* streamName, int value);
  int send(const char* feedId, const char* streamName, const char* value);
  int receive(const char* feedId, const char* streamName,
              stream_value_read_callback callback, void* context);

  // Update datasource location
  // NOTE: On an Arduino Uno and other ATMEGA based boards, double has
  // 4-byte (32 bits) precision, which is the same as float. So there's
  // no natural double-precision floating number on these boards. With
  // a float value, we have a precision of roughly 7 digits, that means
  // either 5 or 6 digits after the floating point. According to wikipedia,
  // a difference of 0.00001 will give us ~1.1132m distance. If this
  // precision is good for you, you can use the double-version we provided
  // here. Otherwise, you may need to use the string-version and do the
  // actual conversion by yourselves.
  // However, with an Arduino Due board, double has 8-bytes (64 bits)
  // precision, which means you are free to use the double-version only
  // without any precision problems.
  int updateLocation(const char* feedId, const char* name,
                     double latitude, double longitude, double elevation);
  int updateLocation(const char* feedId, const char* name,
                     const char* latitude, const char* longitude,
                     const char* elevation);
  int readLocation(const char* feedId, location_read_callback callback,
                   void* context);
private:
  Client* _client;
  const char* _key;
  const char* _host;
  int _port;
  NullPrint _null_print;

  void writeSendHeader(const char* feedId,
                       const char* streamName,
                       int contentLength);
  void writeHttpHeader(int contentLength);
  int readContentLength();
  int skipHttpHeader();
  int readStatusCode(bool closeClient);
  int waitForString(const char* str);
  void printEncodedString(const char* str);
  void close();
  int readStreamValue(stream_value_read_callback callback, void* context);
  int readLocation(location_read_callback callback, void* context);
};

#endif  /* M2XStreamClient_h */
