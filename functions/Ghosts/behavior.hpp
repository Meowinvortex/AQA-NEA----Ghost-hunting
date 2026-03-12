#ifndef BEHAVIOR_H
#define BEHAVIOR_H

#include <SFML/Graphics.hpp>
#include <map>
#include "../struct.hpp"

namespace GH{
namespace GHOST{
namespace BEHAV{


    extern std::string current;
    extern float activity_value;
    extern float event;
    extern bool currently_throwing;
    extern std::map<std::string,int> events_map;
    void event_tick(sf::Time delta, float sanity, std::string ghost_type, std::string prev_behav);
    void hunt_tick(sf::Time delta, float sanity);
    std::string wander(float speed, sf::RectangleShape& sprite, std::vector<sf::CircleShape>& walk_path, sf::RectangleShape& chosen_room, sf::Time delta, std::map<int,GH::CREATE::obj> rooms, bool full_wander);
    void flicker(sf::RectangleShape& sprite);
    int throw_object(std::map<int,GH::CREATE::obj>& objects, sf::CircleShape radius, sf::Time delta, std::map<int,GH::CREATE::obj>& walls, std::map<int,GH::CREATE::obj>& floors, bool& can_UV);
    std::string chase(sf::RectangleShape& ghost, sf::RectangleShape& player, sf::Time delta);
    void ghost_ability_check(bool in_light, bool seen_player, std::string ghost_type, float& speed, float sanity);
}
}
}


#endif