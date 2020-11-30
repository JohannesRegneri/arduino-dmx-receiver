# Arduino-dmx-receiver

The Software allows you to receive DMX512 Data with an Arduino an the SN75176bp.

Tested with Arduino nano & Arduino Uno

You need to use the [Arduino IDE 1.0.6](https://www.arduino.cc/en/Main/OldSoftwareReleases)
  - replace the original ...arduino-1.0.6\hardware\arduino\cores\arduino\HardwareSerial.cpp 
  file with the one in this Archive
  - use the Test File for two RGB LEDs
  
 !unplug the RX Pin while uploading the Code!

 
### Pinout
  
  #### Pinout for SN75176bp
  | Pin | Name ||  Name | Pin|
  |-----|-------|-|------|-------|   
  | 1-> | RX      || 5-> | VC  | 
  | 2-> | GND     || 6-> | PORT A  |   
  | 3-> | GND     || 7-> | PORT B  | 
  | 4-> | n.A     || 8-> | GND | 
   
  #### XLR-Connector
  | Pin | Name | 
  |-----|-------|
  | 1 | GND | 
  | 2 | PORT A | 
  | 3 | PORT B | 
