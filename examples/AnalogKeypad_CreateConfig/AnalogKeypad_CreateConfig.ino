// AnalogKeypad_CreateConfig.ino
//
// This sketch will monitor the analog pin and create a map array that can be copied from
// the serial monitor into your sketch; allowing for easier setup of arbitrary analog keypads
//
// Just run it and follow the instructions in the serial monitor
// The line to copy will be very similiar to the following.  Review the AnalogKeypad_Simple 
// example sketch on how to apply it.
//
// const int KeypadMap[] = {0, 152, 346, 536, 787, 1024};
// 
//

#include <AnalogKeypad.h>

const uint8_t c_KeypadAnalogPin = A0; // make sure to set this to the correct pin
const int c_QualityWarningLevel = 4;

struct MapItem 
{
    int value;
    MapItem* prev;
    MapItem* next;
};

MapItem* g_valueMap = nullptr;
int g_analogValueNoPress; // should be 1024, but on some its 4095, like ESP32
int g_valueEpsilon = 0;
int g_lastSample;

bool readLevel(int* level, int* quality) 
{
    int lastValues[5];
    int sumValues = 0;
    uint8_t lastValueIndex = 0;

    // init the running average table
    for (uint8_t index = 0; index < countof(lastValues); index++) 
    {
        sumValues += g_analogValueNoPress;
        lastValues[index] = g_analogValueNoPress;
    }

    int average = 0;

    for (int deltaTrigger = 1; deltaTrigger < 10; deltaTrigger++) 
    {
        for (uint8_t deltaTests = 0; deltaTests < 50; deltaTests++) 
        {
            int sample = analogRead(c_KeypadAnalogPin);

            // update running average sum with new sample
            sumValues -= lastValues[lastValueIndex];
            sumValues += sample;

            // update running average values with new sample
            lastValues[lastValueIndex] = sample;
            lastValueIndex = (lastValueIndex + 1) % countof(lastValues);

            average = (sumValues / countof(lastValues));
            int delta = abs(sample - average);
            if (delta < deltaTrigger) 
            {
                *level = average;
                *quality = deltaTrigger;
                return true;
            }
            delay(20);
        }
    }

    *level = average;
    *quality = 0;
    return false;
}

void includeValue(int value) 
{
    // ignore lowest bit value due to value source being analogRead
    // and the values returned often flutter by this bit
    if ((value % 2) == 1)
    {
        value--;
    }

    if (g_valueMap == nullptr)
    {
        g_valueMap = new MapItem{ value, nullptr, nullptr };
    }
    else
    {
        MapItem* item = g_valueMap;
        MapItem* newItem;

        while (item)
        {
            if (abs(item->value - value) < g_valueEpsilon)
            {
                return; // already present
            }
            if (item->value > value)
            {
                // insert before
                newItem = new MapItem{ value, item->prev, item };
                if (item->prev == nullptr)
                {
                    g_valueMap = newItem;
                }
                else
                {
                    item->prev->next = newItem;
                }
                item->prev = newItem;
                return;
            }
            item = item->next;
        }
    }
}

// this will print a line similiar to this...
// const int KeypadMap[] = {0, 152, 346, 536, 787, 1024};
//
void printMap() 
{
    Serial.print("const int KeypadMap[] = {");

    MapItem* item = g_valueMap;
    bool first = true;
    while (item)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            Serial.print(", ");
        }

        Serial.print(item->value);
        item = item->next;
    }

    Serial.println("};");
}

void updateEpsilon(int quality) 
{
    // update the value epsilon to reduce near same values being logged
    if (quality > g_valueEpsilon) 
    {
        g_valueEpsilon = quality;
    }
}

void printStabilityError() 
{
    Serial.println("ANALOG READ NOT STABLE ENOUGH!");
    Serial.println("Please confirm your wiring and restart sketch");
}

void printStabilityWarning() 
{
    Serial.println("The quality of the analog readings are poor and you may see erratic results.");
}

void printInvalidNoPressError()
{
    Serial.println("THE LEVEL OF THE ANALOG READINGS WITH NO BUTTON PRESSED ARE TOO LOW!");
    Serial.println("Please confirm your wiring and restart sketch");
}

void setup() 
{
    Serial.begin(115200);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println();
    Serial.println();
    Serial.println("Initializing...");

    int quality;
    if (!readLevel(&g_analogValueNoPress, &quality)) 
    {
        printStabilityError();
    }
    if (g_analogValueNoPress < 256)
    {
        printInvalidNoPressError();
    }
    if (quality > c_QualityWarningLevel) 
    {
        printStabilityWarning();
    }

    updateEpsilon(g_analogValueNoPress / 128); // the greater number of bits, the greater the epsilon

    Serial.println();
    Serial.print("none pressed value : ");
    Serial.println(g_analogValueNoPress);
    Serial.print("quality : ");
    Serial.println(quality);
    Serial.println("Initialized");

    includeValue(g_analogValueNoPress);

    g_lastSample = g_analogValueNoPress;

    Serial.println();
    Serial.println();
    Serial.println("Press one button at a time, hold it until instructed to release");
    Serial.println();
}

void loop() 
{
    int sample = analogRead(c_KeypadAnalogPin);

    if (abs(sample - g_lastSample) > 10) 
    {
        int value;
        int quality;

        Serial.println("-sampling, please wait...");

        if (!readLevel(&value, &quality)) 
        {
            printStabilityError();
        }
        else 
        {
            if (quality > c_QualityWarningLevel)
            {
                printStabilityWarning();
            }

            updateEpsilon(quality);
            includeValue(value);
        }

        Serial.println("-please release the button-");

        // wait for button release
        while (abs(sample - g_analogValueNoPress) > g_valueEpsilon) 
        {
            delay(10);
            sample = analogRead(c_KeypadAnalogPin);
        }

        printMap();
    }
    g_lastSample = sample;
    delay(50);
}