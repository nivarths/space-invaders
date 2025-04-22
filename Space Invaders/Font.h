// Font.h
#pragma once
#include "Sprite.h"
#include <unordered_map>

// Build and return a small map of ('char' → 8×8 Sprite) for exactly
// the glyphs you need (“SCORE”, “LEVEL”, “CREDIT 00”).
std::unordered_map<char, Sprite> createGameFont();
