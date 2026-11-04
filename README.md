# Water Turbidity Monitor

An Arduino sketch for monitoring relative water turbidity with a sensor, displaying the result on an I2C LCD, and optionally reporting readings to ThingSpeak.

## Hardware

- Raspberry Pi Pico W (the sketch uses Pico-style `Wire.setSDA` / `Wire.setSCL`)
- Turbidity sensor connected to GP26 / ADC0
- 16x2 I2C LCD at address `0x27`
- I2C SDA on GP4 and SCL on GP5

Check your sensor's voltage range and use a suitable divider before connecting its output to the board.

## Arduino libraries

Install these libraries using Arduino IDE's Library Manager:

- LiquidCrystal_I2C
- ThingSpeak

Select a Raspberry Pi Pico W board supported by your installed Arduino core.

## Configure and run

1. Open `turbidityfinal.ino` in Arduino IDE.
2. Replace `YOUR_WIFI_SSID`, `YOUR_WIFI_PASSWORD`, and `YOUR_THINGSPEAK_WRITE_API_KEY` in the sketch with your own values. Do not commit real credentials to a public repository.
3. Set `channelID` to your ThingSpeak channel ID.
4. Check the sensor calibration constants `CLEAR_ADC` and `CLOUDY_ADC` against your hardware.
5. Select the Pico W board and port, then upload.
6. Open Serial Monitor at 115200 baud.

The sketch averages 10 ADC readings, maps them to a relative 0-100 index, labels the result Pure, Cloudy, or Muddy, displays it on the LCD, and sends the index to ThingSpeak every 20 seconds while connected.

## Notes

The index is a simple calibration-based estimate, not a certified water-quality measurement. The LCD library and board core may vary by setup; adjust dependencies and wiring if needed.
