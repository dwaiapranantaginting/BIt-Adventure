#include "Platform.h"
#include <iostream>
#include <cstdlib> // Untuk rand()

sf::Texture Platform::topTexture;
sf::Texture Platform::bottomTexture;
bool        Platform::texturesLoaded = false;

bool Platform::loadTextures() {
    if (texturesLoaded) return true;

    if (!topTexture.loadFromFile("assets/map/platform_atas.png")) {
        std::cerr << "[ERROR] Gagal load platform_atas.png!\n";
        return false;
    }

    if (!bottomTexture.loadFromFile("assets/map/platform_bawah.png")) {
        std::cerr << "[ERROR] Gagal load platform_bawah.png!\n";
        return false;
    }

    texturesLoaded = true;
    std::cout << "[OK] Platform textures (Atas & Bawah) loaded!\n";
    return true;
}

Platform::Platform(float x, float y, float width, PlatformType type)
    : posX(x), posY(y), platWidth(width), type(type)
{
    loadTextures();

    shape.setSize({width, 16.f});
    shape.setPosition({x, y});
    shape.setFillColor(sf::Color(139, 90, 43));
    shape.setOutlineColor(sf::Color(80, 50, 20));
    shape.setOutlineThickness(1.f);

    if (texturesLoaded) {
        sf::Texture& currentTex = (type == PlatformType::TOP) ? topTexture : bottomTexture;
        sf::Sprite tile(currentTex);

        // Susun platform dari kiri ke kanan setiap 16 pixel
        for (float mx = posX; mx < posX + platWidth; mx += 16.f) {
            tile.setPosition({mx, posY});

            // ACAK: Pilih salah satu dari 3 variasi (0, 16, atau 32)
            int randomX = (rand() % 3) * 16; 

            tile.setTextureRect(sf::IntRect({randomX, 0}, {16, 16}));
            tileSprites.push_back(tile);
        }
    }
}

void Platform::draw(sf::RenderTarget& target) {
    if (texturesLoaded) {
        for (auto& t : tileSprites) {
            target.draw(t);
        }
    } else {
        target.draw(shape);
    }
}

sf::FloatRect Platform::getBounds() {
    return sf::FloatRect({posX, posY}, {platWidth, 16.f});
}