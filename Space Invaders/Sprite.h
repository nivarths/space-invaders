#ifndef SPRITE_H
#define SPRITE_H

#include <cstddef>
#include <cstdint>

class Sprite
{
private:
    size_t spriteWidth;
    size_t spriteHeight;
    uint8_t* spriteData;

public:
    // constructors
    Sprite();
    Sprite(size_t spriteWidth, size_t spriteHeight);
    Sprite(size_t w, size_t h, uint8_t* d);

    // copy constructor
    Sprite(const Sprite& other);

    // copy assignment
    Sprite& operator=(const Sprite& other);

    // move constructor
    Sprite(Sprite&& other) noexcept;

    // move assignment
    Sprite& operator=(Sprite&& other) noexcept;

    // destructor
    ~Sprite();

    // const accessors
    const uint8_t* getSpriteData() const;
    size_t getSpriteWidth() const;
    size_t getSpriteHeight() const;

    // non-const accessors
    uint8_t*& getSpriteData();
    size_t& getSpriteWidth();
    size_t& getSpriteHeight();

    void setPixel(size_t x, size_t y, uint8_t value);
    uint8_t getPixel(size_t x, size_t y) const;
    void setSpriteData(uint8_t* new_data);
};

#endif
