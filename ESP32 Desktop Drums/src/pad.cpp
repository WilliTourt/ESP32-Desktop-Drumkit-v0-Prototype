#include "pad.h"
#include "def.h"

Pad::Pad(uint8_t piezo_pin, uint8_t out_pin, uint16_t hit_threshold, uint16_t upper_limit)
    : _piezo_pin(piezo_pin),
      _out_pin(out_pin),
      _force(0),
      _last_trigger_state(false),
      _peak_val(0),
      _adc_reading(false),
      _adc_start_time(0),
      _hit_threshold(hit_threshold),
      _upper_limit(upper_limit) {
    pinMode(_piezo_pin, INPUT);
    pinMode(_out_pin, OUTPUT);
}

uint8_t Pad::getForce() {
    return _force;
}

void Pad::setOut(bool state) {
    digitalWrite(_out_pin, state);
}

void Pad::doADCRead() {
    digitalWrite(DEBUG_PIN, HIGH);
    analogRead(_piezo_pin);
    digitalWrite(DEBUG_PIN, LOW);
}

void Pad::updateForce() {
    bool current_hit = _isHit();
    
    if (current_hit && !_adc_reading) {
        _adc_reading = true;
        _adc_start_time = millis();
        _peak_val = 0;
    }

    if (_adc_reading) {
        uint16_t val = _getVal();

        // Skip readings in peak area
        if (val < 3750) {
            _peak_val = max(_peak_val, val);
        }

        if (!current_hit && ((millis() - _adc_start_time) > ADC_READING_WINDOW_MS)) {
            if (_peak_val >= _upper_limit) {
                _force = 127;
            } else {
                _force = map(_peak_val, _hit_threshold, _upper_limit, 1, 127);
            }
            _peak_val = 0;
            _adc_reading = false;
        }
    }
    
    _last_trigger_state = current_hit;
}

bool Pad::isTriggered() {
    bool current_hit = _isHit();
    bool triggered = (current_hit && !_last_trigger_state);
    _last_trigger_state = current_hit;
    return triggered;
}

inline bool Pad::_isHit() {
    return (_getVal() > _hit_threshold);
}

inline uint16_t Pad::_getVal() {
    return analogRead(_piezo_pin);
}