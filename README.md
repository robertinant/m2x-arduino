m2x-arduino
===========

The Arduino library used to send/receive data to/from [AT&amp;T's M2X service](https://m2x.att.com/).

How to Install the library
==========================

This library depends on [jsonlite](https://github.com/amamchur/jsonlite), the installation steps are as follows:

1. Clone the [jsonlite](https://github.com/amamchur/jsonlite) repository.
2. Open the Arduino IDE, click `Sketch->Import Library...->Add Library...`, then navigate to `amalgamated/jsonlite` folder in the cloned jsonlite repository. The jsonlite library will be imported to Arduino this way.

   **NOTE**: If you cloned the jsonlite library, there will be 3 folders named jsonlite:
   * `jsonlite`: the repo folder
   * `jsonlite/jsonlite`: the un-flattened jsonlite source folder
   * `jsonlite/amalgamated/jsonlite`: the flattened jsonlite source for arduino

   The last one here should be the one to use, the first 2 won't work!
3. Use the same way as #2 to import `M2XStreamClient` library in the current folder.
4. Now you can find M2X examples under `File->Examples->M2XStreamClient`
5. Enjoy coding!

Hardware Setup
==============

Board Setup
-----------

The Arduino website has a very good [tutorial](http://arduino.cc/en/Guide/HomePage) on setting up the Arduino board. It contains detailed instructions on how to install the Arduino IDE, sets up your board for initial testing. Feel free to proceed to the [Arduino site](http://arduino.cc/en/Guide/HomePage) to get a basic idea on Arduino.

Wifi/Ethernet Shield Setup
--------------------------

To send data to the AT&amp;T M2X server, or receive data from the AT&amp;T M2X server, your Arduino board needs connection to the Internet. Hence an Arduino [Wifi Shield](http://arduino.cc/en/Main/ArduinoWiFiShield) or [Ethernet Shield](http://arduino.cc/en/Main/ArduinoEthernetShield) is needed to give your board the power to connect to the Internet. To install the shield, hook the shield on your Arduino board, you can use the pins on the shield the same way as the real pins on the Arduino boards.

Sensor Setup
------------

Different sensors can be hooked on an Arduino board to provide different properties including temperatures, humidity, etc. You can use a breadboard as well as wires to connect different sensors to your Arduino. For detailed tutorial on connecting different sensors, please refer to the Arduino [Examples page](http://arduino.cc/en/Tutorial/HomePage).


Variables used in Examples
==========================

In order to run the given examples, different variables need to be configured. We will walk through those variables in this section.

Network Configuration
---------------------

If you are using a Wifi Shield, the following variables need configuration:

```
char ssid[] = "<ssid>";
char pass[] = "<WPA password>";
```

Just fill in the SSID and password of the Wifi hotspot, you should be good to go.

For an Ethernet Shield, the following variables are needed:

```
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
IPAddress ip(192,168,1,17);
```

For a newer Ethernet Shield, the MAC address should be printed on a sticker on the shield. However, some sold Ethernet Shields have no MAC address shown on the board. In this case, you can use any MAC address, as long as it is not conflicted with another network device within the same LAN.

The IP address here is only used when DHCP fails to give a valid IP address. It is recommended, though not required, to provide a unique IP address here.

M2X API Key
-----------

Once you [register](https://m2x.att.com/signup) for an AT&amp;T M2X account, an API key is automatically generated for you. This key is called a _Primary Master Key_ and can be found in the _Master Keys_ tab of your [Account Settings](https://m2x.att.com/account). This key cannot be edited nor deleted, but it can be regenerated. It will give you full access to all APIs.

However, you can also create a _Data Source API Key_ associated with a given Data Source(Feed), you can use the Data Source API key to access the streams belonging to that Data Source.

You can customize this variable in the following line in the examples:

```
char m2xKey[] = "<M2X access key>";
```

Feed ID
-------

A feed is associated with a data source, it is a set of data streams, such as streams of locations, temperatures, etc. The following line is needed to configure the feed used:

```
char feedId[] = "<feed id>";
```

Stream Name
------------

A stream in a feed is a set of timed series data of a specific type(i,e. humidity, temperature), you can use the M2XStreamClient library to send stream values to M2X server, or receive stream values from M2X server. Use the following line to configure the stream if needed:

```
char streamName[] = "<stream name>";
```

Using the M2XStreamClient library
=========================

The M2X Arduino library can be used with both Wifi connection and Ethernet connection. For a Wifi connection, use the following code:

```
WiFiClient client;
M2XStreamClient m2xClient(&client, m2xKey);
```

For an Ethernet connection, use the following code:

```
EthernetClient client;
M2XStreamClient m2xClient(&client, m2xKey);
```

In the M2XStreamClient, 4 types of API functions are provided here:

* `send`: Send stream value to M2X server
* `receive`: Receive stream value from M2X server
* `updateLocation`: Send location value of a feed to M2X server
* `readLocation`: Receive location values of a feed from M2X server

Returned values
---------------

For all those functions, the HTTP status code will be returned if we can fulfill a HTTP request. For example, `200` will be returned upon success, `401` will be returned if we didn't provide a valid M2X API Key.

Otherwise, the following error codes will be used:

```
static const int E_NOCONNECTION = -1;
static const int E_DISCONNECTED = -2;
static const int E_NOTREACHABLE = -3;
static const int E_INVALID = -4;
static const int E_JSON_INVALID = -5;
```

Send stream value
-----------------

The following functions can be used to send value to a stream, which belongs to a feed:

```
int send(const char* feedId, const char* streamName, double value);
int send(const char* feedId, const char* streamName, long value);
int send(const char* feedId, const char* streamName, int value);
int send(const char* feedId, const char* streamName, const char* value);
```

Each one here requires a feed ID and a stream name, different types of value can be used here. Feel free to use any of them to send stream values to M2X server.

Receive stream value
--------------------

Since Arduino board contains very limited API, we cannot put the whole returned string in memory, parse it into JSON representations and read what we want. Instead, we use a callback-based mechanism here. We parse the returned JSON string piece by piece, whenever we got a new stream value point, we will call the following callback functions:

```
void (*stream_value_read_callback)(const char* at,
                                   const char* value,
                                   int index,
                                   void* context);

```

The implementation of the callback function is left for the user to fill in, you can read the value of the point in the `value` argument, and the timestamp of the point in the `at` argument. We even pass the index of this this data point in the whole stream as well as a user-specified context variable to this function, so as you can perform different tasks on this.

To read the stream values, all you need to do is calling this function:

```
int receive(const char* feedId, const char* streamName,
            stream_value_read_callback callback, void* context);
```

Besides the feed ID and stream name, only the callback function and a user context needs to be specified.

Update Datasource Location
--------------------------

You can use either of the following functions to update the location for a data source(feed):

```
int updateLocation(const char* feedId, const char* name,
                   double latitude, double longitude, double elevation);
int updateLocation(const char* feedId, const char* name,
                   const char* latitude, const char* longitude,
                   const char* elevation);
```

Different from stream values, locations are attached to feeds rather than streams.

The reasons we are providing these 2 functions is due to floating point value precision: on most Arduino boards, `double` is the same as `float`, i.e., 32-bit (4-byte) single precision floating number. That means only 7 digits in the number is reliable. When we are using `double` here to represent latitude/longitude, it means only 5 digits after the floating point is accurate, which means we can represent as accurate to ~1.1132m distance using `double` here. If you want to represent cordinates that are more specific, you need to use strings here.

Read Datasource Location
------------------------

Similar to reading stream values, we also use callback functions here. The only difference is that different parameters are used in the function:

```
void (*location_read_callback)(const char* name,
                               double latitude,
                               double longitude,
                               double elevation,
                               const char* timestamp,
                               int index,
                               void* context);

```

For memory space consideration, now we only provide double-precision when reading locations. An index of the location points is also provided here together with a user-specified context.

The API is also slightly different, in that the stream name is not needed here:

```
int readLocation(const char* feedId, location_read_callback callback,
                 void* context);

```

Examples
========

We provide a series of examples. By reading these examples, we believe that you can get an idea of how to use the `M2XStreamClient` library to perform all kinds of tasks.

Note that the examples contain fictionary variables, they need to be configured as the description above said before running on the Arduino board. Each of the examples here also needs either a Wifi Shield or an Ethernet Shield hooked on your device.

In the `PostExample` and `EthernetPostExample`, a temperature sensor, a breadboard and 5 wires are also needed to get data of the temperature, you need to wire the board like [this](http://cl.ly/image/3M0P3T1A0G0l) before running the code.

After you have configured variables and the board, plug the Arduino board onto your computer via a Micro-USB cable, click `Verify` in the Arduino IDE, then click `Upload`, the code should be uploaded to the board. You can check all the outputs in the `Serial Monitor` of the Arduino IDE.`

PostExample
-----------

This example shows how to post temperatures to M2X server. Before running this, you need to have a valid M2X Key, a feed ID and a stream name. The Arduino board needs to be configured like [this](http://cl.ly/image/3M0P3T1A0G0l). In this example, we are using an [Arduino Uno](http://arduino.cc/en/Main/arduinoBoardUno) board. If you are using other boards, keep in mind that we are reading from `A0` in the code, the wiring should be similar to this one shown in the illustration.

ReceiveExample
--------------

This example reads stream values from M2X server. And prints the stream data point got to Serial interface. You can find the actual values in the Arduino `Serial Monitor`.

EthernetPostExample
-------------------

This one is similar to the `PostExample`, except that EthernetClient is used instead of WifiClient. If you are using an Ethernet Shield instead of a Wifi Shield, you can use this example.

EthernetReceiveExample
----------------------

This one is similar to the `ReceiveExample`, except that EthernetClient is used instead of WifiClient.

UpdateLocationExample
---------------------

This one sends location data to M2X server. Idealy a GPS device should be used here to read the cordinates, but for simplicity, we just use pre-set values here to show how to use the API.

ReadLocationExample
-------------------

This one reads location data of a feed from M2X server, and prints them to Serial interfact. You can check the output in the `Serial Monitor` of the Arduino IDE.

