#include <Arduino.h>
#include <AFMotor.h>
#include <Wire.h>

AF_DCMotor motor1(1);
AF_DCMotor motor2(2);
AF_DCMotor motor3(3);
AF_DCMotor motor4(4);

const int SERIAL_BOUND_RATE = 9600;
const int SERIAL_TIMEOUT = 1000;

const int WIRE_TIMEOUT = 1000;
const int WIRE_USOUND_ADDRESS = 9;

const int USOUND_CLEAR = 0;
const int USOUND_FRONT_BLOCKED = 1;
const int USOUND_BACK_BLOCKED = 2;

const String command_forward = "FW ";
const String command_backward = "BK ";
const String command_stop = "ST";
const String command_brake = "BR";
const String command_normal_left = "NL";
const String command_normal_right = "NR";
const String command_hard_left = "HL";
const String command_hard_right = "HR";

const byte DIRECTION_STOP = 0;
const byte DIRECTION_FORWARD = 1;
const byte DIRECTION_BACKWARD = 2;

const byte SPEED_MIN = 150;
const byte SPEED_MAX = 255;

const byte DELAY_SPEED_ADJUSTMENT = 10;
const byte DELAY_BREAK_DIRECTION_CHANGE = 15;
const byte DELAY_BREAK_SPEED_ADJUSTMENT = 1;

const byte FUNCTION_NORMAL = 0;
const byte FUNCTION_ESCAPE = 1;

byte direction = DIRECTION_STOP;
byte speed = 0;
byte function = FUNCTION_NORMAL;

void setAllSpeed(byte speed)
{
    motor1.setSpeed(speed);
    motor2.setSpeed(speed);
    motor3.setSpeed(speed);
    motor4.setSpeed(speed);
}

void setAllStatus(byte state)
{
    motor1.run(state);
    motor2.run(state);
    motor3.run(state);
    motor4.run(state);
}

void matchTargetSpeed(byte targetSpeed, byte delayTime)
{
    if (targetSpeed < SPEED_MIN)
    {
        targetSpeed = SPEED_MIN;
    }
    if (speed > targetSpeed)
    {
        for (; speed > targetSpeed; speed--)
        {
            setAllSpeed(speed);
            delay(delayTime);
        }
    }
    else if (speed < targetSpeed)
    {
        for (; speed < targetSpeed; speed++)
        {
            setAllSpeed(speed);
            delay(delayTime);
        }
    }
}

void processCommand(String incomingString)
{
    if (incomingString.equals(command_brake))
    {
        if (direction == DIRECTION_BACKWARD)
        {
            setAllStatus(FORWARD);
        }
        else if (direction == DIRECTION_FORWARD)
        {
            setAllStatus(BACKWARD);
        }
        delay(DELAY_BREAK_DIRECTION_CHANGE);
        matchTargetSpeed(SPEED_MIN, DELAY_BREAK_SPEED_ADJUSTMENT);
        setAllStatus(RELEASE);
        direction = DIRECTION_STOP;
    }
    else if (incomingString.equals(command_stop))
    {
        matchTargetSpeed(SPEED_MIN, DELAY_SPEED_ADJUSTMENT);
        direction = DIRECTION_STOP;
        setAllStatus(RELEASE);
    }
    else if (incomingString.startsWith(command_forward))
    {
        if (direction == DIRECTION_BACKWARD)
        {
            // Slow down and then change the direction.
            matchTargetSpeed(SPEED_MIN, DELAY_SPEED_ADJUSTMENT);
            direction = DIRECTION_FORWARD;
            setAllStatus(FORWARD);
            byte targetSpeed = incomingString.substring(incomingString.indexOf(" ")).toInt();
            matchTargetSpeed(targetSpeed, DELAY_SPEED_ADJUSTMENT);
        }
        else
        {
            // Just adjust the speed or start moving if stopped.
            byte targetSpeed = incomingString.substring(incomingString.indexOf(" ")).toInt();
            matchTargetSpeed(targetSpeed, DELAY_SPEED_ADJUSTMENT);
            direction = DIRECTION_FORWARD;
            setAllStatus(FORWARD);
        }
    }
    else if (incomingString.startsWith(command_backward))
    {
        if (direction == DIRECTION_FORWARD)
        {
            // Slow down and then change the direction.
            matchTargetSpeed(SPEED_MIN, DELAY_SPEED_ADJUSTMENT);
            direction = DIRECTION_BACKWARD;
            setAllStatus(BACKWARD);
            byte targetSpeed = incomingString.substring(incomingString.indexOf(" ")).toInt();
            matchTargetSpeed(targetSpeed, DELAY_SPEED_ADJUSTMENT);
        }
        else
        {
            // Just adjust the speed or start moving if stopped.
            byte targetSpeed = incomingString.substring(incomingString.indexOf(" ")).toInt();
            matchTargetSpeed(targetSpeed, DELAY_SPEED_ADJUSTMENT);
            direction = DIRECTION_BACKWARD;
            setAllStatus(BACKWARD);
        }
    }
    else if (incomingString.startsWith(command_normal_left))
    {
        motor2.run(RELEASE);
        motor3.run(RELEASE);
        if (direction == DIRECTION_BACKWARD)
        {
            motor1.run(BACKWARD);
            motor4.run(BACKWARD);
        }
        else
        {
            motor1.run(FORWARD);
            motor4.run(FORWARD);
        }
    }
    else if (incomingString.startsWith(command_normal_right))
    {
        motor1.run(RELEASE);
        motor4.run(RELEASE);
        if (direction == DIRECTION_BACKWARD)
        {
            motor2.run(BACKWARD);
            motor3.run(BACKWARD);
        }
        else
        {
            motor2.run(FORWARD);
            motor3.run(FORWARD);
        }
    }
    else if (incomingString.startsWith(command_hard_left))
    {
        if (direction == DIRECTION_BACKWARD)
        {
            motor1.run(BACKWARD);
            motor4.run(BACKWARD);
            motor2.run(FORWARD);
            motor3.run(FORWARD);
        }
        else
        {
            motor1.run(FORWARD);
            motor4.run(FORWARD);
            motor2.run(BACKWARD);
            motor3.run(BACKWARD);
        }
    }
    else if (incomingString.startsWith(command_hard_right))
    {
        if (direction == DIRECTION_BACKWARD)
        {
            motor1.run(FORWARD);
            motor4.run(FORWARD);
            motor2.run(BACKWARD);
            motor3.run(BACKWARD);
        }
        else
        {
            motor1.run(BACKWARD);
            motor4.run(BACKWARD);
            motor2.run(FORWARD);
            motor3.run(FORWARD);
        }
    }
}

void receiveEvent(int bytes)
{
    int incomingData = Wire.read();
    if (incomingData == USOUND_FRONT_BLOCKED && direction == DIRECTION_FORWARD)
    {
        processCommand("BR");
        function = FUNCTION_ESCAPE;
        processCommand("HR");
    }
    if (incomingData == USOUND_BACK_BLOCKED && direction == DIRECTION_BACKWARD)
    {
        processCommand("BR");
        function = FUNCTION_ESCAPE;
        processCommand("HR");
    }
    if (incomingData == USOUND_CLEAR && function == FUNCTION_ESCAPE)
    {
        if (direction == DIRECTION_FORWARD)
        {
            processCommand("FW 250");
        }
        else if (direction == DIRECTION_BACKWARD)
        {
            processCommand("BK 250");
        }
        function = FUNCTION_NORMAL;
    }
}

void setup()
{
    Serial.begin(SERIAL_BOUND_RATE);
    Serial.setTimeout(SERIAL_TIMEOUT);

    setAllSpeed(SPEED_MIN);
    setAllStatus(RELEASE);

    Wire.begin(WIRE_USOUND_ADDRESS);
    Wire.onReceive(receiveEvent);
    Wire.setTimeout(WIRE_TIMEOUT);
}

void loop()
{
    if (Serial.available() > 0)
    {
        String incomingString = Serial.readString();
        processCommand(incomingString);
    }

    processCommand("FW 250");
    delay(3 * 1000);

    processCommand("BK 250");
    delay(3 * 1000);
    //    processCommand("BK 150");
    //    delay(3 * 1000);
    //    processCommand("FW 255");
    //    delay(3 * 1000);
    //    processCommand("BK 255");
    //    delay(3 * 1000);

    //    processCommand("FW 200");
    processCommand("HR");
    delay(3 * 1000);
    //    processCommand("BR");
    //    delay(5 * 1000);
}
