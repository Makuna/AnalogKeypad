#include <AnalogKeypad.h>

const uint16_t KeypadHoldTimeMs = 5000;
const uint8_t KeypadAnalogPin = A0;

AnalogKeypad keypad(KeypadAnalogPin, KeypadHoldTimeMs);

// the button event callback
// this will be called when buttons are pressed and released
void ButtonHandler(const ButtonParam& param)
{
  switch (param.button)
  {
    case ButtonId_1:
    Serial.print("B1 ");
    break;
    case ButtonId_2:
    Serial.print("B2 ");
    break;
    case ButtonId_3:
    Serial.print("B3 ");
    break;
    case ButtonId_4:
    Serial.print("B4 ");
    break;
    case ButtonId_5:
    Serial.print("B5 ");
    break;
    default:
    break;
  };
  
  switch (param.state)
  {
    case ButtonState_Up:
    Serial.print("Up ");
    break;
    
    case ButtonState_Down:
    Serial.print("Down ");
    break;
    
    case ButtonState_Click:
    Serial.print("Click ");
    break;
    
    case ButtonState_DoubleClick:
    Serial.print("Double Click ");
    break;
    
    case ButtonState_Hold:
    Serial.print("Hold ");
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
}