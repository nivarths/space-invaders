#include "Alien.h"
#include <algorithm>

// constructor
Alien::Alien(size_t startX, size_t startY, const Sprite* sprite, AlienType alienType)
    : Entity(startX, startY, sprite), type(alienType), points(0), canShoot(false), shootCooldown(0), currentShootCooldown(0)
{
    setMoveSpeed(1.0f);
    setPointsBasedOnType();
}

// move constructor
Alien::Alien(Alien&& other) noexcept 
    : Entity(std::move(other)), type(other.type), points(other.points), canShoot(other.canShoot), shootCooldown(other.shootCooldown), currentShootCooldown(other.currentShootCooldown), state(other.state), deathAnimation(other.deathAnimation), deathTimer(other.deathTimer)
{
    other.deathAnimation = nullptr;
}

// move operator assignment
Alien& Alien::operator=(Alien&& other) noexcept
{
    if(this != &other)
    {
        Entity::operator=(std::move(other));
        type = other.type;
        points = other.points;
        canShoot = other.canShoot;
        shootCooldown = other.shootCooldown;
        currentShootCooldown = other.currentShootCooldown;
        state = other.state;
        deathAnimation = other.deathAnimation;
        deathTimer = other.deathTimer;
        other.deathAnimation = nullptr;
    }
    return *this;
}

// draw function
void Alien::drawAlien(Buffer& buffer, uint32_t colour) const
{
    switch (state) 
    {
    case AlienState::Alive:
        Entity::draw(buffer, colour);
        break;
    case AlienState::Dying: 
    {
        const Sprite* spr = deathAnimation->getCurrentFrame();
        if (spr) 
        {
            buffer.drawSprite(*spr, getX(), getY(), colour);
        }
        break;
    }
    case AlienState::Dead:
        break;
    }
}

// move function
void Alien::moveAlien(int direction, size_t bufferWidth)
{
    if (state != AlienState::Alive) {
        int newX = static_cast<int>(getX()) + static_cast<int>(2 * direction * getMoveSpeed());
        if (newX < 0) {
            setPosition(0, getY());
        } else if (newX + sprite->getSpriteWidth() > bufferWidth) {
            setPosition(bufferWidth - sprite->getSpriteWidth(), getY());
        } else {
            setPosition(static_cast<size_t>(newX), getY());
        }
    }
}

// reset function
void Alien::reset(size_t startX, size_t startY)
{
    setPosition(startX, startY);
    state = AlienState::Alive;
    setMoveSpeed(1.0f);
}

// set death animation
void Alien::setDeathAnimation(SpriteAnimation* animation)
{
    deathAnimation = animation;
}

// update death animation
/*void Alien::updateDeathAnimation()
{
    if (state != AlienState::Alive) {
        deathAnimation->advance();
        if (alienAnimation->getTime() >= alienAnimation->getFrameCount() * alienAnimation->getFrameDuration()) {
            alienDeathCounter = 60; // 1 second at 60 FPS
        }
    }
}*/

// check if alien is dead
bool Alien::isAlienDead() const
{
    return state == AlienState::Dead;
}

// set alien as dead
void Alien::setDead(bool dead)
{
    if (dead && state == AlienState::Alive && deathAnimation) {
        state = AlienState::Dying;
        deathTimer = 0.0;
    } else if (dead) {
        state = AlienState::Dead;
    }
}

// get death counter
/*size_t Alien::getDeathCounter() const
{
    return alienDeathCounter;
}

// decrement death counter
void Alien::decrementDeathCounter()
{
    if (alienDeathCounter > 0) {
        --alienDeathCounter;
    }
}*/

// get alien type
AlienType Alien::getType() const
{
    return type;
}

// New methods implementation
bool Alien::canFire() const {
    return state == AlienState::Alive && canShoot && isReadyToShoot();
}

size_t Alien::getPoints() const {
    return points;
}

void Alien::setMovementPattern(int pattern) 
{
    movementPattern = pattern;
}

void Alien::update(double dt)
{
    if (state == AlienState::Alive) 
    {
        if (currentShootCooldown > 0) 
        {
            --currentShootCooldown;
        }
    }
    else if (state == AlienState::Dying) 
    {
        deathAnimation->update(dt);
        deathTimer += dt;
        if (deathAnimation->isFinished())  // or deathTimer >= ...
            state = AlienState::Dead;
    }
}

void Alien::setShootCooldown(size_t cooldown) {
    shootCooldown = cooldown;
    currentShootCooldown = cooldown;
}

void Alien::resetShootCooldown() {
    currentShootCooldown = shootCooldown;
}

bool Alien::isReadyToShoot() const {
    return currentShootCooldown == 0;
}

void Alien::setPointsBasedOnType() {
    switch (type) {
        case AlienType::TYPE_A:
            points = 10;
            break;
        case AlienType::TYPE_B:
            points = 20;
            break;
        case AlienType::TYPE_C:
            points = 30;
            break;
        default:
            points = 0;
            break;
    }
}

void Alien::kill()
{
    if(state == AlienState::Alive && deathAnimation) 
    {
        state = AlienState::Dying;
        deathTimer = 0.0;
    }
    else 
    {
        state = AlienState::Dead;
    }
}
