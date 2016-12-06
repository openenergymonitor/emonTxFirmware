# Default emonTx Firmware

## Option 1.) Compile with Arduino IDE

http://openenergymonitor.org/emon/buildingblocks/setting-up-the-arduino-environment

## Option 2.) Compile and upload firmware using [PlatformIO](https://platformio.org)

## Install patformio (if needed)

See [platformio install quick start](http://docs.platformio.org/en/latest/installation.html#super-quick-mac-linux)

Recomended to use install script which may require sudo:

`python -c "$(curl -fsSL https://raw.githubusercontent.com/platformio/platformio/master/scripts/get-platformio.py)"`

## Compile
  
    $ pio run

## Upload

    $ pio run -t upload
    
## Test (optional)

See [PlatfomIO unit test docs](http://docs.platformio.org/en/feature-platformio-30/platforms/unit_testing.html#example). Requires PlatformIO 3.x

    $ pio test

