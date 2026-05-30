#pragma once
#include <SFML/Graphics.hpp>

class Laser {
public:
    Laser(float x, float y, bool goRight);

    void update(float dt);
    void draw(sf::RenderTarget& target);
    bool isAlive() { return alive; }
    sf::FloatRect getBounds();

    float animTimer = 0.f;
    float animSpeed = 0.08f; // kecepatan animasi laser
    int   animFrame = 0;

private:
    sf::Texture  texture;
    sf::Sprite   sprite;

    sf::Vector2f velocity;
    float        moveSpeed = 200.f;
    float        lifetime  = 3.f; // detik sebelum hilang
    bool         alive     = true;
    bool         goRight   = true;
};