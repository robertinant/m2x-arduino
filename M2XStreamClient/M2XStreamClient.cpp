#include "M2XStreamClient.h"

#include "utility/jsonlite_parser.h"

#define HEX(t_) ((char) (((t_) > 9) ? ((t_) - 10 + 'A') : ((t_) + '0')))

const char* M2XStreamClient::kDefaultM2XHost = "api-m2x.att.com";
const char* kUserAgentLine = "User-Agent: M2X Arduino Client/0.1";

#define WAITING_AT 0x1
#define GOT_AT 0x2
#define WAITING_VALUE 0x4
#define GOT_VALUE 0x8

#define TEST_GOT_ALL(state_) (((state_) & (GOT_AT | GOT_VALUE)) == \
                              (GOT_AT | GOT_VALUE))

#define TEST_IS_AT(state_) (((state_) & (WAITING_AT | GOT_AT)) == WAITING_AT)
#define TEST_IS_VALUE(state_) (((state_) & (WAITING_VALUE | GOT_VALUE)) == \
                               WAITING_VALUE)

#define AT_BUF_LEN 20
#define VALUE_BUF_LEN 20 // enlarge this if you need more chars

#define MAX_DOUBLE_DIGITS 7

typedef struct {
  uint8_t state;
  char at_str[AT_BUF_LEN + 1];
  char value_str[VALUE_BUF_LEN + 1];
  int index;

  stream_value_read_callback callback;
  void* context;
} json_parsing_context_state;

static int print_encoded_string(Print* print, const char* str);

M2XStreamClient::M2XStreamClient(Client* client,
                                 const char* key,
                                 const char* host,
                                 int port) : _client(client),
                                             _key(key),
                                             _host(host),
                                             _port(port),
                                             _null_print() {
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          double value) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);

    _client->print("value=");
    // value is a double, does not need encoding
    _client->print(value);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          long value) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);

    _client->print("value=");
    // value is a long, does not need encoding
    _client->print(value);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          int value) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);

    _client->print("value=");
    // value is an int, does not need encoding
    _client->print(value);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::send(const char* feedId,
                          const char* streamName,
                          const char* value) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif
    writeSendHeader(feedId, streamName,
                    // 6 for "value="
                    _null_print.print(value) + 6);

    _client->print("value=");
    print_encoded_string(_client, value);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }

  return readStatusCode(true);
}

int M2XStreamClient::receive(const char* feedId, const char* streamName,
                             stream_value_read_callback callback, void* context) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif
    _client->print("GET /v1/feeds/");
    print_encoded_string(_client, feedId);
    _client->print("/streams/");
    print_encoded_string(_client, streamName);
    _client->println("/values HTTP/1.0");

    writeHttpHeader(-1);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }
  int status = readStatusCode(false);
  if ((status >= 200) && (status <= 299)) {
    readStreamValue(callback, context);
  }

  close();
  return status;
}

static int print_encoded_string(Print* print, const char* str) {
  int bytes = 0;
  for (int i = 0; str[i] != 0; i++) {
    if (((str[i] >= 'A') && (str[i] <= 'Z')) ||
        ((str[i] >= 'a') && (str[i] <= 'z')) ||
        ((str[i] >= '0') && (str[i] <= '9')) ||
        (str[i] == '-') || (str[i] == '_') ||
        (str[i] == '.') || (str[i] == '~')) {
      bytes += print->print(str[i]);
    } else {
      // Encode all other characters
      bytes += print->print('%');
      bytes += print->print(HEX(str[i] / 16));
      bytes += print->print(HEX(str[i] % 16));
    }
  }
  return bytes;
}

static int write_location_data(Print* print, const char* name,
                               double latitude, double longitude,
                               double elevation) {
  int bytes = 0;
  bytes += print->print("name=");
  bytes += print_encoded_string(print, name);
  bytes += print->print("&latitude=");
  bytes += print->print(latitude, MAX_DOUBLE_DIGITS);
  bytes += print->print("&longitude=");
  bytes += print->print(longitude, MAX_DOUBLE_DIGITS);
  bytes += print->print("&elevation=");
  bytes += print->print(elevation);
  return bytes;
}

static int write_location_data(Print* print, const char* name,
                               const char* latitude, const char* longitude,
                               const char* elevation) {
  int bytes = 0;
  bytes += print->print("name=");
  bytes += print_encoded_string(print, name);
  bytes += print->print("&latitude=");
  bytes += print_encoded_string(print, latitude);
  bytes += print->print("&longitude=");
  bytes += print_encoded_string(print, longitude);
  bytes += print->print("&elevation=");
  bytes += print_encoded_string(print, elevation);
  return bytes;
}

int M2XStreamClient::updateLocation(const char* feedId,
                                    const char* name,
                                    double latitude,
                                    double longitude,
                                    double elevation) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif

    int length = write_location_data(&_null_print, name, latitude, longitude,
                                     elevation);
    _client->print("PUT /v1/feeds/");
    print_encoded_string(_client, feedId);
    _client->println("/location HTTP/1.0");

    writeHttpHeader(length);
    write_location_data(_client, name, latitude, longitude, elevation);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

int M2XStreamClient::updateLocation(const char* feedId,
                                    const char* name,
                                    const char* latitude,
                                    const char* longitude,
                                    const char* elevation) {
  if (_client->connect(_host, _port)) {
#ifdef DEBUG
    Serial.println("Connected to M2X server!");
#endif

    int length = write_location_data(&_null_print, name, latitude, longitude,
                                     elevation);
    _client->print("PUT /v1/feeds/");
    print_encoded_string(_client, feedId);
    _client->println("/location HTTP/1.0");

    writeHttpHeader(length);
    write_location_data(_client, name, latitude, longitude, elevation);
  } else {
#ifdef DEBUG
    Serial.println("ERROR: Cannot connect to M2X server!");
#endif
    return E_NOCONNECTION;
  }
  return readStatusCode(true);
}

void M2XStreamClient::writeSendHeader(const char* feedId,
                                      const char* streamName,
                                      int contentLength) {
  _client->print("PUT /v1/feeds/");
  print_encoded_string(_client, feedId);
  _client->print("/streams/");
  print_encoded_string(_client, streamName);
  _client->println(" HTTP/1.0");

  writeHttpHeader(contentLength);
}

void M2XStreamClient::writeHttpHeader(int contentLength) {
  _client->println(kUserAgentLine);
  _client->print("X-M2X-KEY: ");
  _client->println(_key);

  _client->print("Host: ");
  print_encoded_string(_client, _host);
  if (_port != kDefaultM2XPort) {
    _client->print(":");
    // port is an integer, does not need encoding
    _client->print(_port);
  }
  _client->println();

  if (contentLength > 0) {
    _client->println("Content-Type: application/x-www-form-urlencoded");
#ifdef DEBUG
    Serial.print("Content Length: ");
    Serial.println(contentLength);
#endif
    _client->print("Content-Length: ");
    _client->println(contentLength);
  }
  _client->println();
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
      close();
      return E_DISCONNECTED;
    }

    delay(1000);
  }
  // never reached here
  return E_NOTREACHABLE;
}

int M2XStreamClient::readStatusCode(bool closeClient) {
  int responseCode = 0;
  int ret = waitForString("HTTP/*.* ");
  if (ret != E_OK) {
    if (closeClient) close();
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
        if (closeClient) close();
        return responseCode;
      }
    }

    if (!_client->connected()) {
#ifdef DEBUG
      Serial.println("ERROR: The client is disconnected from the server!");
#endif
      if (closeClient) close();
      return E_DISCONNECTED;
    }

    delay(1000);
  }

  // never reached here
  return E_NOTREACHABLE;
}

int M2XStreamClient::readContentLength() {
  int ret = waitForString("Content-Length: ");
  if (ret != E_OK) {
    return ret;
  }

  // From now on, ret is not needed, we can use it
  // to keep the final result
  ret = 0;
  while (true) {
    while (_client->available()) {
      char c = _client->read();
#ifdef DEBUG
      Serial.print(c);
#endif
      if ((c == '\r') || (c == '\n')) {
        return (ret == 0) ? (E_INVALID) : (ret);
      } else {
        ret = ret * 10 + (c - '0');
      }
    }

    if (!_client->connected()) {
#ifdef DEBUG
      Serial.println("ERROR: The client is disconnected from the server!");
#endif
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

void M2XStreamClient::close() {
  // Eats up buffered data before closing
  _client->flush();
  _client->stop();
}

static void on_key_found(jsonlite_callback_context* context,
                         jsonlite_token* token)
{
  json_parsing_context_state* state =
      (json_parsing_context_state*) context->client_state;
  if (strncmp((const char*) token->start, "at", 2) == 0) {
    state->state |= WAITING_AT;
  } else if ((strncmp((const char*) token->start, "value", 5) == 0) &&
             (token->start[5] != 's')) { // get rid of "values"
    state->state |= WAITING_VALUE;
  }
}

static void on_string_found(jsonlite_callback_context* context,
                            jsonlite_token* token)
{
  json_parsing_context_state* state =
      (json_parsing_context_state*) context->client_state;
  char* buf = NULL;
  uint8_t mask = 0;
  int buf_len = 0;

  if (TEST_IS_AT(state->state)) {
    buf = state->at_str;
    mask = GOT_AT;
    buf_len = AT_BUF_LEN;
  } else if (TEST_IS_VALUE(state->state)) {
    buf = state->value_str;
    mask = GOT_VALUE;
    buf_len = VALUE_BUF_LEN;
  }
  if (buf) {
    if (buf_len > (token->end - token->start)) {
      buf_len = token->end - token->start;
    }
    strncpy(buf, (const char*) token->start, buf_len);
    buf[buf_len] = '\0';
    state->state |= mask;
  }

  if (TEST_GOT_ALL(state->state)) {
    state->callback(state->at_str, state->value_str,
                    state->index++, state->context);
    state->state = state->at_str[0] = state->value_str[0] = 0;
  }
}

int M2XStreamClient::readStreamValue(stream_value_read_callback callback,
                                      void* context) {
  static const int BUF_LEN = 32;
  char buf[BUF_LEN];

  int length = readContentLength();
  if (length < 0) {
    close();
    return length;
  }

  int index = skipHttpHeader();
  if (index != E_OK) {
    close();
    return index;
  }
  index = 0;

  json_parsing_context_state state;
  state.state = state.index = state.at_str[0] = state.value_str[0] = 0;
  state.callback = callback;
  state.context = context;

  jsonlite_parser_callbacks cbs = jsonlite_default_callbacks;
  cbs.key_found = on_key_found;
  cbs.string_found = on_string_found;
  cbs.context.client_state = &state;

  jsonlite_parser p = jsonlite_parser_init(jsonlite_parser_estimate_size(5));
  jsonlite_parser_set_callback(p, &cbs);

  jsonlite_result result = jsonlite_result_unknown;
  while (index < length) {
    int i = 0;

#ifdef DEBUG
    Serial.print("Received Data: ");
#endif
    while ((i < BUF_LEN) && _client->available()) {
      buf[i++] = _client->read();
#ifdef DEBUG
      Serial.print(buf[i - 1]);
#endif
    }
#ifdef DEBUG
    Serial.println();
#endif

    if ((!_client->connected()) &&
        (!_client->available()) &&
        ((index + i) < length)) {
      jsonlite_parser_release(p);
      close();
      return E_NOCONNECTION;
    }

    result = jsonlite_parser_tokenize(p, buf, i);
    if ((result != jsonlite_result_ok) &&
        (result != jsonlite_result_end_of_stream)) {
      jsonlite_parser_release(p);
      close();
      return E_JSON_INVALID;
    }

    index += i;
  }

  jsonlite_parser_release(p);
  close();
  return (result == jsonlite_result_ok) ? (E_OK) : (E_JSON_INVALID);
}
