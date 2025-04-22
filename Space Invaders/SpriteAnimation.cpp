// SpriteAnimation.cpp
#include "SpriteAnimation.h"
#include <algorithm>
#include <cmath>

void SpriteAnimation::advance() {
    // no‐op for now: your time‐based stepping lives in update()
}

void SpriteAnimation::update(double dt) {
    elapsedSeconds += dt;
    double total = frame_duration * double(num_frames);
    if (elapsedSeconds >= total) {
        if (loop) {
            elapsedSeconds = std::fmod(elapsedSeconds, total);
        } else {
            elapsedSeconds = total;
        }
    }
}

bool SpriteAnimation::isFinished() const {
    // non‐looping animation is “done” once elapsed hits its full span
    return !loop
        && elapsedSeconds >= double(frame_duration * num_frames);
}

const Sprite* SpriteAnimation::getCurrentFrame() const {
    if (num_frames == 0 || frames == nullptr) return nullptr;
    // clamp to last frame if we’re past the end
    size_t idx = std::min<size_t>(
        num_frames - 1,
        size_t(elapsedSeconds / frame_duration)
    );
    return frames[idx];
}
