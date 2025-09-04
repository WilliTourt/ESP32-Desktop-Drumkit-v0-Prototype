#include <Arduino.h>
#include <MIDI.h>
#include "BLEMIDI_Transport.h"
#include "hardware/BLEMIDI_ESP32.h"

// #include "UsbMidi.h"
// ALL COMMENTED LINES BELOW ARE USBMIDI RELATED CODE, 
// I switched to PLAN B: 
// Not using UsbMidi library, but using USART with a bridge program

#include "OneButtonTiny.h"
#include "battery.h"
#include "pad.h"
#include "def.h"

enum class MIDI_MODE { USBMIDI, BLEMIDI, SERIALMIDI };
MIDI_MODE current_mode = MIDI_MODE::BLEMIDI;
// bool is_usb_connected = false;
bool is_ble_connected = false;

bool setup_done = false;
bool mode_chng = false;
bool pads_need_update = false;
bool battery_update_flag = false;
bool blinker = false;

NoteOff_Flag_t NoteOffFlags[7] = {};

#define PAD_SILENCE_MS 10
uint32_t pad_silence_until = 0;

/* Notes used in the drumkit
	 L3  L2 	R2	R3
	   L1		  R1  R4
*/
// 38 - Acoustic Snare         L1
// 50 - High Tom               L2
// 46 - Open Hi-Hat            L3
// 45 - Low Tom                R1
// 47 - Low-Mid Tom            R2
// 51 - Ride Cymbal 1          R3
// 49 - Crash Cymbal 1         R4

const uint8_t PadType[7] = {
	ACOUSTIC_SNARE,
	HIGH_TOM      ,
	OPEN_HI_HAT   ,
	LOW_TOM       ,
	LOW_MID_TOM   ,
	CRASH_CYMBAL_1,
	RIDE_CYMBAL_1
};

#define DRUMPAD_THRESHOLD 220
#define CYMBAL_THRESHOLD 110
#define DRUMPAD_UPPER_THRESHOLD 3072
#define CYMBAL_UPPER_THRESHOLD 1024

// INST declarations ------------------------------------------------------//
Pad	snare_l1 (PIEZO_L1_PIN , OUT_L1_PIN , DRUMPAD_THRESHOLD, DRUMPAD_UPPER_THRESHOLD); // L1
Pad	  tom_l2 (PIEZO_L2_PIN , OUT_L2_PIN , DRUMPAD_THRESHOLD, DRUMPAD_UPPER_THRESHOLD); // L2
Pad	crash_l3 (PIEZO_L3_PIN , OUT_L3_PIN , CYMBAL_THRESHOLD , CYMBAL_UPPER_THRESHOLD ); // L3
Pad	  tom_r1 (PIEZO_R1_PIN , OUT_R1_PIN , DRUMPAD_THRESHOLD, DRUMPAD_UPPER_THRESHOLD); // R1
Pad	  tom_r2 (PIEZO_R2_PIN , OUT_R2_PIN , DRUMPAD_THRESHOLD, DRUMPAD_UPPER_THRESHOLD); // R2
Pad	crash_r3 (PIEZO_R3_PIN , OUT_EX1_PIN, CYMBAL_THRESHOLD , CYMBAL_UPPER_THRESHOLD ); // R3
Pad	 ride_r4 (PIEZO_EX1_PIN, OUT_R3_PIN , CYMBAL_THRESHOLD , CYMBAL_UPPER_THRESHOLD ); // R4

BLEMIDI_CREATE_INSTANCE("ESP32 Desktop DrumKit v0", MIDI)
// UsbMidi USBMIDI;
OneButtonTiny button(S_PIN, true, true);
// ------------------------------------------------------------------------//

// Function Prototypes ----------------------------------------------------//
void IRAM_ATTR PADs_Update_ISR();
void IRAM_ATTR BAT_Update_ISR();

void Blink() {
	digitalWrite(LED_PIN, HIGH);
	delay(60);
	digitalWrite(LED_PIN, LOW);
}

void Beep() {
	digitalWrite(BUZZER_PIN, HIGH);
	delay(60);
	digitalWrite(BUZZER_PIN, LOW);
}

// https://midi.org/expanded-midi-1-0-messages-list
void Serial_SendMIDI(uint8_t note, uint8_t velocity, uint8_t channel = 10) {
    Serial.write(0x90 | ((channel - 1) & 0x0F));
    Serial.write(note & 0x7F);
    Serial.write(velocity & 0x7F);
}

void PAD_SendNoteOff(uint8_t i) {
	if (NoteOffFlags[i].noteOn_sent && (millis() - NoteOffFlags[i].noteOn_timestamp > NOTE_OFF_DELAY_MS)) {
		if (current_mode == MIDI_MODE::BLEMIDI && is_ble_connected) {
			MIDI.sendNoteOn(NoteOffFlags[i].note, 0, NoteOffFlags[i].channel);
		} else if (current_mode == MIDI_MODE::SERIALMIDI) {
			Serial.write(0x80 | ((NoteOffFlags[i].channel - 1) & 0x0F));
			Serial.write(NoteOffFlags[i].note & 0x7F);
			Serial.write(0x00);
		}
		NoteOffFlags[i].noteOn_sent = false;
	}
}

void Callback_BLE_Connected()    { is_ble_connected = true; }
void Callback_BLE_Disconnected() { is_ble_connected = false; }
void Callback_NoteOn(byte channel, byte note, byte velocity) {}
// void Callback_USB_Connected()    { is_usb_connected = true; }
// void Callback_USB_Disconnected() { is_usb_connected = false; }

void Callback_ButtonSingleClick() {
	if (current_mode == MIDI_MODE::BLEMIDI) {
		// current_mode = MIDI_MODE::USBMIDI;
		current_mode = MIDI_MODE::SERIALMIDI;
		Beep();
	} else {
		current_mode = MIDI_MODE::BLEMIDI;
		Beep();
		delay(100);
		Beep();
	}
	mode_chng = true;
}
// ------------------------------------------------------------------------//


void setup() {
	
	// setCpuFrequencyMhz(240);
	pinMode(DEBUG_PIN, OUTPUT);

	pinMode(LED_PIN, OUTPUT);
	pinMode(BUZZER_PIN, OUTPUT);

	BLEMIDI.setHandleConnected(Callback_BLE_Connected);
	BLEMIDI.setHandleDisconnected(Callback_BLE_Disconnected);
	MIDI.begin();
	MIDI.setHandleNoteOn(Callback_NoteOn);

	// USBMIDI.begin();
	// USBMIDI.onDeviceConnected(Callback_USB_Connected);
	// USBMIDI.onDeviceDisconnectbd(Callback_USB_Disconnected);

	button.setClickMs(200);
	button.setDebounceMs(25);
	button.attachClick(Callback_ButtonSingleClick);

	// Timer for PADs update
	hw_timer_t* timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &PADs_Update_ISR, true);
    timerAlarmWrite(timer, 100, true);            			// 100 us (80Mhz clk)
    timerAlarmEnable(timer);

	// Timer for BAT update
	hw_timer_t* timer1 = timerBegin(1, 80, true);
    timerAttachInterrupt(timer1, &BAT_Update_ISR, true);
    timerAlarmWrite(timer1, 1000000, true);         		// 1s
    timerAlarmEnable(timer1);

	Serial.begin(115200);

	digitalWrite(LED_PIN, HIGH);
	digitalWrite(BUZZER_PIN, HIGH);
	delay(500);
	digitalWrite(LED_PIN, LOW);
	digitalWrite(BUZZER_PIN, LOW);

	setup_done = true;
}

void loop() {

	button.tick();

	// if (!mode_chng && current_mode == MIDI_MODE::USBMIDI) {
	// 	USBMIDI.update();
	// }

	if (pads_need_update && setup_done) {
		pads_need_update = false;

		Pad* pads[7] = {&snare_l1, &tom_l2, &crash_l3, &tom_r1, &tom_r2, &crash_r3, &ride_r4 };

        for (uint8_t i = 0; i < 7; i++) {
            pads[i]->updateForce();
        }

		// pads[0]->doADCRead(); // For Debugging

		if (millis() > pad_silence_until) {
			for (uint8_t i = 0; i < 7; i++) {

				// L3 and R3 has INTERFERENCE issue, skip for now :(
				if (i == 2 || i == 5) { continue; }

				if (pads[i]->isTriggered()) {
					uint8_t velocity = pads[i]->getVelocity();
					uint8_t note = PadType[i];
					uint8_t channel = 10;

					if (current_mode == MIDI_MODE::BLEMIDI && is_ble_connected) {
						MIDI.sendNoteOn(note, velocity, channel);
					// } else if (current_mode == MIDI_MODE::USBMIDI && is_usb_connected) {
					// 	USBMIDI.noteOn(PadType[i], velocity, 1);
					} else if (current_mode == MIDI_MODE::SERIALMIDI) {
						Serial_SendMIDI(note, velocity, channel);
					}

					NoteOffFlags[i].noteOn_sent = true;
					NoteOffFlags[i].noteOn_timestamp = millis();
					NoteOffFlags[i].note = note;
					NoteOffFlags[i].channel = channel;

					pad_silence_until = millis() + PAD_SILENCE_MS;
				}
			}
		}
	}

	if (mode_chng) {
		pads_need_update = false;
		setup_done = false;
		if (current_mode == MIDI_MODE::BLEMIDI) {
			// USBMIDI.end();
			// delay(30);
			BLEMIDI.begin();
			BLEMIDI.setHandleConnected(Callback_BLE_Connected);
			BLEMIDI.setHandleDisconnected(Callback_BLE_Disconnected);
			MIDI.setHandleNoteOn(Callback_NoteOn);
		} else {
			BLEMIDI.end();
			delay(30);
			// USBMIDI.begin();
			// USBMIDI.onDeviceConnected(Callback_USB_Connected);
			// USBMIDI.onDeviceDisconnected(Callback_USB_Disconnected);
		}

		Blink();
		mode_chng = false;
		setup_done = true;
	}

	for (uint8_t i = 0; i < 7; i++) {
		PAD_SendNoteOff(i);
	}

	// if (battery_update_flag) {
	// 	battery_update_flag = false;
	// 	battery.read();
	// }

	// if (battery.isLow() && blinker) {
	// 	Beep();
	// }
}

void IRAM_ATTR PADs_Update_ISR() {
	if (setup_done) { pads_need_update = true; }
}

void IRAM_ATTR BAT_Update_ISR() {
    // battery_update_flag = true;
	// blinker = !blinker;
}
