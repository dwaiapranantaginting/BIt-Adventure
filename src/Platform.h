#pragma once
#include <SFML/Graphics.hpp>

class Platform {
public:
    // Texture di-load sekali, dishare ke semua platform
    static sf::Texture sharedTexture;
    static bool        textureLoaded;
    static bool        loadTexture();

    Platform(float x, float y, float width);

    void draw(sf::RenderTarget& target);
    sf::FloatRect getBounds();

private:
    sf::RectangleShape shape;
    float posX, posY, platWidth;
};