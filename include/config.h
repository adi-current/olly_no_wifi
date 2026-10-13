#ifndef CONFIG_H
#define CONFIG_H

#define DISPLAY_SDA_PIN 8
#define DISPLAY_SCL_PIN 9

// KY-040 rotary encoder
// Uses GPIO0/1/3 (within the 0-4 range) - GPIO2 is deliberately skipped,
// it's an ESP32-C3 strapping pin (boot mode selection) and risky to use
// for a button that could be held down at power-on.
// Also avoids GPIO8/9 (already used for I2C, and also strapping pins)
// and GPIO18/19 (reserved by native USB-CDC, see platformio.ini build_flags)
#define ENCODER_CLK_PIN 3
#define ENCODER_DT_PIN  1
#define ENCODER_SW_PIN  0

// Touch sensor (external TTP223-style breakout - the ESP32-C3 has no
// built-in capacitive touch peripheral, unlike the original ESP32/S2/S3).
// Only one physical sensor exists, so it carries both roles via press
// duration - see TouchModule.
#define TOUCH_SENSOR_PIN 4

// Passive buzzer, driven via LEDC (PWM) - see BuzzerModule.
// GPIO7 is unused, isn't a strapping pin, and isn't reserved by USB-CDC.
#define BUZZER_PIN 7

#endif
