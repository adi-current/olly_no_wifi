#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

enum class GameType { ROCK_PAPER_SCISSORS, COIN_TOSS, DICE_ROLL };

// Owns Rock Paper Scissors, Coin Toss, and Dice Roll. Draws full-screen via
// a raw U8G2 pointer handed in from main.cpp (see DisplayModule::getRawDisplay())
// - this module doesn't know about Home/Menu/the status strip at all, same
// separation FaceModule/MenuModule already keep.
//
// Input signals match the project-wide convention used everywhere else:
//   rotationDelta  -> browse/cycle (only RPS uses this, to cycle the pick)
//   actionPressed  -> touch TAP (confirm a pick / trigger a toss/roll / go again)
//   backPressed    -> touch long press (always exits back to the menu)
class GamesModule {
  public:
    void begin(U8G2* display);

    // Call once when the user selects a game from the menu - resets all
    // per-game state so a stale result from last time never leaks through.
    void start(GameType type);

    // Call every loop() while this game is active. Returns true exactly
    // once, the frame the game is done and wants to return to the menu
    // (either the user backed out, or Coin Toss's own auto-return timer
    // elapsed - Dice Roll never auto-returns, see update()).
    bool update(int rotationDelta, bool actionPressed, bool backPressed);

    void render();

  private:
    U8G2* display = nullptr;
    GameType currentGame = GameType::ROCK_PAPER_SCISSORS;

    // --- Rock Paper Scissors ---
    enum class Choice : uint8_t { ROCK, PAPER, SCISSORS };
    enum class RpsPhase { PICKING, REVEALING, RESULT };
    RpsPhase rpsPhase = RpsPhase::PICKING;
    Choice playerChoice = Choice::ROCK;
    Choice ollyChoice = Choice::ROCK;
    unsigned long rpsPhaseStartMs = 0;
    static const unsigned long RPS_REVEAL_MS = 500;   // suspense beat before showing the result

    // --- Coin Toss / Dice Roll share the same Prompt -> Spin -> Result skeleton ---
    // Coin Toss and Dice Roll: Result never auto-returns - a tap goes again
    // (back to Spinning) and only a long-press (Hold, see update()) exits.
    enum class InstantPhase { PROMPT, SPINNING, RESULT };
    InstantPhase instantPhase = InstantPhase::PROMPT;
    unsigned long instantPhaseStartMs = 0;
    static const unsigned long SPIN_MS = 800;
    bool coinIsHeads = false;
    int diceValue = 1;
    int spinDisplayFrame = 0; // which fake face to show mid-spin, cycles quickly

    const char* choiceName(Choice c) const;
    void renderRPS();
    void renderCoin();
    void renderDice();
};
