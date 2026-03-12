#ifndef WALL_TEXTURES_H
#define WALL_TEXTURES_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>

namespace GH{
namespace TEXTURES{
namespace WALLS{
    std::map<int, sf::Texture> set_up_walls();

    std::map<int, sf::Texture> set_up_floors();
    
}
}
}

#endif