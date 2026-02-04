# RV-IOT Sensor Monitoring System

**Real-Time Edge Computing & Cloud Analytics**
An ESP32-powered IoT node integrating multi-sensor fusion, local HMI, and remote data logging.

## Overview

The **RV-IOT Monitoring System** is a versatile IoT application designed for high-speed environmental data processing. Built around the **ESP32-WROOM-32** microcontroller, the system leverages dual-core processing to maintain a responsive local interface while simultaneously handling cloud connectivity. It serves as a comprehensive example of an end-to-end IoT pipeline—from raw analog signal acquisition to cloud-based visualization.

## Key Functionalities

* **Intelligent Sensing**: Real-time monitoring of light intensity (LDR) and analog inputs (Potentiometer) with 12-bit ADC precision (0-4095).
* **TFT Dashboard**: Displays live readings, timestamps, and system status with color-mapped feedback.
* **NeoPixel Feedback**: A 16-million color LED programmed to change hues based on sensor thresholds (e.g., Green for low, Red for high).
* **Temporal Logging**: Integrated **DS1307 RTC** ensures every data packet is timestamped with the exact Date, Day, and Time.
* **Interactive Control**: A matrix keypad interface allowing the user to query historical metrics (Minimum/Maximum values) instantly on the local display.
* **ThingSpeak Integration**: Synchronous updates to the cloud every 20 seconds, facilitating remote analysis and long-term data storage.

## Hardware & Components

* **Controller**: ESP32 (160MHz - 240MHz) with integrated Wi-Fi and Dual-mode Bluetooth.
* **Display**: SPI-based TFT Display (ST7735/ILI9341 compatible).
* **Sensors**: Potentiometer (Analog Pin 36) and LDR (Analog Pin 39).
* **RTC**: DS1307 via I2C.
* **UI**: I2C Matrix Keypad and WS2812B NeoPixel.

## Software Logic

The firmware utilizes a robust main loop that samples data into a 40-element buffer.

1. **Local Loop**: Samples sensors every 500ms, updates the TFT, and monitors the keypad for interrupts.
2. **Stat Engine**: When keys '1' or '2' are pressed, the system calculates and displays the `min_pot_val` or `max_pot_val` from the current session.
3. **Cloud Sync**: After every 40 local cycles, the system initiates a secure Wi-Fi handshake and pushes the latest datasets to the **ThingSpeak** channel using the `writeFields` API.
