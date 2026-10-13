#include "GamesModule.h"
#include "rps_bitmaps.h"
#include "coin_bitmaps.h"
#include "dice_bitmaps.h"
#include <esp_system.h> // esp_random() - hardware RNG, no seeding needed

void GamesModule::begin(U8G2* d) {
  display = d;
}

void GamesModule::start(GameType type) {
  currentGame = type;
  if (type == GameType::ROCK_PAPER_SCISSORS) {
    rpsPhase = RpsPhase::PICKING;
    playerChoice = Choice::ROCK;
  } else {
    instantPhase = InstantPhase::PROMPT;
  }
}

const char* GamesModule::choiceName(Choice c) const {
  switch (c) {
    case Choice::ROCK:     return "ROCK";
    case Choice::PAPER:    return "PAPER";
    case Choice::SCISSORS: return "SCISSORS";
  }
  return "";
}

bool GamesModule::update(int rotationDelta, bool actionPressed, bool backPressed) {
  if (backPressed) return true; // universal back - same for every game, no exceptions

  unsigned long now = millis();

  if (currentGame == GameType::ROCK_PAPER_SCISSORS) {
    switch (rpsPhase) {
      case RpsPhase::PICKING:
        if (rotationDelta != 0) {
          int idx = (int)playerChoice + rotationDelta;
          idx = ((idx % 3) + 3) % 3; // wrap both directions
          playerChoice = (Choice)idx;
        }
        if (actionPressed) {
          rpsPhase = RpsPhase::REVEALING;
          rpsPhaseStartMs = now;
        }
        return false;

      case RpsPhase::REVEALING:
        if (now - rpsPhaseStartMs >= RPS_REVEAL_MS) {
          ollyChoice = (Choice)(esp_random() % 3);
          rpsPhase = RpsPhase::RESULT;
          rpsPhaseStartMs = now;
        }
        return false;

      case RpsPhase::RESULT:
        // Never auto-returns - a tap starts a new round instead of exiting.
        // Only backPressed (handled at the top of this function) exits.
        if (actionPressed) {
          rpsPhase = RpsPhase::PICKING;
        }
        return false;
    }
  } else {
    // Coin Toss / Dice Roll share one Prompt -> Spin -> Result flow
    switch (instantPhase) {
      case InstantPhase::PROMPT:
        if (actionPressed) {
          instantPhase = InstantPhase::SPINNING;
          instantPhaseStartMs = now;
          spinDisplayFrame = 0;
        }
        return false;

      case InstantPhase::SPINNING:
        if (now - instantPhaseStartMs >= SPIN_MS) {
          if (currentGame == GameType::COIN_TOSS) {
            coinIsHeads = (esp_random() % 2) == 0;
          } else {
            diceValue = (esp_random() % 6) + 1;
          }
          instantPhase = InstantPhase::RESULT;
          instantPhaseStartMs = now;
        } else {
          // cycle the displayed "fake" face quickly for a spin effect
          int frameCount = (currentGame == GameType::COIN_TOSS) ? 2 : 6;
          spinDisplayFrame = (now / 80) % frameCount;
        }
        return false;

      case InstantPhase::RESULT:
        if (currentGame == GameType::DICE_ROLL) {
          // Dice Roll never auto-returns - a tap re-rolls instead of exiting.
          // Only backPressed (handled at the top of this function) exits.
          if (actionPressed) {
            instantPhase = InstantPhase::SPINNING;
            instantPhaseStartMs = now;
          }
          return false;
        } else {
          // Coin Toss: a tap or the timeout both end it, back to the menu.
          if (actionPressed || now - instantPhaseStartMs >= RESULT_MS) {
            return true;
          }
          return false;
        }
    }
  }
  return false;
}

void GamesModule::render() {
  if (!display) return;
  if (currentGame == GameType::ROCK_PAPER_SCISSORS) renderRPS();
  else if (currentGame == GameType::COIN_TOSS) renderCoin();
  else renderDice();
}

void GamesModule::renderRPS() {
  if (rpsPhase == RpsPhase::PICKING) {
    int idx = (int)playerChoice;
    int x = (128 - RPS_ICON_WIDTH) / 2;
    display->drawXBMP(x, 2, RPS_ICON_WIDTH, RPS_ICON_HEIGHT, PLAYER_RPS_BMPS[idx]);

    display->setFont(u8g2_font_6x10_tr);
    const char* nameLabel = choiceName(playerChoice);
    int lw = display->getStrWidth(nameLabel);
    display->drawStr((128 - lw) / 2, 48, nameLabel);

    const char* hint = "Rotate, Tap=Pick";
    int hw = display->getStrWidth(hint);
    display->drawStr((128 - hw) / 2, 60, hint);

  } else if (rpsPhase == RpsPhase::REVEALING) {
    display->setFont(u8g2_font_7x13B_tr);
    const char* msg = "...";
    int w = display->getStrWidth(msg);
    display->drawStr((128 - w) / 2, 36, msg);

  } else { // RESULT
    // Olly on the left (facing right, mirrored set) - Player on the right
    // (facing left, normal set) - so the two hands face each other rather
    // than both pointing the same way.
    int gap = 8;
    int totalW = RPS_ICON_WIDTH + gap + RPS_ICON_WIDTH;
    int x0 = (128 - totalW) / 2;
    int ollyX = x0;
    int playerX = x0 + RPS_ICON_WIDTH + gap;

    display->drawXBMP(ollyX, 0, RPS_ICON_WIDTH, RPS_ICON_HEIGHT, OLLY_RPS_BMPS[(int)ollyChoice]);
    display->drawXBMP(playerX, 0, RPS_ICON_WIDTH, RPS_ICON_HEIGHT, PLAYER_RPS_BMPS[(int)playerChoice]);

    // "OLLY"/"YOU" labels establish which side is which - right is always you
    display->setFont(u8g2_font_5x7_tr);
    const char* ollyLabel = "OLLY";
    int olw = display->getStrWidth(ollyLabel);
    display->drawStr(ollyX + (RPS_ICON_WIDTH - olw) / 2, 46, ollyLabel);

    const char* youLabel = "YOU";
    int ylw = display->getStrWidth(youLabel);
    display->drawStr(playerX + (RPS_ICON_WIDTH - ylw) / 2, 46, youLabel);

    const char* resultText;
    if (playerChoice == ollyChoice) {
      resultText = "DRAW";
    } else if ((playerChoice == Choice::ROCK && ollyChoice == Choice::SCISSORS) ||
               (playerChoice == Choice::PAPER && ollyChoice == Choice::ROCK) ||
               (playerChoice == Choice::SCISSORS && ollyChoice == Choice::PAPER)) {
      resultText = "YOU WIN!";
    } else {
      resultText = "YOU LOSE";
    }
    display->setFont(u8g2_font_7x13B_tr);
    int w = display->getStrWidth(resultText);
    display->drawStr((128 - w) / 2, 60, resultText);
  }
}

void GamesModule::renderCoin() {
  if (instantPhase == InstantPhase::PROMPT) {
    display->setFont(u8g2_font_7x13B_tr);
    const char* msg = "Tap to flip";
    int w = display->getStrWidth(msg);
    display->drawStr((128 - w) / 2, 36, msg);
    return;
  }

  // Spinning: alternate the two coin faces quickly for a flip effect.
  // Result: show the actual outcome's face, held steady.
  bool showHeads = (instantPhase == InstantPhase::SPINNING)
                      ? (spinDisplayFrame == 0)
                      : coinIsHeads;

  int x = (128 - COIN_HEADS_WIDTH) / 2;
  int y = 2;
  display->drawXBMP(x, y, COIN_HEADS_WIDTH, COIN_HEADS_HEIGHT,
                     showHeads ? coin_heads_bmp : coin_tails_bmp);

  display->setFont(u8g2_font_6x10_tr);
  const char* label = showHeads ? "HEADS" : "TAILS";
  int lw = display->getStrWidth(label);
  display->drawStr((128 - lw) / 2, y + COIN_HEADS_HEIGHT + 8, label);
}

void GamesModule::renderDice() {
  if (instantPhase == InstantPhase::PROMPT) {
    display->setFont(u8g2_font_7x13B_tr);
    const char* msg = "Tap to roll";
    int w = display->getStrWidth(msg);
    display->drawStr((128 - w) / 2, 36, msg);
    return;
  }

  int value = (instantPhase == InstantPhase::SPINNING) ? (spinDisplayFrame + 1) : diceValue;
  int x = (128 - DIE_FACE_WIDTH) / 2;
  int y = (64 - DIE_FACE_HEIGHT) / 2;
  display->drawXBMP(x, y, DIE_FACE_WIDTH, DIE_FACE_HEIGHT, DIE_FACE_BMPS[value - 1]);
}
