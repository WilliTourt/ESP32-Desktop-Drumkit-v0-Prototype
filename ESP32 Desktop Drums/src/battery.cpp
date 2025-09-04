#include "battery.h"

void Battery::read() {
    if (_lastReadTime == 0) {
        _lastReadTime = millis();
    } else if (millis() - _lastReadTime > BAT_READ_TIME_GAP) {
        uint16_t adcValue = analogRead(ADC_PIN);
        float adcVoltage = (float)adcValue * ADC_VREF / 4096.0;
        _voltage = adcVoltage / BAT_VOLT_DIVIDER;

        _isLow = (_voltage < BAT_LOW);
        _lastReadTime = 0;
    }
}

uint8_t Battery::getPercentage() {
    uint8_t percentage = (_voltage - BAT_EMPTY) / (BAT_FULL - BAT_EMPTY) * 100;
    return ((percentage > 100) ? 100 : percentage);
}

float Battery::getVoltage() {
    return _voltage;
}
bool Battery::isLow() {
     return _isLow;
}

Battery battery;
