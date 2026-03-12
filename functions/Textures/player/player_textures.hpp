#ifndef PLAYER_TEXTURES_H
#define PLAYER_TEXTURES_H

#include <filesystem>
#include <vector>
#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>
#include <string>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/ostreamwrapper.h"


namespace GH{
namespace TEXTURES{
namespace PLAYER{


    void save_avatar();

    void change_skin();

    void compile_player_textures(sf::RenderTexture& compiled_texture);

    void change_rect(std::map <std::string, sf::IntRect>& player_rect, std::string mode);

    void cycle_hair(int num);

    void cycle_outfit(int num);

    void cycle_accessories(int num);

    void start_up();
}
}
}

#endif