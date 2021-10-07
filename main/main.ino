//temperature sensor config
#include <OneWire.h>
#include <DallasTemperature.h>

#define TEMPERATURE_BUS 2

OneWire oneWire(TEMPERATURE_BUS);
DallasTemperature sensors(&oneWire);

//water level sensor(s)
#define MAIN_WATER_LEVEL_SENSOR_TRIG 4
#define MAIN_WATER_LEVEL_SENSOR_ECHO 5

long duration;
int distance;

//rtc config
#include <virtuabotixRTC.h>
virtuabotixRTC myRTC(13, 12, 11);

//pump
#define RELAY_PUMP_PIN 6

//lcd display config
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

//control buttons
#define INTERFACE_CHOICE_BTN 3

//multitasking setup
const int log_interval = 1000;
long log_time = 0;
long last_time = millis();

//variable used, to define, which tab should be displayed on a screen
int selected_tab = 1;

//error handeling
String throw_error = "      BRAK      ";

//icons
const byte termometer_ico[8] = {B00100, B01010, B01010, B01110, B01110, B11111, B11111, B01110}; //icon for termometer
const byte water_ico[8] = {B00100, B00100, B01010, B01010, B10001, B10001, B10001, B01110};      //icon for water droplet
const byte clock_ico[8] = {B00000, B01110, B10101, B10111, B10001, B01110, B00000, B00000};      //icon for clock
const byte error_ico[8] = {B11111, B01110, B01110, B00100, B00000, B00100, B01110, B00100};      //icon for error info

void setup()
{
    //temperature sensor init
    sensors.begin();

    //water level sensor init
    pinMode(MAIN_WATER_LEVEL_SENSOR_TRIG, OUTPUT);
    pinMode(MAIN_WATER_LEVEL_SENSOR_ECHO, INPUT);

    //lcd display init
    lcd.init();
    lcd.backlight();

    //pump init
    pinMode(RELAY_PUMP_PIN, OUTPUT);
    digitalWrite(RELAY_PUMP_PIN, LOW);

    //interface button init
    pinMode(INTERFACE_CHOICE_BTN, INPUT_PULLUP);

    //create characters from icons
    lcd.createChar(0, termometer_ico);
    lcd.createChar(1, water_ico);
    lcd.createChar(2, clock_ico);
    lcd.createChar(3, error_ico);
}

void loop()
{
    long elapsed_time = millis() - last_time;
    last_time += elapsed_time;

    interface_choice();
    display_interface(elapsed_time);
    refill_water();
}

int check_water_level()
{
    digitalWrite(MAIN_WATER_LEVEL_SENSOR_TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(MAIN_WATER_LEVEL_SENSOR_TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(MAIN_WATER_LEVEL_SENSOR_TRIG, LOW);

    duration = pulseIn(MAIN_WATER_LEVEL_SENSOR_ECHO, HIGH);
    return duration / 58; //distance in cm
}

void refill_water()
{
    if (check_water_level() > 5)
        digitalWrite(RELAY_PUMP_PIN, HIGH);
    else
        digitalWrite(RELAY_PUMP_PIN, LOW);
}

//checks button state, when pressed adds 1 to selected_tab variable
//When selected_tab = last tab -> sets variable to 1
void interface_choice()
{

    if (!digitalRead(INTERFACE_CHOICE_BTN))
    {
        if (selected_tab < 5)
            selected_tab++;
        else
            selected_tab = 1;

        while (!digitalRead(INTERFACE_CHOICE_BTN))
        {
            //do nothing while button pressed
        }
    }
}

//Checks what tab should be displayed and shows it
void display_interface(long elapsed_time)
{
    log_time += elapsed_time; //it's used for checking if any logs can be performed
    if (log_time >= log_interval)
    {
        if (selected_tab == 1)
            display_water_temperature();
        else if (selected_tab == 2)
            display_water_level();
        else if (selected_tab == 3)
            display_time_and_date();
        else if (selected_tab == 4)
            display_combined_view();
        else
            display_errors();

        log_time -= log_interval;
    }
}

//checks temperature and displays it on a screen
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
        throw_error = "TEMP SENSOR ERR ";
    }
}

//checks water level and displays it on a screen
//values in an if statment should be changed, cause thay may differ in different enviroments
void display_water_level()
{
    int water_lvl = check_water_level();

    lcd.setCursor(0, 0);
    lcd.print("  WATER LEVEL   ");

    lcd.setCursor(0, 1);
    if (water_lvl >= 10)
        lcd.print("    NO WATER    ");
    else if (water_lvl >= 7)
        lcd.print("LITTLE OF WATER ");
    else if (water_lvl >= 5)
        lcd.print("OPTIMUM OF WATER");
    else
        lcd.print(" LOTS OF WATER  ");
}

void display_time_and_date()
{
    myRTC.updateTime();
    int date[2] = {myRTC.dayofmonth, myRTC.month};
    int time[3] = {myRTC.hours, myRTC.minutes, myRTC.seconds};
    lcd.setCursor(0, 0);
    for (int value : date)
    {
        if (value < 10)
            lcd.print("0");
        lcd.print(value);
        lcd.print("/");
    }
    lcd.print(myRTC.year);
    lcd.print("      ");
    lcd.setCursor(0, 1);
    for (int value : time)
    {
        if (value < 10)
            lcd.print("0");
        lcd.print(value);
        lcd.print(":");
    }
    lcd.setCursor(8, 1);
    lcd.print("          ");
}

void display_combined_view()
{
    //time section
    myRTC.updateTime();
    int time[2] = {myRTC.hours, myRTC.minutes};
    lcd.setCursor(0, 0);
    lcd.print(char(2));
    for (int value : time)
    {
        if (value < 10)
            lcd.print("0");
        lcd.print(value);
        lcd.print(":");
    }
    lcd.setCursor(6, 0);
    lcd.print("   ");

    //water level section
    int water_lvl = check_water_level();
    lcd.print(char(1));
    if (water_lvl >= 10)
        lcd.print("NONE  ");
    else if (water_lvl >= 7)
        lcd.print("LITTLE");
    else if (water_lvl >= 5)
        lcd.print("OPTIM ");
    else
        lcd.print("LOTS  ");

    //temperature section
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
        lcd.print("ERROR  ");
        throw_error = "BLAD SENSOR TEMP";
    }

    //error handeling section
    lcd.print(char(3));
    if (throw_error == "      BRAK      ")
        lcd.print("NO ");
    else
        lcd.print("YES");
}

void display_errors()
{
    lcd.setCursor(0, 0);
    lcd.print("     ERRORS     ");
    lcd.setCursor(0, 1);
    lcd.print(throw_error);
}