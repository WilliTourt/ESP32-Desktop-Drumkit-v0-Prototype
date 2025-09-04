#pragma once

#include <Arduino.h>

/* MIDI NOTE CODES ------------------------------------------
    General MIDI (GM) Percussion Key Map
    https://musescore.org/sites/musescore.org/files/General%20MIDI%20Standard%20Percussion%20Set%20Key%20Map.pdf

    35 - Acoustic Bass Drum
    36 - Bass Drum 1
    37 - Side Stick
    38 - Acoustic Snare         L1
    39 - Hand Clap
    40 - Electric Snare
    41 - Low Floor Tom
    42 - Closed Hi-Hat
    43 - High Floor Tom
    44 - Pedal Hi-Hat
    45 - Low Tom                R1
    46 - Open Hi-Hat            L3
    47 - Low-Mid Tom            R2
    48 - Hi-Mid Tom
    49 - Crash Cymbal 1         R4
    50 - High Tom               L2
    51 - Ride Cymbal 1          R3
    52 - Chinese Cymbal
    53 - Ride Bell
    54 - Tambourine
    55 - Splash Cymbal
    56 - Cowbell
    57 - Crash Cymbal 2
    58 - Vibraslap
    59 - Ride Cymbal 2
    60 - Hi Bongo
    61 - Low Bongo
    62 - Mute Hi Conga
    63 - Open Hi Conga
    64 - Low Conga
    65 - High Timbale
    66 - Low Timbale
    67 - High Agogo
    68 - Low Agogo
    69 - Cabasa
    70 - Maracas
    71 - Short Whistle
    72 - Long Whistle
    73 - Short Guiro
    74 - Long Guiro
    75 - Claves
    76 - Hi Wood Block
    77 - Low Wood Block
    78 - Mute Cuica
    79 - Open Cuica
    80 - Mute Triangle
    81 - Open Triangle
    82 - Shaker
---------------------------------------------------------- */

// Notes used in the drumkit
// 38 - Acoustic Snare         L1
// 50 - High Tom               L2
// 46 - Open Hi-Hat            L3
// 45 - Low Tom                R1
// 47 - Low-Mid Tom            R2
// 51 - Ride Cymbal 1          R3
// 49 - Crash Cymbal 1         R4

#define ACOUSTIC_SNARE     38
#define LOW_TOM            45
#define OPEN_HI_HAT        46
#define LOW_MID_TOM        47
#define CRASH_CYMBAL_1     49
#define HIGH_TOM           50
#define RIDE_CYMBAL_1      51

#define ADC_PAD_HIT_DEFAULT_THRESHOLD 300
#define ADC_PAD_DEFAULT_UPPER_LIMIT 4095
#define ADC_READING_WINDOW_MS 12

#define NOTE_OFF_DELAY_MS 20

typedef struct {
    bool noteOn_sent;
    uint32_t noteOn_timestamp;
    uint8_t note;
    uint8_t channel;
} NoteOff_Flag_t;

class Pad {
    public:
        Pad(uint8_t piezo_pin, uint8_t out_pin, uint16_t hit_threshold = ADC_PAD_HIT_DEFAULT_THRESHOLD, uint16_t upper_limit = ADC_PAD_DEFAULT_UPPER_LIMIT);
        uint8_t getForce();
        void updateForce();
        bool isTriggered();
        uint8_t getVelocity() { return getForce(); }

        void setOut(bool state);

        // For Debugging
        void doADCRead();

    private:
        uint8_t _piezo_pin, _out_pin;
        uint8_t _force;
        bool _last_trigger_state;

        // For solving INTERFERENCE
        uint16_t _hit_threshold;
        uint16_t _upper_limit;

        // For adc reading window
        inline bool _isHit();
        inline uint16_t _getVal();
        bool _adc_reading;
        uint32_t _adc_start_time;
        uint16_t _peak_val;
};
