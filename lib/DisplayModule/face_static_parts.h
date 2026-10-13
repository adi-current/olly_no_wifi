#pragma once
#include <Arduino.h>

// TEMPORARY static parts (brows, mouth, whiskers) - lifted directly from the
// original hand-drawn 96x64 Olly bitmap so the face looks complete while we
// modularize it piece by piece. These will be replaced by procedural
// drawEyebrows()/drawMouth() functions in a later pass (parametrized by
// BrowStyle / MouthStyle), same as drawEyes() already is.

#define BROW_LEFT_WIDTH  13
#define BROW_LEFT_HEIGHT 4
static const unsigned char brow_left_bmp[] PROGMEM = {
0xFC, 0x07,
0xFC, 0x07,
0x07, 0x18,
0x03, 0x18,
};

#define BROW_RIGHT_WIDTH  12
#define BROW_RIGHT_HEIGHT 4
static const unsigned char brow_right_bmp[] PROGMEM = {
0xFC, 0x03,
0xFC, 0x03,
0x03, 0x0C,
0x03, 0x0C,
};

#define MOUTH_WIDTH  11
#define MOUTH_HEIGHT 4
static const unsigned char mouth_bmp[] PROGMEM = {
0x02, 0x04,
0x03, 0x06,
0xFC, 0x03,
0xF8, 0x01,
};

#define WHISKER_LEFT_WIDTH  6
#define WHISKER_LEFT_HEIGHT 4
static const unsigned char whisker_left_bmp[] PROGMEM = {
0x33,
0x33,
0x0D,
0x0C,
};

#define WHISKER_RIGHT_WIDTH  6
#define WHISKER_RIGHT_HEIGHT 4
static const unsigned char whisker_right_bmp[] PROGMEM = {
0x33,
0x33,
0x0C,
0x0C,
};
