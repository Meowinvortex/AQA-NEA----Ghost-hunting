#ifndef MATHS_H
#define MATHS_H

#include <SFML/Graphics.hpp>
#include <cmath>
#include <Candle/Candle.hpp>
#include <cstdlib>
#include <iostream>
#include <map>
#include "struct.hpp"

using namespace std;

namespace GH{
namespace MATH{

     void stamina_check(float& stamina, float& speed);

     float getmouseang(sf::Vector2i mousePos, sf::Vector2f center);

     tuple <string,string> check_velocity(sf::Time delta_time, float speed, int M_WIDTH, int M_HEIGHT, sf::RectangleShape& body, std::map<int, GH::CREATE::obj>& collision_sprites, std::vector<sf::RectangleShape>& hitboxes, sf::View& view, sf::Vector2f previous_pos);

     map<int, pair<sf::Vector2f, sf::Vector2f>> get_points(sf::RectangleShape& shape);

     bool rect_in_view(const sf::FloatRect& rect, const sf::View& view);

     vector<sf::Vector2f> vertex_hitbox(sf::RectangleShape& shape, sf::Texture& texture);

     std::vector<string> merge_sort(std::vector<string> unsorted);

     bool binary_search(vector<std::string>& arr, int low, int high, std::string target);

     sf::Vector2f cardinal_direction(sf::Vector2f player_pos, sf::Vector2f ghost_pos);

     bool prevent_collisions(std::vector<sf::RectangleShape>& hitboxes, sf::View& view,
        sf::RectangleShape& body,
        sf::Time deltaTime, float speed,
        std::map<int, GH::CREATE::obj>& collision_sprites, sf::Vector2f previous_pos);

}
}

#endif