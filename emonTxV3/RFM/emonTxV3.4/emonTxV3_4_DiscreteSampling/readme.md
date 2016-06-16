# Default emonTx Firmware

## Compile with Arduino IDE

http://openenergymonitor.org/emon/buildingblocks/setting-up-the-arduino-environment

## Compile and upload firmware using [PlatformIO](https://platformio.org)

## Install patformio if needed

See [platformio install quick start](http://docs.platformio.org/en/latest/installation.html#super-quick-mac-linux)

Recomended to use install script which may require sudo:

`python -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"`

or via python pipL

    sudo pip install platformio


## Compile
  
    $ pio run

## Upload

    $ pio run -t upload

## Test 

See [PlatfomIO unit test docs](http://docs.platformio.org/en/feature-platformio-30/platforms/unit_testing.html#example)

    $ pio test

## Install libs (no longer required)


**Note: installing libs is no longer requires since required libs are now defined in `platformio.ini`, lib installation will be prompted at fist compile**

*In current version of platformio (2.9) it's not possible to specify a particualr version when definining in .ini. This will be fixed in platformio 3.0*


Platformoio does not using the libraries in firmware/librarys instead we can install the libs direct from git via platform io lib manager.

jeeib:

    platformio lib install 252 --version="e70c9d9f4e"

dht22:

    platformio lib install 19 --version="09344416d2"

Dallas Temperature Control:

    platformio lib install 54 --version="3.7.7"
    
emonlib:

    platformio lib install 116 --version="1.0"

LiquidCrystal_I2C:

    platformio lib install 576 --version="4bb48bd648"


