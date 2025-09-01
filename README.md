# SM7901_pico

<img src="SilentPico.png" alt="SilentPico" width="400">

This project (unnamed yet) offers a simple and innovative solution to automate the process of maintaining noise pollution using Rapberry Pi Pico W with BitDogLab Kit.

## Features

1. Noise detection
2. Digital-to-dB conversion
3. Media, max and min dB measurement
4. Wi-Fi and MQTT connectivity
5. Local mode if Wi-Fi connection fails
6. Auto Wi-Fi and MQTT reconnection
7. Store dB values on flash if connection fails

## Hardware Requirements

* Raspberry Pi Pico W
* SM7901TTL sensor (3.3V output)
* Oled Display (Optional) connected on GPIO 14 (SDA) 15 (SCL) (if you are not using Pico W with BITDOGLAB board)
* Button at GPIO 5 (Optional)

## Installation

Currently, SilentPico does not yet have the functionality to edit the wifi SSID/PASSWORD, MQTT topic/Broker IP Addres and other parameters, you will need to install a IDE to edit the code and compile, follow these steps:

### Windows:

1. Install Pico W drivers with [Zadig](https://zadig.akeo.ie/)
2. Install [Visual Studio Code](https://code.visualstudio.com/Download)
3. Install the Raspberry Pi Pico VS Code Extension (There's no need to install other things, Pico extension do everything)
4. Now you can clone this repo, install submodules and finally compile.

See the oficial [documentation](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf) for more detailed steps.

## Usage

1. Libraries
   
2. Traffic noise measurement

4. Industry

5. Apartments

6. Another noise measurement you need in function of time

## TODO

The project is still under development, some upgrades are planned to be out very soon:

- [ ] Release .uf2 file
- [ ] Better way to edit Wi-Fi SSID and PASSWORD and other settings
- [x] DNS Resolve for broker ip
- [x] Store JSONs on flash
- [x] Option to alternate Local and MQTT running mode
- [x] Wi-Fi and MQTT: auto reconnection

## Autors

* **jadielucas** (Jadiel Lucas)

## Thanks to

* [**tjko**](https://github.com/tjko) for pico-lfs driver
* [**daschr**](https://github.com/daschr) for ssd1306 pico driver

## License

This project is licensed under the MIT License - see [LICENSE.md](https://github.com/jadielucas/IP_Project/blob/main/LICENSE) for more details.
