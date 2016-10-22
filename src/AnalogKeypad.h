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

const int c_ButtonAnalogValue_None = 994;
const int c_ButtonAnalogValue_1 = 4; // left
const int c_ButtonAnalogValue_2 = 144; // up
const int c_ButtonAnalogValue_3 = 320; // down
const int c_ButtonAnalogValue_4 = 490; // right
const int c_ButtonAnalogValue_5 = 720; // action
const int c_ButtonAnalogValue_Error = 20;

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
    AnalogKeypad(uint8_t pin, uint16_t msHoldTime) :
        _pin(pin),
        _msHoldTime(msHoldTime),
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
        ButtonParam param;
        uint32_t sampleTime = millis();
        int sample = analogRead(_pin);
        int value = (_sumValues / countof(_lastValues)); // running average
        int delta = abs(sample - value);

        // update running average sum with new sample
        _sumValues -= _lastValues[_lastValueIndex];
        _sumValues += sample;

        // update running average values with new sample
        _lastValues[_lastValueIndex] = sample;
        _lastValueIndex = (_lastValueIndex + 1) % countof(_lastValues);

        // was there a change in value? 
        if (delta < 2 && abs(_lastValue - value) > c_ButtonAnalogValue_Error)
        {
            _lastValue = value;
            /*
            Serial.print(sample);
            Serial.print("-");
            Serial.print(value);
            Serial.print(" ");
            */
            if (_state.state != ButtonState_Up)
            {
                // notify release previous button
                param.button = _state.button;
                param.state = ButtonState_Up;
                callback(param);

                if (_state.state == ButtonState_Down)
                {
                    // notify click button
                    param.button = _state.button;
                    param.state = ButtonState_Click;
                    callback(param);
                }
            }

            _state.button = valueToButtonId(value);
            if (_state.button == ButtonId_None)
            {
                _state.state = ButtonState_Up;
            }
            else
            {
                _state.state = ButtonState_Down;

                // notify acquire new button
                param.button = _state.button;
                param.state = ButtonState_Down;
                callback(param);

                _holdStartTime = sampleTime;
            }
        }
        else
        {
            if (_state.state == ButtonState_Down &&
                (sampleTime - _holdStartTime) > _msHoldTime)
            {
                _state.state = ButtonState_Hold;

                // notify hold button
                param.button = _state.button;
                param.state = ButtonState_Hold;
                callback(param);
            }
        }

    }

private:
    const uint32_t _msHoldTime;
    const uint8_t _pin;

    uint32_t _holdStartTime;
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