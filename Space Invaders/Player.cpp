#include "Player.h"
#include <algorithm>

// constructor
Player::Player(size_t startX, size_t startY, const Sprite* sprite, size_t initialLives) 
    : Entity(startX, startY, sprite), lives(initialLives), invincible(false), 
      invincibilityTimer(0), powerUpTimer(0), state(PlayerState::NORMAL), 
      score(0), shootCooldown(0), currentShootCooldown(0)
{
    setMoveSpeed(1.0f);
}

// draw function
void Player::drawPlayer(Buffer& buffer, uint32_t colour) const
{
    if (state != PlayerState::DEAD) {
        if (invincible && (invincibilityTimer / 5) % 2 == 0) {
            // Flash effect during invincibility
            return;
        }
        draw(buffer, colour);
        if (state == PlayerState::POWERED_UP) {
            drawPowerUpEffect(buffer, colour);
        }
    }
}

// move function
void Player::movePlayer(int direction, size_t bufferWidth)
{
    if (state != PlayerState::DEAD) {
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
void Player::reset(size_t startX, size_t startY, size_t initialLives)
{
    setPosition(startX, startY);
    lives = initialLives;
    invincible = false;
    invincibilityTimer = 0;
    powerUpTimer = 0;
    state = PlayerState::NORMAL;
    score = 0;
    setMoveSpeed(1.0f);
}

// lose life function
void Player::loseLife()
{
    if (!invincible && state != PlayerState::DEAD) {
        if (lives > 0) {
            --lives;
            if (lives == 0) {
                state = PlayerState::DEAD;
            } else {
                setInvincible(true);
                invincibilityTimer = 90; // 1.5 seconds at 60 FPS
            }
        }
    }
}

// get lives
size_t Player::getLives() const
{
    return lives;
}

// set lives
void Player::setLives(size_t l)
{
    lives = l;
    if (lives == 0) {
        state = PlayerState::DEAD;
    }
}

// get player X
size_t Player::getPlayerX() const
{
    return getX();
}

// get player Y
size_t Player::getPlayerY() const
{
    return getY();
}

// set player X
void Player::setPlayerX(size_t newX)
{
    setPosition(newX, getY());
}

// New methods implementation
void Player::setInvincible(bool invincible)
{
    this->invincible = invincible;
    if (invincible) {
        state = PlayerState::INVINCIBLE;
    } else if (state == PlayerState::INVINCIBLE) {
        state = PlayerState::NORMAL;
    }
}

void Player::updateInvincibility()
{
    if (invincible && invincibilityTimer > 0) {
        --invincibilityTimer;
        if (invincibilityTimer == 0) {
            setInvincible(false);
        }
    }
}

void Player::applyPowerUp(int type)
{
    switch (type) {
        case 0: // Speed boost
            setMoveSpeed(2.0f);
            powerUpTimer = 300; // 5 seconds at 60 FPS
            state = PlayerState::POWERED_UP;
            break;
        case 1: // Shield
            setInvincible(true);
            invincibilityTimer = 300; // 5 seconds at 60 FPS
            break;
        // Add more power-up types as needed
    }
}

void Player::updatePowerUp()
{
    if (state == PlayerState::POWERED_UP && powerUpTimer > 0) {
        --powerUpTimer;
        if (powerUpTimer == 0) {
            setMoveSpeed(1.0f);
            state = PlayerState::NORMAL;
        }
    }
}

PlayerState Player::getState() const
{
    return state;
}

void Player::setState(PlayerState newState)
{
    state = newState;
}

void Player::addScore(size_t points)
{
    score += points;
}

size_t Player::getScore() const
{
    return score;
}

bool Player::canShoot() const
{
    return state != PlayerState::DEAD && isReadyToShoot();
}

void Player::setShootCooldown(size_t cooldown)
{
    shootCooldown = cooldown;
    currentShootCooldown = cooldown;
}

void Player::resetShootCooldown()
{
    currentShootCooldown = shootCooldown;
}

bool Player::isReadyToShoot() const
{
    return currentShootCooldown == 0;
}

void Player::update(double dt)
{
    updateInvincibility();
    updatePowerUp();
    if (currentShootCooldown > 0) {
        --currentShootCooldown;
    }
}

void Player::drawPowerUpEffect(Buffer& buffer, uint32_t colour) const
{
    // Draw a pulsing outline around the player when powered up
    if (powerUpTimer > 0) {
        size_t pulseSize = (powerUpTimer / 10) % 2; // Creates a pulsing effect
        const Sprite* playerSprite = getSprite();
        if (playerSprite) {
            // Draw outline
            for (size_t y = 0; y < playerSprite->getSpriteHeight(); ++y) {
                for (size_t x = 0; x < playerSprite->getSpriteWidth(); ++x) {
                    if (playerSprite->getPixel(x, y)) {
                        // Draw outline pixels
                        buffer.drawSprite(*playerSprite, getX() - pulseSize, getY(), colour);
                        buffer.drawSprite(*playerSprite, getX() + pulseSize, getY(), colour);
                        buffer.drawSprite(*playerSprite, getX(), getY() - pulseSize, colour);
                        buffer.drawSprite(*playerSprite, getX(), getY() + pulseSize, colour);
                    }
                }
            }
        }
    }
}

bool Player::isInvincible() const 
{
    return state == PlayerState::INVINCIBLE;
}

size_t Player::getInvincibilityTimer() const 
{ 
    return invincibilityTimer; 
}