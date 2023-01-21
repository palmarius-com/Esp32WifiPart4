
#include <WiFi.h>                                     // needed to connect to WiFi
//|-- WiFi @ 2.0.0

#include <ESPAsyncWebServer.h>                        // needed to create a simple webserver (make sure tools -> board is set to ESP32, otherwise you will get a "WebServer.h: No such file or directory" error)
// lib_deps = me-no-dev/AsyncTCP @ ^1.1.1
// lib_deps = me-no-dev/ESP Async WebServer @ ^1.2.3                // broken !!
// lib_deps = ottowinter/ESPAsyncWebServer-esphome @ ^3.0.0         // use this instead

#include <WebSocketsServer.h>                         // needed for instant communication between client and server through Websockets
// lib_deps = links2004/WebSockets @ ^2.3.7                         // generates Warnings :-/ hope and pray

#include <ArduinoJson.h>                              // needed for JSON encapsulation (send multiple variables with one string)
// lib_deps = bblanchon/ArduinoJson  @ ^6.20.0                      //ok

#include <SPIFFS.h>
//|-- SPIFFS @ 2.0.0
//|   |-- FS @ 2.0.0

// Prototypes 
void webSocketEvent(byte num, WStype_t type, uint8_t *payload, size_t length);
void sendJson(String l_type, String l_value);
void sendJsonArray(String l_type, int l_array_values[]);


// first startUp after Upload
/*

------
E (31) SPIFFS: mount failed, -10025
[    37][E][SPIFFS.cpp:89] begin(): Mounting SPIFFS failed! Error: -1
SPIFFS could not initialize
Setting up Access Point ... [   176][E][WiFiGeneric.cpp:184] set_esp_interface_ip(): The GatewayP address (192.168.1.2) can't be within the DHCP range (192.168.1.2 -- 192.168.1.12)
Failed!
Starting Access Point ... Ready
IP address = 192.168.1.1
----
after connect:
Starting Access Point ... Ready
IP address = 192.168.1.1
[335504][E][vfs_api.cpp:24] open(): File system is not mounted
[335505][E][vfs_api.cpp:24] open(): File system is not mounted
[335558][E][vfs_api.cpp:24] open(): File system is not mounted
[335559][E][vfs_api.cpp:24] open(): File system is not mounted
[335559][E][vfs_api.cpp:24] open(): File system is not mounted
[335564][E][vfs_api.cpp:24] open(): File system is not mounted
----

    so far everything as expected

    PlatformIO icon
    Platform -> Build Filesystem Image

----
Building FS image from 'data' directory to .pio\build\esp-wrover-kit\spiffs.bin
/chart.js
/favicon.png
/javascript.js
/webpage.html
============================================================== [SUCCESS]
----

 
    PlatformIO icon
    Platform -> Upload Filesystem Image
   
    caused ***[uploadfs] Error 2
    just like explained on https://randomnerdtutorials.com/esp32-vs-code-platformio-spiffs/
    I didn't close the Serialmonitor

    I used the serial-monitor, because I'm using custom boards with the chip only.
    I always need to GND GPIO0 and GPIO2 for the ESP to say "waiting for upload"
    before I can transmit data ...

    after I saw that the ESP is in boot-mode
    I closed the serial-monitor and pressed "Platform -> Upload Filesystem Image" again

----
Building FS image from 'data' directory to .pio\build\esp-wrover-kit\spiffs.bin
/chart.js
/favicon.png
/javascript.js
/webpage.html
Looking for upload port...
Auto-detected: COM10
Uploading .pio\build\esp-wrover-kit\spiffs.bin
esptool.py v4.4
Serial port COM10
Connecting....
Chip is ESP32-D0WD-V3 (revision v3.0)
Features: WiFi, BT, Dual Core, 240MHz, VRef calibration in efuse, Coding Scheme None
Crystal is 40MHz
MAC: 08:b6:1f:27:2c:f0
Uploading stub...
Running stub...
Stub running...
Changing baud rate to 460800
Changed.
Configuring flash size...
Flash will be erased from 0x00290000 to 0x003effff...
Compressed 1441792 bytes to 153848...
Writing at 0x00290000... (10 %)
Writing at 0x002a8f3e... (20 %)
Writing at 0x002c349d... (30 %)
Writing at 0x002dfbd1... (40 %)
Writing at 0x002fb6e8... (50 %)
Writing at 0x003158cc... (60 %)
Writing at 0x0032fa25... (70 %)
Writing at 0x003498ad... (80 %)
Writing at 0x00363b8d... (90 %)
Writing at 0x0037c9d1... (100 %)
Wrote 1441792 bytes (153848 compressed) at 0x00290000 in 9.7 seconds (effective 1186.6 kbit/s)...
Hash of data verified.

Leaving...
Hard resetting via RTS pin...
============================================================= [SUCCESS] 
----

    perfect :-)
    rebooted the ESP (with floating GPIO0 and GPIO2)

----
rst:0x1 (POWERON_RESET),boot:0x12 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0030,len:1184
load:0x40078000,len:13104
load:0x40080400,len:3036
entry 0x400805e4
Setting up Access Point ... [   285][E][WiFiGeneric.cpp:184] set_esp_interface_ip(): The GatewayP address (192.168.1.2) can't be within the DHCP range (192.168.1.2 -- 192.168.1.12)
Failed!
Starting Access Point ... Ready
IP address = 192.168.1.1
Client 0 connected              // saw the graph moving
Client 0 disconnected           // after closing the webpage in chrome on iPhone
----


*/
