#include "Entity.h"
#include <algorithm>

Entity::Entity(size_t x, size_t y, const Sprite* spr, float speed)
    : position({x, y}), sprite(spr), moveSpeed(speed)
{
    updateBounds();
}

void Entity::draw(Buffer& buffer, uint32_t color) const {
    if (sprite) {
        buffer.drawSprite(*sprite, position.x, position.y, color);
    }
}

void Entity::moveX(int dx) {
    position.x += static_cast<size_t>(dx * moveSpeed);
    updateBounds();
}

void Entity::moveY(int dy) {
    position.y += static_cast<size_t>(dy * moveSpeed);
    updateBounds();
}

void Entity::setPosition(size_t x, size_t y) {
    position.x = x;
    position.y = y;
    updateBounds();
}

void Entity::setSprite(const Sprite* spr) {
    sprite = spr;
    updateBounds();
}

size_t Entity::getX() const {
    return position.x;
}

size_t Entity::getY() const {
    return position.y;
}

void Entity::updateBounds() {
    if (sprite) {
        bounds.x = position.x;
        bounds.y = position.y;
        bounds.width = sprite->getSpriteWidth();
        bounds.height = sprite->getSpriteHeight();
    } else {
        bounds = {position.x, position.y, 0, 0};
    }
}

const Entity::Bounds& Entity::getBounds() const {
    return bounds;
}

bool Entity::checkCollision(const Entity& other) const {
    return bounds.intersects(other.getBounds());
}

void Entity::setMoveSpeed(float speed) {
    moveSpeed = speed;
}

float Entity::getMoveSpeed() const {
    return moveSpeed;
}

const Sprite* Entity::getSprite() const {
    return sprite;
} 