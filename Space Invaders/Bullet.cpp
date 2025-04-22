#include "Bullet.h"
#include <algorithm>

// constructor
Bullet::Bullet(size_t startX, size_t startY, const Sprite* sprite, BulletType bulletType) : Entity(startX, startY, sprite), type(bulletType), damage(1), hasTrail(false), trailTimer(0)
{
    setMoveSpeed(1.0f);
}

// draw function
void Bullet::drawBullet(Buffer& buffer, uint32_t colour) const
{
    draw(buffer, colour);
    if (hasTrail && trailTimer > 0) {
        // Draw trail effect
        buffer.drawSprite(*sprite, getX(), getY() - 1, colour);
    }
}

// move function
void Bullet::moveBullet(int direction)
{
    moveY(direction);
}

// reset function
void Bullet::reset(size_t startX, size_t startY)
{
    setPosition(startX, startY);
    hasTrail = false;
    trailTimer = 0;
    setMoveSpeed(1.0f);
}

// New methods implementation
void Bullet::setType(BulletType newType)
{
    type = newType;
}

BulletType Bullet::getType() const
{
    return type;
}

void Bullet::setDamage(size_t newDamage)
{
    damage = newDamage;
}

size_t Bullet::getDamage() const
{
    return damage;
}

void Bullet::enableTrail(bool enable)
{
    hasTrail = enable;
    if (enable) {
        trailTimer = 30; // 0.5 seconds at 60 FPS
    }
}

void Bullet::updateTrail()
{
    if (hasTrail && trailTimer > 0) {
        --trailTimer;
    }
}

bool Bullet::hasTrailEffect() const
{
    return hasTrail && trailTimer > 0;
}

void Bullet::update(double dt)
{
    updateTrail();

    if (type == BulletType::PLAYER)
    {
        moveY(2); // move up
    }
    else if (type == BulletType::ALIEN)
    {
        moveY(-2); // move down
    }
        
}