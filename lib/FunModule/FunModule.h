#pragma once
#include <Arduino.h>
#include <U8g2lib.h>

enum class FunType { MOTIVATION, JOKE };

// Owns Random Motivation and Random Joke. Same drawing/input pattern as
// GamesModule - full-screen via a raw U8G2 pointer, no knowledge of
// Home/Menu/the status strip.
class FunModule {
  public:
    void begin(U8G2* display);

    // Call once when the user selects this feature from the menu.
    void start(FunType type);

    // actionPressed (touch short press) behavior:
    //   Motivation -> shows a new random quote
    //   Joke, setup showing    -> reveals the punchline
    //   Joke, punchline showing -> picks a new random joke
    // backPressed (touch long press) always exits to the menu.
    bool update(int rotationDelta, bool actionPressed, bool backPressed);

    void render();

  private:
    U8G2* display = nullptr;
    FunType currentType = FunType::MOTIVATION;

    int currentQuoteIndex = 0;
    int currentJokeIndex = 0;
    bool punchlineRevealed = false;

    void pickNewQuote();
    void pickNewJoke();

    // Simple greedy word-wrap, drawn line by line starting at startY.
    void renderWrapped(const char* text, int startY, int lineHeight, int maxWidth);
};
