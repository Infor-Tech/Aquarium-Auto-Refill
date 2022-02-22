/*  Aquarium refill controler
 *
 *  github repo: https://github.com/Infor-Tech/Aquarium-Auto-Refill/
 *
 *  Parts used in this project:
 *      - Arduino UNO
 *      - LCD1602 display with i2c module
 *      - DS18B20 digital temperature sensor
 *      - DS1302 RTC module
 *      - HC-SR04 ultrasonic ranging module
 *      - 4ch relay
 *      - pushbutton
 *
 *  fritzing project, diagrams and schematics avaiable
 *
 *  Author: KRYSTIAN SLIWINSKI
 *  Contact: k.sliwinski@protonmail.com
 *  github: https://github.com/Infor-Tech
 */

// config
#define OPTIMUM_OF_WATER 5 // in cm
#define LITTLE_OF_WATER 7  // in cm
#define NO_WATER 10        // in cm
#define PUMP_TRIGGER 5     // in cm
#define PUMP_TARGET 3
#define PUMP_CUTOFF 15000 // in ms

// temperature sensor config
#include <OneWire.h>
#include <DallasTemperature.h>
#define TEMPERATURE_BUS 2

OneWire oneWire(TEMPERATURE_BUS);
DallasTemperature sensors(&oneWire);

// water level sensor(s)
#define MAIN_WATER_LEVEL_SENSOR_TRIG 4
#define MAIN_WATER_LEVEL_SENSOR_ECHO 5

// rtc config -> clock(CLK, DAT, RST)
#include <virtuabotixRTC.h>
virtuabotixRTC clock(13, 12, 11);

// pump
#define RELAY_PUMP_PIN 6
unsigned long pump_running_since;
bool is_pump_running_since_set = false;
bool pump_error = false;

// lcd
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

// control button
#define INTERFACE_CHOICE_BTN 3

// multitasking setup
const unsigned int log_interval = 1000;
unsigned long log_time = 0;
unsigned long last_time = millis();

// variable used, to define, which tab should be displayed on a screen
byte selected_tab = 1;

// error handeling
String throw_error = "      NONE      ";

// icons
byte thermometer_ico[8] = {B00100, B01010, B01010, B01110, B01110, B11111, B11111, B01110}; // icon for thermometer
byte water_ico[8] = {B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110};       // icon for water droplet
byte clock_ico[8] = {B00000, B01110, B10101, B10111, B10001, B01110, B00000, B00000};       // icon for clock
byte error_ico[8] = {B11111, B01110, B01110, B00100, B00000, B00100, B01110, B00100};       // icon for error info

void setup()
{
    // temperature sensor init
    sensors.begin();

    // water level sensor init
    pinMode(MAIN_WATER_LEVEL_SENSOR_TRIG, OUTPUT);
    pinMode(MAIN_WATER_LEVEL_SENSOR_ECHO, INPUT);

    lcd.init();
    lcd.backlight();

    // pump init
    pinMode(RELAY_PUMP_PIN, OUTPUT);
    digitalWrite(RELAY_PUMP_PIN, LOW);

    pinMode(INTERFACE_CHOICE_BTN, INPUT_PULLUP);

    // create characters from icons
    lcd.createChar(0, thermometer_ico);
    lcd.createChar(1, water_ico);
    lcd.createChar(2, clock_ico);
    lcd.createChar(3, error_ico);
}

void loop()
{
    unsigned long elapsed_time = millis() - last_time;
    last_time += elapsed_time;

    interface_choice();
    display_interface(elapsed_time);
    refill_water();
}

/**
 * Checks water level with HC-SR04
 * @return distance between a sensor and water surface in centimeters
 */
int check_water_level()
{
    digitalWrite(MAIN_WATER_LEVEL_SENSOR_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(MAIN_WATER_LEVEL_SENSOR_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(MAIN_WATER_LEVEL_SENSOR_TRIG, LOW);
    return pulseIn(MAIN_WATER_LEVEL_SENSOR_ECHO, HIGH) / 58;
}

// When water level drops below specific level (PUMP_TRIGGER) pump turns on and runs until a water level rises to the PUMP_TARGET
// Pump cannot run for longer than specified in PUMP_CUTOFF. When pump cuts off, it can no longer be triggered
void refill_water()
{
    byte water_lvl = check_water_level();
    if (!pump_error && water_lvl > PUMP_TRIGGER)
    {
        if (!is_pump_running_since_set)
        {
            pump_running_since = millis();
            is_pump_running_since_set = true;
        }
        else if (millis() - pump_running_since < PUMP_CUTOFF)
            digitalWrite(RELAY_PUMP_PIN, LOW); // safety check, pump cannot run for longer than specified in PUMP_CUTOFF
        else
        {
            throw_error = (throw_error == "TEMP SENSOR ERR ") ? "TEMP & LVL ERROR" : "LVL SENSOR ERROR";
            pump_error = true;
        }
    }
    else if (pump_error || water_lvl <= PUMP_TARGET)
    {
        digitalWrite(RELAY_PUMP_PIN, HIGH);
        is_pump_running_since_set = false;
    }
}

// checks button state, when pressed adds 1 to selected_tab variable
// When selected_tab = last tab -> sets variable to 1
void interface_choice()
{
    if (!digitalRead(INTERFACE_CHOICE_BTN))
    {
        selected_tab = selected_tab < 5 ? ++selected_tab : 1;
        while (!digitalRead(INTERFACE_CHOICE_BTN))
        {
            // do nothing while button pressed
        }
    }
}

// Checks what tab should be displayed and shows it
void display_interface(long elapsed_time)
{
    log_time += elapsed_time; // it's used for checking if any logs can be performed
    if (log_time >= log_interval)
    {
        switch (selected_tab)
        {
        case 1:
            display_water_temperature();
            break;
        case 2:
            display_water_level();
            break;
        case 3:
            display_time_and_date();
            break;
        case 4:
            display_combined_view();
            break;
        default:
            display_errors();
        }
        log_time -= log_interval;
    }
}

// checks temperature and displays it on a screen
void display_water_temperature()
{
    lcd.setCursor(0, 0);
    lcd.print("   WATER TEMP   ");

    sensors.requestTemperatures(); // Send the command to get temperatures
    float temperature = sensors.getTempCByIndex(0);

    if (temperature != DEVICE_DISCONNECTED_C)
    {
        lcd.setCursor(0, 1);
        lcd.print("    ");
        lcd.print(temperature);
        lcd.print((char)223);
        lcd.print("C   ");
    }
    else
    {
        lcd.setCursor(0, 1);
        lcd.print("  SENSOR ERROR  ");
        throw_error = (throw_error == "LVL SENSOR ERROR") ? "TEMP & LVL ERROR" : "TEMP SENSOR ERR ";
    }
}

// checks water level and displays it on a screen
void display_water_level()
{
    int water_lvl = check_water_level();

    lcd.setCursor(0, 0);
    lcd.print("  WATER LEVEL   ");

    lcd.setCursor(0, 1);
    // comparing readings from hc-sr04 with preseted values
    if (water_lvl >= NO_WATER)
        lcd.print("    NO WATER    ");
    else if (water_lvl >= LITTLE_OF_WATER)
        lcd.print("LITTLE OF WATER ");
    else if (water_lvl >= OPTIMUM_OF_WATER)
        lcd.print("OPTIMUM OF WATER");
    else
        lcd.print(" LOTS OF WATER  ");
}

void display_time_and_date()
{
    clock.updateTime();
    byte date[2] = {clock.dayofmonth, clock.month};
    byte time[3] = {clock.hours, clock.minutes, clock.seconds};
    lcd.setCursor(0, 0);
    for (byte value : date)
    {
        if (value < 10)
            lcd.print("0");
        lcd.print(value);
        lcd.print("/");
    }
    lcd.print(clock.year);
    lcd.print("      ");
    lcd.setCursor(0, 1);
    for (byte value : time)
    {
        if (value < 10)
            lcd.print("0");
        lcd.print(value);
        lcd.print(":");
    }
    lcd.setCursor(8, 1); // 8 cause we need to remove last ":" character from last iteration of line 242
    lcd.print("          ");
}

void display_combined_view()
{
    // time section
    clock.updateTime();
    byte time[2] = {clock.hours, clock.minutes};
    lcd.setCursor(0, 0);
    lcd.print(char(2));
    for (byte value : time)
    {
        if (value < 10)
            lcd.print("0");
        lcd.print(value);
        lcd.print(":");
    }
    lcd.setCursor(6, 0); // 6 cause we need to remove last ":" character from last iteration of line 257
    lcd.print("   ");

    // water level section
    int water_lvl = check_water_level();
    lcd.print(char(1));
    if (water_lvl >= NO_WATER)
        lcd.print("NONE  ");
    else if (water_lvl >= LITTLE_OF_WATER)
        lcd.print("LITTLE");
    else if (water_lvl >= OPTIMUM_OF_WATER)
        lcd.print("OPTIM ");
    else
        lcd.print("LOTS  ");

    // temperature section
    sensors.requestTemperatures();
    float temperature = sensors.getTempCByIndex(0);
    lcd.setCursor(0, 1);
    lcd.print(char(0));
    if (temperature != DEVICE_DISCONNECTED_C)
    {
        lcd.print(temperature);
        lcd.print((char)223);
        lcd.print("C ");
    }
    else
    {
        lcd.print("ERROR   ");
        throw_error = throw_error == "LVL SENSOR ERROR" ? "TEMP & LVL ERROR" : "TEMP SENSOR ERR ";
    }

    // error handeling section
    lcd.print(char(3));
    lcd.print(throw_error == "      NONE      " ? "NO " : "YES");
}

void display_errors()
{
    lcd.setCursor(0, 0);
    lcd.print("     ERRORS     ");
    lcd.setCursor(0, 1);
    lcd.print(throw_error);
}