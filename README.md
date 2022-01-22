# NIMBU

## Prerequisites

```
mkdir -p esp
cd esp
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf
./install.sh all
```

Now that ESP-IDF is installed, we'll also install an alias to quickly launch the ESP-IDF environment.
```
echo "alias launch_idf='. $HOME/esp/esp-idf/export.sh'" >> ~/.bashrc
source ~/.bashrc
```

## Project Structure

```
root
├── nimbu
│   ├── build
│   ├── CMakeLists.txt
│   ├── components
│   ├── main
│   │   ├── bt
│   │   ├── CMakeLists.txt
│   │   ├── common
│   │   ├── led
│   │   ├── main.cpp
```

Nimbu is based on ESP-IDF, which runs on FreeRTOS.  
All commands related to ESP-IDF funcitonality - building/flashing/etc should be done from within the `nimbu` folder.
```
cd nimbu
```

`components` contains the ESP-IDF tools and libs required for building the application.  
`main` contains the entire source.  
Make sure that the path to CMake is that of the linked ESP-IDF installation.

### Configure the project

Current sdkconfig already includes the necessary settings.  
In case reconfiguration is required, make sure to:

* Set the use of external I2S codec for audio output, and configure the output PINs
* Enable Classic Bluetooth and A2DP under Component config --> Bluetooth --> Bluedroid Enable

```
idf.py set-target esp32
idf.py menuconfig
```

### Build and Flash

Build the project.

```
idf.py build
```

Flash it to the board, then run monitor tool to view serial output.

```
idf.py -p PORT flash monitor
```

(To exit the serial monitor, type ``Ctrl-]``.)
