#include "Laser.h"
#include <iostream>

Laser::Laser(float x, float y, bool goRight)
    : sprite(texture)
{
    this->goRight = goRight;

    // Load texture laser
    if (!texture.loadFromFile("assets/sprites/laser.png"))
    {
        std::cerr << "[ERROR] Gagal load laser.png!\n";
    }

    // Load sound laser
    if (!laserBuffer.loadFromFile("assets/sounds/sound_laser.mp3"))
    {
        std::cerr << "[ERROR] Gagal load sound_laser.mp3!\n";
    }

    laserSound.setBuffer(laserBuffer);

    // Putar suara saat laser dibuat
    laserSound.play();

    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({0, 0}, {96, 32}));

    velocity = {0.f, 0.f};
    lifetime = 1.0f;

    if (this->goRight)
    {
        sprite.setOrigin({0.f, 16.f});
        sprite.setScale({4.f, 1.5f});
    }
    else
    {
        sprite.setOrigin({0.f, 16.f});
        sprite.setScale({-4.f, 1.5f});
    }

    sprite.setPosition({x, y});
}

void Laser::update(float dt)
{
    if (!alive) return;

    lifetime -= dt;

    if (lifetime <= 0.f)
    {
        alive = false;
        return;
    }

    animTimer += dt;

    if (animTimer >= animSpeed)
    {
        animTimer = 0.f;
        animFrame++;

        if (animFrame >= 5)
            animFrame = 0;

        sprite.setTextureRect(
            sf::IntRect({animFrame * 96, 0}, {96, 32})
        );
    }
}

void Laser::draw(sf::RenderTarget& target)
{
    if (!alive) return;

    target.draw(sprite);
}

sf::FloatRect Laser::getBounds()
{
    return sprite.getGlobalBounds();
}