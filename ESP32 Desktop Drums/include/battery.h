#pragma once

#include <Arduino.h>
#include "def.h"

#define ADC_VREF 3.28
#define BAT_VOLT_DIVIDER 0.6357
#define BAT_READ_TIME_GAP 10000 // 10s
#define BAT_LOW 3.1
#define BAT_FULL 4.20
#define BAT_EMPTY 2.85

class Battery {
    public:
        void read();
        uint8_t getPercentage();
        float getVoltage();
        bool isLow();

    private:
        float _voltage = 0.0;
        bool _isLow = false;
        uint32_t _lastReadTime = 0;
};

extern Battery battery;