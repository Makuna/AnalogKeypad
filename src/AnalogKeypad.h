/*--------------------------------------------------------------------
AnalogKeypad is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as
published by the Free Software Foundation, either version 3 of
the License, or (at your option) any later version.

AnalogKeypad is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

See GNU Lesser General Public License at <http://www.gnu.org/licenses/>.
--------------------------------------------------------------------*/

#pragma once 

#define countof(a) (sizeof(a) / sizeof(a[0]))

// values below were measured from an esp8266
// 
const int c_ButtonAnalogValue_None = 994;
const int c_ButtonAnalogValue_1 = 0; // left
const int c_ButtonAnalogValue_2 = 152; // up
const int c_ButtonAnalogValue_3 = 346; // down
const int c_ButtonAnalogValue_4 = 536; // right
const int c_ButtonAnalogValue_5 = 787; // action
const int c_ButtonAnalogValue_Error = 20; // normal measured drift * 10; but must remain less that half distance between any two values above

enum ButtonId
{
    ButtonId_None,
    ButtonId_1,
    ButtonId_2,
    ButtonId_3,
    ButtonId_4,
    ButtonId_5
};

enum ButtonState
{
    ButtonState_Up,
    ButtonState_Down,
    ButtonState_Click,
    ButtonState_DoubleClick, // not implemented yet
    ButtonState_Hold,
};

struct ButtonParam
{
    ButtonId button;
    ButtonState state;
};

// define ButtonUpdateCallback to be a function like
// void MyCallBack(const ButtonParam& param) {}
typedef void(*ButtonUpdateCallback)(const ButtonParam& param);

class AnalogKeypad
{
public:
    AnalogKeypad(uint8_t pin, uint16_t msHoldTime, uint16_t msClickTime = 33) :
        _pin(pin),
        _msHoldTime(msHoldTime),
        _msClickTime(msClickTime),
        _lastValueIndex(0),
        _sumValues(0)
    {
        for (uint8_t index = 0; index < countof(_lastValues); index++)
        {
            _sumValues += c_ButtonAnalogValue_None;
            _lastValues[index] = c_ButtonAnalogValue_None;
        }
        _state.button = ButtonId_None;
        _state.state = ButtonState_Up;
    }

    bool loop(ButtonUpdateCallback callback)
    {
        uint32_t sampleTime = millis();
        int sample = analogRead(_pin);

        // update running average sum with new sample
        _sumValues -= _lastValues[_lastValueIndex];
        _sumValues += sample;

        // update running average values with new sample
        _lastValues[_lastValueIndex] = sample;
        _lastValueIndex = (_lastValueIndex + 1) % countof(_lastValues);

        int value = (_sumValues / countof(_lastValues)); // running average
        /* Usefull debug to determine button values
        Serial.print(sample);
        Serial.print("-");
        Serial.println(value);
        */
        // what button is currently thought to be pressed
        ButtonId newButton = valueToButtonId(value);

        int delta = abs(sample - value);

        // was there a change in value? 
        if (delta < 2 && 
            abs(_lastValue - value) > c_ButtonAnalogValue_Error &&
            newButton != _state.button)
        {
            _lastValue = value;

            // check previous state for notifications needed to
            // close out previous button handling
            //
            if (_state.state != ButtonState_Up)
            {
                ButtonParam param;

                // notify release previous button
                param.button = _state.button;
                param.state = ButtonState_Up;
                callback(param);

                // hold and click are exclusive, 
                // so only trigger click if currently down
                if (_state.state == ButtonState_Down) 
                {
                    // check debounce time
                    if ((sampleTime - _downStartTime) > _msClickTime)
                    {
                        // notify click button
                        param.button = _state.button;
                        param.state = ButtonState_Click;
                        callback(param);
                    }
                }
            }

            // apply new button state
            //
            _state.button = newButton;

            if (_state.button == ButtonId_None)
            {
                _state.state = ButtonState_Up;
            }
            else
            {
                _state.state = ButtonState_Down;

                // notify acquired new button
                callback(_state);

                _downStartTime = sampleTime;
            }
        }
        else
        {
            // no state change, update timers
            //
            if (_state.state == ButtonState_Down)
            {
                if ((sampleTime - _downStartTime) > _msHoldTime)
                {
                    _state.state = ButtonState_Hold;

                    // notify hold button
                    callback(_state);
                }
            }
        }

    }

private:
    const uint32_t _msHoldTime;
    const uint32_t _msClickTime;
    const uint8_t _pin;

    uint32_t _downStartTime;
    int _lastValues[5];
    int _sumValues;
    int _lastValue;

    ButtonParam _state;
    uint8_t _lastValueIndex;
    

    ButtonId valueToButtonId(int value)
    {
        if (value < c_ButtonAnalogValue_1 + c_ButtonAnalogValue_Error)
        {
            return ButtonId_1;
        }
        else if (value < c_ButtonAnalogValue_2 + c_ButtonAnalogValue_Error)
        {
            return ButtonId_2;
        }
        else if (value < c_ButtonAnalogValue_3 + c_ButtonAnalogValue_Error)
        {
            return ButtonId_3;
        }
        else if (value < c_ButtonAnalogValue_4 + c_ButtonAnalogValue_Error)
        {
            return ButtonId_4;
        }
        else if (value < c_ButtonAnalogValue_5 + c_ButtonAnalogValue_Error)
        {
            return ButtonId_5;
        }
        else
        {
            return ButtonId_None;
        }
    }
};