#ifndef STRUCT_H
#define STRUCT_H

#include <SFML/Graphics.hpp>
#include <vector>
#include <Candle/Candle.hpp>
#include <random>

namespace GH{
namespace CREATE{
    struct obj{
    sf::RectangleShape shape;  //The main shape of the object, it will be what is visibily seen by the player
    sf::FloatRect hitbox;  //The vertex array will be used for the hitbox of the player for detecting collision with the player, when set up it will wrap around the visibile part of the texture
    sf::Texture texture;  //The texture will be stored here to ensure the shape never loses it
    sf::IntRect rect;  //Used soley for walls and floors a bigger rectangle defentiion then the texture itsself allows the wall and floor texture to repeat rather then stretch
    int room_protection; //How much protection a room has casted
    int theme; //index for SQL table
    bool has_collision;  //Used by the code to determine if the player can walk through the object or not
    bool blocks_light;  //Used by the code to determine if the player's flashlight is blocked by the object
    bool throwable; //Used to determine if the ghost can through this object
    std::pair<int,int> size;  //Gets givem the size of the object
    int index;  //For walls and floors, since the textures are stored permentley somewhere else whilst the code is running, the index determines where in the array the wall/floor texture is stored
    };          //index is also used for objects when loading textures in sql database

    class light_system{//class for a light system
        public:
        std::vector<candle::RadialLight> lights; //stores all the lights
        std::vector<sf::RectangleShape> light_switches; //stores all the light switches
        int light_temp;
        bool lights_on; //are the lights on
        sf::Clock light_flicker; //for flickering lights on and off
        float flicker_time;

        light_system(){
            lights_on = false;
            flicker_time = -1;
        }

        void update_temp(){
            sf::Color temp;

            if(light_temp == 0){
                temp = sf::Color(255,255,255,60);
            }
            else{
                temp = sf::Color(255,255*(0.2*light_temp),255*(0.1*light_temp),60);
            }
        }

        void create_light_switch(){
            sf::RectangleShape temp;
            temp.setSize({32,48});
            temp.setOrigin({16,24});
            light_switches.push_back(temp);
        }

        void flicker_lights(std::mt19937 gen){
            if(flicker_time == -1 || light_flicker.getElapsedTime().asSeconds() > flicker_time){
                light_flicker.restart();
                std::uniform_int_distribution<> t(20,100);
                flicker_time = t(gen);
                flicker_time /= 100;
                lights_on = !lights_on;
            }
        }

    };
}
}

#endif