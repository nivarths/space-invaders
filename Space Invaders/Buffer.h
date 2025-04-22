#ifndef BUFFER_H
#define BUFFER_H

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <cstring>
#include <unordered_map>
#include "Sprite.h"

class Sprite;

class Buffer
{
private:
    size_t width;
    size_t height;
    uint32_t* data;
    std::unordered_map<char, Sprite> _fontGlyphs;

public:
    Buffer(size_t width, size_t height);
    ~Buffer();

    // Move semantics
    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    // Delete copy operations
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    // Drawing functions
    void drawSprite(const Sprite& sprite, size_t x, size_t y, uint32_t color);
    void drawText(const char* text, size_t x, size_t y, uint32_t color);
    void clear(uint32_t color);
    void drawPixel(size_t x, size_t y, uint32_t color);

    // Accessors
    uint32_t* getBufferData();
    size_t getWidth() const;
    size_t getHeight() const;

    void setFontGlyphs(const std::unordered_map<char, Sprite>& glyphs);
};

#endif