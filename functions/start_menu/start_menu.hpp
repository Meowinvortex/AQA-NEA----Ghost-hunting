#ifndef START_MENU_H
#define START_MENU_H

#include <unordered_map>
#include <variant>
#include <vector>
#include "../../libs/imgui/imgui.h"
#include "../../libs/imgui-sfml-2.6.1/imgui-SFML.h"
#include <SFML/Graphics.hpp>

namespace GH{
namespace START{
    
    extern int current_skintone;
    extern std::map<std::string,int> equipment;
    extern float user_level;
    extern std::vector<std::string> slots;
    extern bool go_create;

    void set_up();

    void load_maps();

    int start_window();

    void run_map_menu();

    std::unordered_map<std::string, std::variant<int, sf::Color, std::string>> run_avatar_menu(int current_skintone);

    void run_equip_menu();

    std::pair<std::string,int> log_in();

    void spells();
}
}

#endif