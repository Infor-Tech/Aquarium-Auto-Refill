# Aquarium-Auto-Refill

Code, diagrams and schematics for a aquarium refill controller

___

### What such device should do:

- Checks water level
  - If water level is lower than specified, controller turns on a pump
  - When pump runs for longer than specified, it is cut off
- Displays time and date
  - In future, this will be used for controlling lighting of the aquarium
    - At a specific time controller will turn on specific channels
- Checks temperature of the water

___

### Parts and libraries used for this project

- Arduino UNO
- DS18B20 digital temperature sensor
  - Libraries
    - OneWire.h (https://github.com/PaulStoffregen/OneWire)
    - DallasTemperature.h (https://www.arduino.cc/reference/en/libraries/dallastemperature/)
- DS1302 RTC module
  - Libraries
    - virtuabotixRTC.h (https://github.com/chrisfryer78/ArduinoRTClibrary)
- LCD1602 display with i2c module
  - Libraries
    - Wire.h (https://www.arduino.cc/en/reference/wire)
    - LiquidCrystal_I2C.h (https://www.arduino.cc/reference/en/libraries/liquidcrystal-i2c/)
- HC-SR04 ultrasonic ranging module
- 4ch relay (Now we are using only one channel, but 3 extra channels are making some room for the future)
- Pushbutton

___

### Diagrams and schematics

If you don't have Fritzing and you can't open diagrams and schematics here are screenshots for you:

![image](https://user-images.githubusercontent.com/72706877/136063962-b90599fa-fd89-4719-af90-b5a996fdf0fb.png)

![image](https://user-images.githubusercontent.com/72706877/136064160-4660c9ff-0716-4548-9416-1d21823090ca.png)
