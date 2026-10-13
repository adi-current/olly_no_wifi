#pragma once

// Local, offline motivational quotes. Keep each one reasonably short -
// FunModule word-wraps these to the screen width, but a very long quote
// will still run past the bottom of the display, so aim for roughly
// one short sentence, not a paragraph.
static const char* const QUOTES[] = {
  "The best way to get started is to quit talking and begin doing.",
  "Don't watch the clock; do what it does. Keep going.",
  "It always seems impossible until it's done.",
  "Believe you can and you're halfway there.",
  "The future belongs to those who believe in their dreams.",
  "Hardships often prepare ordinary people for extraordinary things.",
  "You are never too old to set another goal.",
  "The only way to do great work is to love what you do.",
  "Act as if what you do makes a difference. It does.",
  "Small steps every day add up to big results.",
};
static const int QUOTE_COUNT = sizeof(QUOTES) / sizeof(QUOTES[0]);
