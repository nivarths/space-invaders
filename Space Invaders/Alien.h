#ifndef ALIEN_H
#define ALIEN_H

#include <cstddef>
#include <cstdint>
#include "Sprite.h"
#include "Buffer.h"
#include "AlienType.h"
#include "SpriteAnimation.h"
#include "Entity.h"

enum class AlienState
{
    Alive,
    Dying,
    Dead
};


class Alien : public Entity
{
private:
    AlienType type;
    size_t points;
    bool canShoot;
    int movementPattern;
    size_t shootCooldown;
    size_t currentShootCooldown;

    AlienState state = AlienState::Alive;
    SpriteAnimation* deathAnimation = nullptr;
    double deathTimer = 0.0;

    void setPointsBasedOnType();

    

public:
    // constructor
    Alien(size_t startX, size_t startY, const Sprite* sprite, AlienType alienType);

    // disable copy
    Alien(const Alien&) = delete;
    Alien& operator=(const Alien&) = delete;

    // move constructor
    Alien(Alien&& other) noexcept;
    Alien& operator=(Alien&& other) noexcept;

    // destructor
    ~Alien() = default;

    // drawing alien
    void drawAlien(Buffer& buffer, uint32_t colour) const;

    // move function
    void moveAlien(int direction, size_t bufferWidth);

    // reset function
    void reset(size_t startX, size_t startY);

    // set death animation
    void setDeathAnimation(SpriteAnimation* animation);

    // update death animation
    void updateDeathAnimation();

    // check if alien is dead
    bool isAlienDead() const;

    // set alien as dead
    void setDead(bool dead);

    // get death counter
    size_t getDeathCounter() const;

    // decrement death counter
    void decrementDeathCounter();

    // get alien type
    AlienType getType() const;

    // New methods
    bool canFire() const;
    size_t getPoints() const;
    void setMovementPattern(int pattern);
    void update(double dt) override;
    void setShootCooldown(size_t cooldown);
    void resetShootCooldown();
    bool isReadyToShoot() const;
    
    // Methods to fix linter errors
    bool isAlive() const { return state == AlienState::Alive; }
    void kill();
    size_t getWidth() const { return sprite->getSpriteWidth(); }
};

#endif