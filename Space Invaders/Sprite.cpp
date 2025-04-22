#include "Sprite.h"
#include <algorithm>
#include <cstring>

// default constructor
Sprite::Sprite() : spriteWidth(0), spriteHeight(0), spriteData(nullptr) {}

// constructor with dimensions
Sprite::Sprite(size_t spriteWidth, size_t spriteHeight) : spriteWidth(spriteWidth), spriteHeight(spriteHeight)
{
    if (spriteWidth > 0 && spriteHeight > 0) 
    {
        spriteData = new uint8_t[spriteWidth * spriteHeight];
        std::memset(spriteData, 0, spriteWidth * spriteHeight);
    } 
    else 
    {
        spriteData = nullptr;
    }
}

// constructor with data
Sprite::Sprite(size_t w, size_t h, uint8_t* d) : spriteWidth(w), spriteHeight(h)
{
    if (w > 0 && h > 0 && d != nullptr) 
    {
        spriteData = new uint8_t[w * h];
        std::memcpy(spriteData, d, w * h);
    } 
    else 
    {
        spriteData = nullptr;
    }
}

// copy constructor
Sprite::Sprite(const Sprite& other) : spriteWidth(other.spriteWidth), spriteHeight(other.spriteHeight)
{
    if (other.spriteData != nullptr) 
    {
        spriteData = new uint8_t[spriteWidth * spriteHeight];
        std::memcpy(spriteData, other.spriteData, spriteWidth * spriteHeight);
    } 
    else 
    {
        spriteData = nullptr;
    }
}

// copy assignment
Sprite& Sprite::operator=(const Sprite& other)
{
    if (this != &other) {
        delete[] spriteData;
        spriteWidth = other.spriteWidth;
        spriteHeight = other.spriteHeight;
        if (other.spriteData != nullptr) 
        {
            spriteData = new uint8_t[spriteWidth * spriteHeight];
            std::memcpy(spriteData, other.spriteData, spriteWidth * spriteHeight);
        } 
        else 
        {
            spriteData = nullptr;
        }
    }
    return *this;
}

// move constructor
Sprite::Sprite(Sprite&& other) noexcept
    : spriteWidth(other.spriteWidth), spriteHeight(other.spriteHeight), spriteData(other.spriteData)
{
    other.spriteWidth = 0;
    other.spriteHeight = 0;
    other.spriteData = nullptr;
}

// move assignment
Sprite& Sprite::operator=(Sprite&& other) noexcept
{
    if (this != &other) 
    {
        delete[] spriteData;
        spriteWidth = other.spriteWidth;
        spriteHeight = other.spriteHeight;
        spriteData = other.spriteData;
        other.spriteWidth = 0;
        other.spriteHeight = 0;
        other.spriteData = nullptr;
    }
    return *this;
}

// destructor
Sprite::~Sprite()
{
    delete[] spriteData;
}

// const accessors
const uint8_t* Sprite::getSpriteData() const
{
    return spriteData;
}

size_t Sprite::getSpriteWidth() const
{
    return spriteWidth;
}

size_t Sprite::getSpriteHeight() const
{
    return spriteHeight;
}

// non-const accessors
uint8_t*& Sprite::getSpriteData()
{
    return spriteData;
}

size_t& Sprite::getSpriteWidth()
{
    return spriteWidth;
}

size_t& Sprite::getSpriteHeight()
{
    return spriteHeight;
}

// pixel operations
void Sprite::setPixel(size_t x, size_t y, uint8_t value)
{
    if (spriteData != nullptr && x < spriteWidth && y < spriteHeight) 
    {
        spriteData[y * spriteWidth + x] = value;
    }
}

uint8_t Sprite::getPixel(size_t x, size_t y) const
{
    if (spriteData != nullptr && x < spriteWidth && y < spriteHeight) 
    {
        return spriteData[y * spriteWidth + x];
    }
    return 0;
}

void Sprite::setSpriteData(uint8_t* new_data)
{
    if (new_data != nullptr && spriteWidth > 0 && spriteHeight > 0) 
    {
        if (spriteData != nullptr) 
        {
            delete[] spriteData;
        }
        spriteData = new uint8_t[spriteWidth * spriteHeight];
        std::memcpy(spriteData, new_data, spriteWidth * spriteHeight);
    }
}
