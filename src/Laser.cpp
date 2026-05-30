#include "Laser.h"
#include <iostream>

Laser::Laser(float x, float y, bool goRight)
    : sprite(texture)
{
    // Simpan dulu ke member variable secara eksplisit
    this->goRight = goRight;
    
    if (!texture.loadFromFile("assets/sprites/laser.png"))
        std::cerr << "[ERROR] Gagal load laser.png!\n";

    sprite.setTexture(texture);
    sprite.setTextureRect(sf::IntRect({0, 0}, {96, 32}));

    velocity = {0.f, 0.f};
    lifetime = 1.0f;

    if (this->goRight) {
        sprite.setOrigin({0.f, 16.f});
        sprite.setScale({4.f, 1.5f});
    } else {
        // PERBAIKAN: Origin tetap di 0.f agar menempel di tangan player
        sprite.setOrigin({0.f, 16.f}); 
        // Skala negatif akan otomatis memanjangkan laser ke arah kiri
        sprite.setScale({-4.f, 1.5f}); 
    }

    sprite.setPosition({x, y});
}

void Laser::update(float dt) {
    if (!alive) return;

    lifetime -= dt;
    if (lifetime <= 0.f) {
        alive = false;
        return;
    }

    // Animasi loop — laser berkedip/beranimasi di tempat
    animTimer += dt;
    if (animTimer >= animSpeed) {
        animTimer = 0.f;
        animFrame++;
        if (animFrame >= 5) animFrame = 0;
        sprite.setTextureRect(
            sf::IntRect({animFrame * 96, 0}, {96, 32}));
    }
}

void Laser::draw(sf::RenderTarget& target) {
    if (!alive) return;
    target.draw(sprite);
}

sf::FloatRect Laser::getBounds() {
    return sprite.getGlobalBounds();
}