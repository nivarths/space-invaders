#include "Buffer.h"
#include "Sprite.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <unordered_map>

// constructor
Buffer::Buffer(size_t bufferWidth, size_t bufferHeight) : width(bufferWidth), height(bufferHeight)
{
    if (bufferWidth > 0 && bufferHeight > 0) 
    {
        data = new uint32_t[bufferWidth * bufferHeight];
        clear(0); // Initialize with black
    } 
    else 
    {
        data = nullptr;
    }
}

// destructor
Buffer::~Buffer()
{
    delete[] data;
}

// move constructor
Buffer::Buffer(Buffer&& other) noexcept 
    : width(other.width), height(other.height), data(other.data)
{
    other.width = 0;
    other.height = 0;
    other.data = nullptr;
}

// move assignment
Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    if (this != &other) 
    {
        delete[] data;
        width = other.width;
        height = other.height;
        data = other.data;
        other.width = 0;
        other.height = 0;
        other.data = nullptr;
    }
    return *this;
}

// draw sprite function
void Buffer::drawSprite(const Sprite& sprite, size_t x, size_t y, uint32_t color)
{
    for (size_t xi = 0; xi < sprite.getSpriteWidth(); ++xi)
    {
        for (size_t yi = 0; yi < sprite.getSpriteHeight(); ++yi)
        {
            size_t sprite_x = x + xi;
            size_t sprite_y = y + yi;
            
            if (sprite_x >= width || sprite_y >= height)
            {
                continue;
            }

            size_t flipped_row = sprite.getSpriteHeight() - 1 - yi;
            uint8_t bit = sprite.getSpriteData()[flipped_row * sprite.getSpriteWidth() + xi];

            if(bit)
            {
                data[sprite_y * width + sprite_x] = color;
            }
        }
    }
}

// clear buffer function
void Buffer::clear(uint32_t color)
{
    for (size_t i = 0; i < width * height; ++i)
    {
        data[i] = color;
    }
}

// getting data function
uint32_t* Buffer::getBufferData()
{
    return data;
}

// getting width function
size_t Buffer::getWidth() const
{
    return width;
}

// getting height function
size_t Buffer::getHeight() const
{
    return height;
}

void Buffer::drawText(const char* text, size_t x, size_t y, uint32_t color)
{
    // Simple text rendering implementation
    size_t xp = x;
    size_t yp = y;
    while (*text)
    {
        if (*text == '\n')
        {
            xp = x;
            yp += 8;
        }
        else
        {
        
            auto it = _fontGlyphs.find(*text);

            if(it != _fontGlyphs.end())
            {
                drawSprite(it->second, xp, yp, color);
            }
            xp += 8;
        }
        ++text;
    }
}

void Buffer::drawPixel(size_t x, size_t y, uint32_t color) {
    if (x < width && y < height) {
        data[y * width + x] = color;
    }
}

void Buffer::setFontGlyphs(const std::unordered_map<char, Sprite>& glyphs)
{
    _fontGlyphs = glyphs;
}