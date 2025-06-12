# Crow Runner Interface

Interface for monitoring and controlling Crow Runner alarm panels using ESP32 and ESP-IDF.

## Description

This project implements an ESP32-based interface to interact with Crow Runner alarm panels. It allows OTA (Over-The-Air) firmware updates through a web interface, as well as analysis and visualization of frames and commands from the panel protocol.

## Main Features

- **Embedded HTTP server**: Provides a web page to upload and update the ESP32 firmware via OTA.
- **Protocol analysis**: Includes documentation and utilities to interpret frames and commands from the Crow Runner panel.
- **Flexible configuration**: Uses PlatformIO and ESP-IDF to facilitate building and deployment.
- **Command parsing example**: See `protocol parsing.md` for examples of supported frames and commands.

## Project Structure

- `/src`: Main source code (OTA server, communication logic, etc.).
- `/include`: Header files.
- `/protocol_parsing.md`: Documentation about the protocol and analyzed frames.
- `/platformio.ini`: PlatformIO configuration.
- `/sdkconfig.*`: ESP-IDF configuration.
- `/partitions_*.csv`: Partition tables for the firmware.

## Requirements

- [PlatformIO](https://platformio.org/) (recommended) or ESP-IDF 5.x
- Compatible ESP32 board (e.g., esp32doit-devkit-v1)
- WiFi network connection

## Basic Usage

1. **Clone the repository**  
   ```sh
   git clone https://github.com/youruser/crow-runner-interface.git
   cd crow-runner-interface
   ```

2. **Configure PlatformIO**  
   Adjust `platformio.ini` if necessary for your board.

3. **Build and upload the firmware**  
   ```sh
   pio run -t upload
   ```

4. **Access the OTA interface**  
   Connect the ESP32 to the network, open a browser, and go to the device's IP address to view the OTA update page.

5. **Update firmware via web**  
   Use the web page served by the ESP32 to upload a new `.bin` file and update the firmware.

## Circuit Details

The circuit includes a bidirectional logic level converter (5V to 3.3V) to adapt the alarm signal voltage levels to the ESP32.  
An HEF40106BP (inverting Schmitt trigger gates) was used to filter noise on the DAT and CLK signals. For this reason, CLK and DAT signals are inverted during sampling.

## Notes

- The `protocol parsing.md` file contains examples and descriptions of the supported frames and commands.
- The OTA server is implemented in `src/ota_http_server.c`.

## License

MIT

## Credits

Developed by G. Cibeira.
Based on ESP-IDF and community examples.
Bit decoding (HDLC like protocol) was based on code from [ESP-CrowAlarmInterface](https://github.com/MadDoct/ESP-CrowAlarmInterface.git) and [crowalarm](https://github.com/sivann/crowalarm).
