#include "Platform.h"
#include <iostream>

// Definisi static variable — di-load sekali untuk semua platform
sf::Texture Platform::sharedTexture;
bool        Platform::textureLoaded = false;

bool Platform::loadTexture() {
    if (textureLoaded) return true;
    if (!sharedTexture.loadFromFile("assets/sprites/Platform_atas.png")) {
        std::cerr << "[ERROR] Gagal load Platform_tile.png!\n";
        return false;
    }
    textureLoaded = true;
    std::cout << "[OK] Platform texture loaded!\n";
    return true;
}

Platform::Platform(float x, float y, float width)
    : posX(x), posY(y), platWidth(width)
{
    loadTexture(); // hanya load jika belum

    shape.setSize({width, 16.f});
    shape.setPosition({x, y});
    shape.setFillColor(sf::Color(139, 90, 43));   // fallback coklat
    shape.setOutlineColor(sf::Color(80, 50, 20));
    shape.setOutlineThickness(1.f);
}

void Platform::draw(sf::RenderTarget& target) {
    if (textureLoaded) {
        sf::Sprite tile(sharedTexture);

        // Gambar kiri (x=0 di texture)
        tile.setTextureRect(sf::IntRect({0, 0}, {16, 16}));
        tile.setPosition({posX, posY});
        target.draw(tile);

        // Gambar tengah (x=16 di texture) — repeat
        tile.setTextureRect(sf::IntRect({16, 0}, {16, 16}));
        float midEnd = posX + platWidth - 16.f;
        for (float mx = posX + 16.f; mx < midEnd; mx += 16.f) {
            tile.setPosition({mx, posY});
            target.draw(tile);
        }

        // Gambar kanan (x=32 di texture)
        tile.setTextureRect(sf::IntRect({32, 0}, {16, 16}));
        tile.setPosition({posX + platWidth - 16.f, posY});
        target.draw(tile);
    } else {
        target.draw(shape); // fallback
    }
}

sf::FloatRect Platform::getBounds() {
    return sf::FloatRect({posX, posY}, {platWidth, 16.f});
}