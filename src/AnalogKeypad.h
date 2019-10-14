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

#ifndef countof
#define countof(a) (sizeof(a) / sizeof(a[0]))
#endif // !countof

enum ButtonState
{
    ButtonState_Up,
    ButtonState_Down,
    ButtonState_Click,
    ButtonState_DoubleClick, // not implemented yet
    ButtonState_Hold,
};

// values below were measured from an esp8266
// 
const int c_ButtonAnalogValue_None = 1023;
const int c_ButtonAnalogValue_Error = 20; // normal measured drift * 10

const size_t c_AnalogReadAverageCount = 5;

const uint8_t ButtonId_None = 255;

struct ButtonParam
{
    ButtonState state;
    uint8_t button;
};

// define ButtonUpdateCallback to be a function like
// void MyCallBack(const ButtonParam& param) {}
typedef void(*ButtonUpdateCallback)(const ButtonParam& param);

class AnalogKeypad
{
public:
    AnalogKeypad(uint8_t pin, const int* analogTable, const size_t analogTableCount, uint16_t msHoldTime, uint16_t msClickTime = 33) :
        _analogTable(analogTable),
        _analogTableCount(analogTableCount),
        _msHoldTime(msHoldTime),
        _msClickTime(msClickTime),
        _pin(pin),
        _sumValues(0),
        _lastValue(c_ButtonAnalogValue_None),
        _analogValueError(c_ButtonAnalogValue_Error),
        _lastValueIndex(0)
    {
        // init the running average table
        for (uint8_t index = 0; index < c_AnalogReadAverageCount; index++)
        {
            _sumValues += c_ButtonAnalogValue_None;
            _lastValues[index] = c_ButtonAnalogValue_None;
        }

        // make sure error is less than half the delta between any two readings
        for (size_t index = 1; index < _analogTableCount; index++)
        {
            int deltaError = (_analogTable[index] - _analogTable[index - 1]) / 2;
            if (deltaError < _analogValueError)
            {
                _analogValueError = deltaError;
            }
        }
        
        _state.button = ButtonId_None;
        _state.state = ButtonState_Up;
    }

    void loop(ButtonUpdateCallback callback)
    {
        uint32_t sampleTime = millis();
        int sample = analogRead(_pin);

        // update running average sum with new sample
        _sumValues -= _lastValues[_lastValueIndex];
        _sumValues += sample;

        // update running average values with new sample
        _lastValues[_lastValueIndex] = sample;
        _lastValueIndex = (_lastValueIndex + 1) % c_AnalogReadAverageCount;

        int value = (_sumValues / c_AnalogReadAverageCount); // running average
        /* Usefull debug to determine button values
        Serial.print(sample);
        Serial.print("-");
        Serial.println(value);
        */
        // what button is currently thought to be pressed
        uint8_t newButton = valueToButtonId(value);

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
    const int* _analogTable;
    const size_t _analogTableCount;
    const uint32_t _msHoldTime;
    const uint32_t _msClickTime;
    const uint8_t _pin;

    uint32_t _downStartTime;
    int _lastValues[c_AnalogReadAverageCount];
    int _sumValues;
    int _lastValue;
    int _analogValueError;

    ButtonParam _state;
    uint8_t _lastValueIndex;
    

    uint8_t valueToButtonId(int value)
    {
        // check the common state of no buttons pressed
        if (value < _analogTable[_analogTableCount - 1] + _analogValueError)
        {
            // search the table for the button
            for (uint8_t button = 0; button < _analogTableCount; button++)
            {
                if (value < _analogTable[button] + _analogValueError)
                {
                    return button;
                }
            }
        }
        
        return ButtonId_None;
    }
};