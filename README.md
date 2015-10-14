# simple-thermo
A Simple Arduino thermostat with hysteresis including 
* Three (3) temperature sensors, 
* Two (2) Relay's and 
* One (1) LCD
* One (1) SD Card for logging

The code is based in the example code from the dependencies 

# Build System
platformio http://platformio.org

## Platformio Dependencies
* [ 19  ] Adafruit-DHT https://github.com/adafruit/DHT-sensor-library
* [ 161 ] SD https://github.com/adafruit/SD
These dependecies are fetched using platformio lib install [id]

```
platformio lib install 19
platformio lib install 161
```


## Other dependencies
I downloaded and unziped the library (already included in this git)
* https://bitbucket.org/fmalpartida/new-liquidcrystal/downloads/
