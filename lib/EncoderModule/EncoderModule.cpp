#include "EncoderModule.h"
#include "config.h"

EncoderModule* EncoderModule::instance = nullptr;

// Full-step quadrature state graph (the classic Buxton-style rotary
// encoder algorithm). Each row is the current state; each column is the
// current pin reading (CLK<<1 | DT), 0-3. A cell's low nibble is the next
// state; the DIR_CW/DIR_CCW bits are only set on the single transition
// that completes one full, valid detent-to-detent rotation.
//
// Any bounce/noise transition that doesn't match a real rotation sequence
// just routes back to R_START (or stays put) instead of being counted -
// that's what makes this robust against the KY-040's contact bounce,
// unlike counting raw pulses.
#define R_START     0x0
#define R_CW_FINAL  0x1
#define R_CW_BEGIN  0x2
#define R_CW_NEXT   0x3
#define R_CCW_BEGIN 0x4
#define R_CCW_FINAL 0x5
#define R_CCW_NEXT  0x6

#define DIR_CW   0x10
#define DIR_CCW  0x20
#define DIR_MASK 0x30

static const uint8_t STATE_TABLE[7][4] = {
  // R_START
  { R_START,    R_CW_BEGIN,  R_CCW_BEGIN, R_START },
  // R_CW_FINAL
  { R_CW_NEXT,  R_START,     R_CW_FINAL,  R_START | DIR_CW },
  // R_CW_BEGIN
  { R_CW_NEXT,  R_CW_BEGIN,  R_START,     R_START },
  // R_CW_NEXT
  { R_CW_NEXT,  R_CW_BEGIN,  R_CW_FINAL,  R_START },
  // R_CCW_BEGIN
  { R_CCW_NEXT, R_START,     R_CCW_BEGIN, R_START },
  // R_CCW_FINAL
  { R_CCW_NEXT, R_CCW_FINAL, R_START,     R_START | DIR_CCW },
  // R_CCW_NEXT
  { R_CCW_NEXT, R_CCW_FINAL, R_CCW_BEGIN, R_START },
};

void EncoderModule::begin() {
  pinMode(ENCODER_CLK_PIN, INPUT_PULLUP);
  pinMode(ENCODER_DT_PIN, INPUT_PULLUP);
  pinMode(ENCODER_SW_PIN, INPUT_PULLUP); // KY-040 SW is active-low

  instance = this;
  encoderState = R_START;

  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK_PIN), onEncoderChangeISR, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_DT_PIN), onEncoderChangeISR, CHANGE);

  buttonRawLast = (digitalRead(ENCODER_SW_PIN) == LOW);
  buttonStableState = buttonRawLast;
  buttonLastChangeMs = millis();
}

void IRAM_ATTR EncoderModule::onEncoderChangeISR() {
  if (instance) instance->handleEncoderChange();
}

void EncoderModule::handleEncoderChange() {
  uint8_t clk = digitalRead(ENCODER_CLK_PIN);
  uint8_t dt  = digitalRead(ENCODER_DT_PIN);
  uint8_t pinState = (dt << 1) | clk; // must match STATE_TABLE's bit convention

  encoderState = STATE_TABLE[encoderState & 0x0F][pinState];
  uint8_t dir = encoderState & DIR_MASK;

  if (dir == DIR_CW) {
    rotationAccumulator++;
  } else if (dir == DIR_CCW) {
    rotationAccumulator--;
  }
}

int EncoderModule::takeRotation() {
  noInterrupts();
  int clicks = rotationAccumulator;
  rotationAccumulator = 0;
  interrupts();
  return clicks;
}

void EncoderModule::update() {
  bool rawPressed = (digitalRead(ENCODER_SW_PIN) == LOW);
  unsigned long now = millis();

  if (rawPressed != buttonRawLast) {
    buttonRawLast = rawPressed;
    buttonLastChangeMs = now;
  }

  if ((now - buttonLastChangeMs) >= DEBOUNCE_MS && rawPressed != buttonStableState) {
    buttonStableState = rawPressed;

    if (buttonStableState) {
      // Just pressed
      buttonPressedAtMs = now;
      longPressFired = false;
    } else {
      // Just released - if we didn't already fire a long press while held,
      // this release counts as a short click.
      if (!longPressFired) {
        clickPending = true;
      }
    }
  }

  // Long press fires while still held, not on release, so the UI can react
  // immediately (e.g. show "release to confirm") rather than waiting.
  if (buttonStableState && !longPressFired && (now - buttonPressedAtMs) >= LONG_PRESS_MS) {
    longPressFired = true;
    longPressPending = true;
  }
}

bool EncoderModule::wasClicked() {
  if (clickPending) {
    clickPending = false;
    return true;
  }
  return false;
}

bool EncoderModule::wasLongPressed() {
  if (longPressPending) {
    longPressPending = false;
    return true;
  }
  return false;
}

bool EncoderModule::isButtonDown() const {
  return buttonStableState;
}
