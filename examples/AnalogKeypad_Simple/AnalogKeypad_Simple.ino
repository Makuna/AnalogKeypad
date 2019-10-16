#include <AnalogKeypad.h>

// the following table is for this Banggood Analog Keypad
// https://www.banggood.com/AD-Analog-Keyboard-Module-Electronic-Building-Blocks-5-Keys-For-Arduino-DIY-p-1374279.html
// see the AnalogKeypad_CreateConfig.ino sketch example on how to create this table
const int KeypadMap[] = {0, 152, 346, 536, 787};

const uint16_t KeypadHoldTimeMs = 5000;
const uint8_t KeypadAnalogPin = A0;

AnalogKeypad keypad(KeypadAnalogPin, KeypadMap, countof(KeypadMap), KeypadHoldTimeMs);

// the button event callback
// this will be called when buttons are pressed and released
void ButtonHandler(const ButtonParam& param)
{
  Serial.print(param.button);
  Serial.print(" ");
  
  switch (param.state)
  {
    case ButtonState_Up:
    Serial.print("Up");
    break;
    
    case ButtonState_Down:
    Serial.print("Down");
    break;
    
    case ButtonState_Click:
    Serial.print("Click");
    break;
    
    case ButtonState_DoubleClick:
    Serial.print("Double Click");
    break;
    
    case ButtonState_Hold:
    Serial.print("Hold");
    break;
  }
  
  Serial.println();
}

void setup() {
  
    Serial.begin(115200);
    while (!Serial); // wait for serial attach

    Serial.println();
    Serial.println("Initialized");
}

void loop() {
  
  keypad.loop(ButtonHandler);

  // simulate other work happening
  // it also should avoid long delays so loop above can be called
  // at least every 10-20ms
  delay(10);
}