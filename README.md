# Crow Runner Interface

Interface for monitoring and controlling Crow Runner alarm panels using ESP32 and ESP-IDF.

## Overview

This project implements an ESP32-based interface to interact with Crow Runner alarm panels. It enables Over-The-Air (OTA) firmware updates via a web interface, as well as analysis and visualization of protocol frames and commands from the panel.

## Project Structure

- `/src`: Main source code (HTTP server, communication logic, etc.).
- `/include`: Header files.
- `/data`: HTML files served by the ESP32 (OTA, alarm status, etc.).
- `/scripts`: Utilities for decoding and testing.
- `/protocol.md`: Documentation about the protocol and analyzed frames.
- `/platformio.ini`: PlatformIO configuration.
- `/sdkconfig.*`: ESP-IDF configuration.
- `/partitions.csv`: Partition tables for the firmware and file system.

## Modules and Features

### 1. `frame_handler`
- Captures and decodes bits received from the alarm panel's serial bus (CLK and DAT).
- Detects frame delimiters and performs bit unstuffing.
- Assembles bytes and builds complete frames, which are sent via a queue to the processing task.
- Calls `process_frame()` for each received frame.

### 2. `protocol_handler`
- Interprets received frames according to the Crow Runner protocol.
- Generates high-level events: keypad presses, system state changes, zone activity, etc.
- Allows registering callbacks for each event type.

### 3. `alarm_manager`
- Maintains the global alarm state (system state, active/triggered zones).
- Receives events from `protocol_handler` and updates the state.
- Allows registering a callback to notify state changes.
- Exposes the current state for other modules to query.

### 4. `wifi_manager`
- Manages WiFi connection in station (STA) mode.
- Uses credentials defined in `wifi_secrets.h`.
- Automatically retries connection and exposes connection status.

### 5. `http_server`
- Embedded HTTP server using ESP-IDF.
- Registers and manages HTTP and WebSocket endpoints.
- Notifies WebSocket clients when the alarm state changes.

### 6. HTTP Handlers
- `/src/handlers/root_handler.c`: Serves the main page (`/`).
- `/src/handlers/ota_handler.c`: Serves the OTA update page (`/ota`).
- `/src/handlers/alarm_handler.c`: Serves the alarm status page (`/alarm`).
- `/src/handlers/ws_alarm_handler.c`: WebSocket endpoint for real-time notifications (`/ws_alarm`).
- `/src/handlers/ota_update_handler.c`: Receives firmware via POST for OTA update (`/ota_update`).
- `/src/handlers/notify_alarm_ws_clients.c`: Notifies all connected WebSocket clients when the alarm state changes.

## HTTP Endpoints

- `GET /`           : Main page (HTML, user interface).
- `GET /ota`        : OTA update page (HTML).
- `GET /alarm`      : Alarm status page (HTML).
- `GET /ws_alarm`   : WebSocket for real-time alarm state notifications (JSON).
- `POST /ota_update`: Receives the binary firmware file for OTA update.

## Data Flow: Serial → Alarm State

1. **Serial Reception** (`frame_handler`):
   - Interrupts on each CLK edge, reads the DAT bit, performs bit unstuffing, and detects frame delimiters.
   - Assembles bytes and builds a complete frame.
   - Sends the frame to the processing queue.

2. **Frame Processing** (`protocol_handler`):
   - Interprets the frame according to the Crow Runner protocol.
   - Generates a high-level event (keypad, state, zones, etc.).
   - Calls registered callbacks for each event type.

3. **State Management** (`alarm_manager`):
   - Receives events and updates the global alarm state.
   - If there are changes, notifies registered modules (e.g., the HTTP server to notify WebSocket clients).

4. **Web/HTTP Notification**:
   - The HTTP server exposes endpoints to query the state and receive real-time notifications via WebSocket.
   - State changes automatically trigger notifications to all connected clients.

## Basic Usage

1. **Clone the repository**
   ```sh
   git clone https://github.com/youruser/crow-runner-interface.git
   cd crow-runner-interface
   ```

2. **Create WiFi credentials file**
   Create the file `include/wifi_secrets.h` with the following content, replacing with your SSID and password:
   ```c
   #define WIFI_SSID     "YourSSID"
   #define WIFI_PASSWORD "YourPassword"
   ```

3. **Configure PlatformIO**
   Adjust `platformio.ini` if necessary for your board.

4. **Build and upload the firmware**
   ```sh
   pio run -t upload
   ```

5. **Access the OTA web interface**
   Connect the ESP32 to the WiFi network, open a browser, and go to the assigned IP address to view the OTA update page.

6. **Update firmware via web**
   Use the page served by the ESP32 to upload a new `.bin` file and update the firmware.

## Circuit Details

The circuit includes a bidirectional logic level converter (5V to 3.3V) to adapt the panel signals to the ESP32. Schmitt Trigger gates are used to filter noise on the DAT and CLK signals.

## Notes

- The `protocol.md` file contains examples and descriptions of supported frames and commands.
- The OTA server is implemented in `src/handlers/ota_update_handler.c`.
- The alarm state is exposed in real time via WebSocket (`/ws_alarm`).

## License

MIT

## Credits

Developed by Gerardo Cibeira.
Based on ESP-IDF and community examples.
Bit decoding (HDLC-like protocol) based on code from [ESP-CrowAlarmInterface](https://github.com/MadDoct/ESP-CrowAlarmInterface.git) and [crowalarm](https://github.com/sivann/crowalarm).
