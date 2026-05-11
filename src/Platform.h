#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

// Tipe untuk membedakan file platform_atas dan platform_bawah
enum class PlatformType { TOP, BOTTOM };

class Platform {
public:
    static sf::Texture topTexture;
    static sf::Texture bottomTexture;
    static bool        texturesLoaded;
    static bool        loadTextures();

    Platform(float x, float y, float width, PlatformType type);

    void draw(sf::RenderTarget& target);
    sf::FloatRect getBounds();

private:
    sf::RectangleShape shape;
    float posX, posY, platWidth;
    PlatformType type;

    // Menyimpan susunan sprite yang sudah diacak saat awal dibuat
    std::vector<sf::Sprite> tileSprites; 
};