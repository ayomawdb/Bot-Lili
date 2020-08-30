#include <Arduino.h>
#include <NewPing.h>
#include <Wire.h>

const int SERIAL_BOUND_RATE = 9600;

const int WIRE_USOUND_ADDRESS = 9;

const int USOUND_CLEAR = 0;
const int USOUND_FRONT_BLOCKED = 1;
const int USOUND_BACK_BLOCKED = 2;

#define SONAR_NUM 2
#define MAX_DISTANCE 200

NewPing sonarFront[SONAR_NUM] = {
    NewPing(4, 5),
    NewPing(6, 7),
};

NewPing sonarBack[SONAR_NUM] = {
    NewPing(8, 9),
    NewPing(10, 11)};

void setup()
{
    pinMode(4, OUTPUT);
    pinMode(5, INPUT);
    pinMode(6, OUTPUT);
    pinMode(7, INPUT);
    pinMode(8, OUTPUT);
    pinMode(9, INPUT);
    pinMode(10, OUTPUT);
    pinMode(11, INPUT);
    Wire.begin();
}

void loop()
{
    Serial.begin(SERIAL_BOUND_RATE);
    boolean allClear = true;
    for (uint8_t i = 0; i < SONAR_NUM; i++)
    {
        delay(50);
        if (sonarFront[i].ping_cm() < 15 && sonarFront[i].ping_cm() > 0)
        {
            Wire.beginTransmission(WIRE_USOUND_ADDRESS);
            Wire.write(USOUND_FRONT_BLOCKED);
            Wire.endTransmission();
            Serial.println("Front Blocked");
            allClear = false;
        }
    }
    for (uint8_t i = 0; i < SONAR_NUM; i++)
    {
        delay(50);
        if (sonarBack[i].ping_cm() < 15 && sonarBack[i].ping_cm() > 0)
        {
            Wire.beginTransmission(WIRE_USOUND_ADDRESS);
            Wire.write(USOUND_BACK_BLOCKED);
            Wire.endTransmission();
            Serial.println("Back Blocked");
            allClear = false;
        }
    }
    if (allClear)
    {
        Wire.beginTransmission(WIRE_USOUND_ADDRESS);
        Wire.write(USOUND_CLEAR);
        Wire.endTransmission();
        Serial.println("Clear");
    }
    delay(50);
}