### Water leak detection for Arduino controller

An Arduino code for getting measures from sensors (DHT11, Iduino SE045) with esp8266 controller (Node MCU v3) and send it to mqtt. Program sends the data collected by the sensors every 5 seconds.

#### How to compile and build
1) Add esp8266 json to arduino preferences
2) Install libraries in arduino (DHT by Adafruit, NTPClient by Fabrice Weinberg, ArduinoJSON by Benoit Blanchon, PubSubClient by Nick O'Leary)
3) Open `.ino` file and compile it. Upload to device. Enjoy. 