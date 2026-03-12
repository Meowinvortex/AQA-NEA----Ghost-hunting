#ifndef INVENTORY_H
#define INVENTORY_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include<Candle/Candle.hpp>

namespace GH{
namespace INV{

    struct item{
       sf::RectangleShape shape;
       sf::IntRect rect;
       std::string name;
       bool in_inventory;
       bool has_light;
       candle::RadialLight light;
       sf::CircleShape light_filter;
    };
    extern std::vector<sf::IntRect> texture_rects;
    extern std::vector<item> loaded_items;
    extern std::vector<sf::RectangleShape> slots;
    extern std::vector<std::string> in_slots;
    extern int selected_slot;
    extern bool locked_on;
    extern sf::RectangleShape* locked_item;

    void setup();

    void align_slots(sf::RenderWindow& window, sf::View& view);

    void load_in_item(std::string type, sf::Vector2f pos);

    void pick_up(sf::RectangleShape& body);

    void drop(sf::RectangleShape& body);

    void slot_change(int num);

    void move_item(sf::RenderWindow& window, std::map<int,GH::CREATE::obj> walls, std::map<int,GH::CREATE::obj> objects);

    void blow_lantern(item& lantern, bool blow, std::string ghost);

    void align_lights(candle::LightingArea& fog);

    bool check_uv(sf::RectangleShape& UV);

    void reset();
}
}
#endif