#include "spells.hpp"
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <iostream>
#include <map>
#include "../struct.hpp"

/*This files handles anything to do with the players spells, including 
-spell abilites
-eqippiung spells
-getting correct textures*/

namespace GH{
namespace SPELLS{

    std::random_device rd;                      
    std::mt19937 gen(rd());
    
    std::map<std::string,sf::SoundBuffer> buffers;
    std::map<std::string,sf::Sound> sounds;

    std::map<std::string,int> spell_times = {{"Dash",3}, {"Speed",10}, {"Basic protection",120}, {"Protection",120}, {"Enhanced protection",120}, {"Spirit form", 180}, {"Chaotic tp", 180}, {"Controlled tp", 500}};
    bool first_cast[3];

    std::vector<sf::RectangleShape> slots = {sf::RectangleShape({50,50}), sf::RectangleShape({50,50}), sf::RectangleShape({50,50})};
    std::vector<sf::RectangleShape> slots_overlay = {sf::RectangleShape({50,0}), sf::RectangleShape({50,0}), sf::RectangleShape({50,0})};

    sf::Texture spell_sheet;
    std::map<std::string,sf::IntRect> spell_rects = {{"Speed",sf::IntRect(0,0,32,32)},{"Dash",sf::IntRect(32,0,32,32)},{"Basic protection",sf::IntRect(64,0,32,32)},
                                                    {"Chaotic tp",sf::IntRect(0,32,32,32)},{"Spirit form",sf::IntRect(32,32,32,32)},{"Protection",sf::IntRect(64,32,32,32)},
                                                    {"Controlled tp",sf::IntRect(0,64,32,32)},{"Disorientation",sf::IntRect(32,64,32,32)},{"Enhanced protection",sf::IntRect(64,64,32,32)}};

    bool is_dashing = false;
    bool is_speeding = false;
    bool is_spirit = false;
    bool controlled_tp = false;
    int tp_left = 0;
    std::vector<sf::Clock> spell_timers;

    void set_up(){//initial setup

        spell_sheet.loadFromFile(ASSETS_DIR"/textures/spells.png");

        buffers["Dash"].loadFromFile(ASSETS_DIR"/sounds/Magic and Spells 5 - Universal Sound Effects/MP3/Magic Spell 11.mp3");
        sounds["Dash"].setBuffer(buffers["Dash"]);
        buffers["Protection_cast"].loadFromFile(ASSETS_DIR"/sounds/Magic and Spells 5 - Universal Sound Effects/MP3/Magic Spell 14.mp3");
        buffers["Protection_damage"].loadFromFile(ASSETS_DIR"/sounds/Magic and Spells 5 - Universal Sound Effects/MP3/Magic Spell 22.mp3");
        buffers["Protection_broken"].loadFromFile(ASSETS_DIR"/sounds/Magic and Spells 5 - Universal Sound Effects/MP3/Magic Spell 24.mp3");
        sounds["Protection_cast"].setBuffer(buffers["Protection_cast"]);
        sounds["Protection_damage"].setBuffer(buffers["Protection_damage"]);
        sounds["Protection_broken"].setBuffer(buffers["Protection_broken"]);
        buffers["Spirit form"].loadFromFile(ASSETS_DIR"/sounds/Magic and Spells 5 - Universal Sound Effects/MP3/Magic Spell 20.mp3");
        sounds["Spirit form"].setBuffer(buffers["Spirit form"]);

        for(int i = 0 ; i < 3 ; i++){
            spell_timers.push_back(sf::Clock{});
            slots[i].setFillColor(sf::Color(255,255,255));
            slots[i].setTexture(&spell_sheet);
            slots_overlay[i].setFillColor(sf::Color(0,0,0));
        }
    }

    void set_slot_txt(std::vector<std::string> spells){//set the spell textures to the correct spell
        for(int i = 0 ; i < 3 ; i++){
            slots[i].setTextureRect(spell_rects[spells[i]]);
            first_cast[i] = true;
        }
    }

    void align_slots(sf::RenderWindow& window, sf::View& view, std::vector<std::string> spells){//align spell slots with the view's position
        sf::FloatRect viewport = view.getViewport();
        sf::Vector2f view_topright(viewport.left + viewport.getSize().x, viewport.top);
        sf::Vector2f world_topright = window.mapPixelToCoords(sf::Vector2i(view_topright.x * window.getSize().x - 100, view_topright.y * window.getSize().y), view);
        
        for(int i = 0 ; i < 3 ; i++){
            slots[i].setPosition({world_topright.x - (i*60), world_topright.y+25});
            slots_overlay[i].setPosition({world_topright.x - (i*60), world_topright.y+25});
            if(!first_cast[i]){
                float percentage = spell_timers[i].getElapsedTime().asSeconds()/spell_times[spells[i]];
                if(percentage > 1){
                    percentage = 1;
                }
                slots_overlay[i].setSize({50, 50-(50*percentage)});
            }
        }
    }

    void dash(sf::RectangleShape& player, sf::Time delta, std::string facing, std::map<int,GH::CREATE::obj> walls, int spell_index){//occurs when the player dashes, sending them in one direction at higher speeds
        if(!is_spirit && !is_dashing && (spell_timers[spell_index].getElapsedTime().asSeconds() >= 3 || first_cast[spell_index])){
            first_cast[spell_index] = false;
            sounds["Dash"].play();
            is_dashing = true;
            spell_timers[spell_index].restart();
            player.setFillColor(sf::Color(255,0,255,150));
        }
        else if(is_dashing && spell_timers[spell_index].getElapsedTime().asSeconds() >= 0.4){
            is_dashing = false;
            spell_timers[spell_index].restart();
            player.setFillColor(sf::Color(255,255,255,255));
        }
        else if(is_dashing){
            sf::Vector2f velocity = {0,0};
            if(facing == "Forward"){
                velocity = {0,-1};
            }
            else if(facing == "Back"){
                velocity = {0,1};
            }
            else if(facing == "Left"){
                velocity = {-1,0};
            }
            else if(facing == "Right"){
                velocity = {1,0};
            }
            bool wall_collision = false;
            for(int i = 0 ; i < 800 ; i++){
                player.move(velocity*(delta.asSeconds()*2));
                for(auto& wall : walls){
                    if(wall.second.hitbox.intersects(player.getGlobalBounds())){
                        is_dashing = false;
                        spell_timers[spell_index].restart();
                        player.setFillColor(sf::Color(255,255,255,255));
                        wall_collision = true;
                        break;
                    }
                }
                if(wall_collision){
                    break;
                }
            }
        }
    }
    
    void speed(float& speed, int spell_index){//used for speed spell to increase the players speed temporarily
        if(spell_timers[spell_index].getElapsedTime().asSeconds() >= 10 || first_cast[spell_index]){
            first_cast[spell_index] = false;
            speed += 200;
            is_speeding = true;
            spell_timers[spell_index].restart();
        }
    }

    void spell_check(std::map<int,std::string> spells, float& speed){//check if any spells need to be stopped
        if(is_speeding){
            for(int i = 0 ; i < 3 ; i++){
                if(spells[i] == "Speed" && spell_timers[i].getElapsedTime().asSeconds() >= 5){
                    spell_timers[i].restart();
                    is_speeding = false;
                    speed -= 200;
                    break;
                }
            }
        }  
    }

    int protection(int tier, int room_protection, int index){
        if(tier < 0){
            room_protection -= tier;
            if(room_protection == 0){
                sounds["Protection_broken"].play();
            }
            else{
                sounds["Protection_damage"].play();
            }
            return room_protection;
        }
        if(spell_timers[index].getElapsedTime().asSeconds() >= 300 || first_cast[index]){
            first_cast[index] = false;
            if(room_protection == 0 && tier > 0){
                room_protection = tier;
                sounds["Protection_cast"].play();
            }

            spell_timers[index].restart();
        }
        return room_protection;
    }

    void spirit_form(int index, sf::RectangleShape& player){
        if(!is_spirit && (spell_timers[index].getElapsedTime().asSeconds() >= 180 || first_cast[index])){
            first_cast[index] = false;
            sounds["Spirit form"].play();
            is_spirit = true;
            spell_timers[index].restart();
            player.setFillColor(sf::Color(255,255,255,60));
        }
        else if(is_spirit && spell_timers[index].getElapsedTime().asSeconds() >= 5){
            is_spirit = false;
            spell_timers[index].restart();
            player.setFillColor(sf::Color(255,255,255,255));
        }
    }

    void tp(int index, bool chaotic, sf::RectangleShape& player, std::map<int,GH::CREATE::obj>& floors, std::map<int,GH::CREATE::obj>& walls){
        if(chaotic && (spell_timers[index].getElapsedTime().asSeconds() >= 180 || first_cast[index])){
            first_cast[index] = false;
            while(true){
                std::uniform_int_distribution<> floor_dist(0,floors.size()-1);
                std::uniform_int_distribution<> wall_dist(0,walls.size()-1);
                int chosen_floor = floor_dist(gen);

                int minx,miny,maxx,maxy;

                int x_1 = floors[chosen_floor].shape.getTransform().transformPoint(floors[chosen_floor].shape.getPoint(0)).x;
                int x_2 = floors[chosen_floor].shape.getTransform().transformPoint(floors[chosen_floor].shape.getPoint(1)).x;
                int y_1 = floors[chosen_floor].shape.getTransform().transformPoint(floors[chosen_floor].shape.getPoint(0)).y;
                int y_2 = floors[chosen_floor].shape.getTransform().transformPoint(floors[chosen_floor].shape.getPoint(2)).y;
                if(x_1 < x_2){minx = x_1; maxx = x_2;}
                else{minx = x_2; maxx = x_1;}
                if(y_1 < y_2){miny = y_1; maxy = y_2;}
                else{miny = y_2; maxy = y_1;}
                std::uniform_int_distribution<> dist_x(minx, maxx);
                std::uniform_int_distribution<> dist_y(miny, maxy);

                sf::Vector2f tp = {dist_x(gen),dist_y(gen)};
                bool can_tp = true;
                for(auto& wall : walls){
                    if(wall.second.shape.getGlobalBounds().contains(tp)){
                        can_tp = false;
                        break;
                    }
                }

                if(can_tp){
                    player.setPosition(tp);
                    return;
                }
            }
        }
        else if(!chaotic && (spell_timers[index].getElapsedTime().asSeconds() >= 500 || first_cast[index])){
            first_cast[index] = false;
            controlled_tp = true;
            tp_left = 5;
        }
    }
}
}