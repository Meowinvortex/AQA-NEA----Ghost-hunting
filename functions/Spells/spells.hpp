#ifndef SPELLS_H
#define SPELLS_H

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <map>
#include "../struct.hpp"

namespace GH{
namespace SPELLS{

    extern bool is_dashing;
    extern bool is_spirit;
    extern bool controlled_tp;
    extern int tp_left;

    extern std::vector<sf::RectangleShape> slots;
    extern std::vector<sf::RectangleShape> slots_overlay;
     
    void set_up();

    void align_slots(sf::RenderWindow& window, sf::View& view, std::vector<std::string> spells);

    void set_slot_txt(std::vector<std::string> spells);

    void dash(sf::RectangleShape& player, sf::Time delta, std::string facing, std::map<int,GH::CREATE::obj> walls, int spell_index);

    void speed(float& speed, int spell_index);

    void spell_check(std::map<int,std::string> spells, float& speed);

    int protection(int tier, int room_protection, int index);

    void spirit_form(int index, sf::RectangleShape& player);

    void tp(int index, bool chaotic, sf::RectangleShape& player, std::map<int,GH::CREATE::obj>& floors, std::map<int,GH::CREATE::obj>& walls);
}
}

#endif