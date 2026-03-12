#include <SFML/Graphics.hpp>
#include <iostream>
#include <map>

#include "walls_textures.hpp"
#include "../../SQL/SQL.hpp"

/*This files loads the sprite sheets for walls and floors and seperated them into their own textures so they can be correctly used*/

namespace GH{
namespace TEXTURES{
namespace WALLS{
    std::map<int, sf::Texture> set_up_walls(){//seperate wall sprite sheet
        sf::IntRect rects[] = {
            sf::IntRect(1,0,32,32), sf::IntRect(178,0,32,32), sf::IntRect(355,0,32,32),
            sf::IntRect(1,32,32,32), sf::IntRect(178,32,32,32), sf::IntRect(355,32,32,32),
            sf::IntRect(1,64,32,32), sf::IntRect(178,64,32,32), sf::IntRect(355,64,32,32),
            sf::IntRect(1,96,32,32), sf::IntRect(178,96,32,32), sf::IntRect(355,96,32,32),
            sf::IntRect(1,128,32,32), sf::IntRect(178,128,32,32), sf::IntRect(355,128,32,32),
            sf::IntRect(1,160,32,32), sf::IntRect(178,160,32,32), sf::IntRect(355,160,32,32),
            sf::IntRect(1,192,32,32), sf::IntRect(178,192,32,32), sf::IntRect(355,192,32,32),
            sf::IntRect(1,224,32,32), sf::IntRect(178,224,32,32), sf::IntRect(355,224,32,32),
            sf::IntRect(1,256,32,32), sf::IntRect(178,256,32,32), sf::IntRect(355,256,32,32),
            sf::IntRect(1,288,32,32), sf::IntRect(178,288,32,32), sf::IntRect(355,288,32,32),
            sf::IntRect(1,320,32,32), sf::IntRect(178,320,32,32), sf::IntRect(355,320,32,32),
            sf::IntRect(1,352,32,32), sf::IntRect(178,352,32,32), sf::IntRect(355,352,32,32),
            sf::IntRect(1,384,32,32), sf::IntRect(178,384,32,32), sf::IntRect(355,384,32,32),
            sf::IntRect(1,416,32,32), sf::IntRect(178,416,32,32), sf::IntRect(355,416,32,32),
            sf::IntRect(1,448,32,32), sf::IntRect(178,448,32,32), sf::IntRect(355,448,32,32),
            sf::IntRect(1,480,32,32), sf::IntRect(178,480,32,32), sf::IntRect(355,480,32,32)
        };
        std::map<int, sf::Texture> temp_map;
        std::map<int, sf::Texture> textures;
        sf::Image temp_image;
        sf::Texture perm_texture;

        perm_texture = GH::SQL::load_texture(23,1);
        sf::Image perm_image = perm_texture.copyToImage();
        for(auto rect : rects){
                sf::Texture temp_texture;
                temp_image.create(rect.width, rect.height);
                temp_image.copy(perm_image, 0, 0, rect);
                temp_texture.loadFromImage(temp_image);
                if(rect != sf::IntRect(0,0,48,7)){
                    temp_texture.setRepeated(true);
                }
                textures[textures.size()] = temp_texture;

        }
        return textures;
    }

    std::map<int, sf::Texture> set_up_floors(){//seperate floor sprite sheet
        std::map<int, sf::Texture> temp_map;
        std::map<int, sf::Texture> textures;
        sf::Image temp_image;
        sf::Texture perm_texture;

        perm_texture = GH::SQL::load_texture(23,2);
        sf::Image perm_image = perm_texture.copyToImage();
        for(int j = 0 ; j < 4 ; j++){
            for(int i = 0 ; i < 18 ; i++){
                    sf::IntRect rect(16+(64*j) , 48+(i*32), 16, 16);
                    sf::Texture temp_texture;
                    temp_image.create(16,16);
                    temp_image.copy(perm_image, 0, 0, rect);
                    temp_texture.loadFromImage(temp_image);
                    temp_texture.setRepeated(true);
                    textures[textures.size()] = temp_texture;

            }
        }
        return textures;
    }
    
}
}
}
