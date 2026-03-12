#include <SFML/Graphics.hpp>
#include <vector>
#include <map>
#include <iostream>

#include "../create_mode/create_mode.hpp"
#include "../maths.hpp"
#include "inventory.hpp"

/*Handles all items that can be used by the player and stored in their inventory*/

namespace GH{
namespace INV{

    sf::Texture texture;
    std::vector<sf::IntRect> texture_rects;
    sf::RectangleShape* locked_item;

    std::vector<item> loaded_items;

    std::vector<sf::RectangleShape> slots = {
        sf::RectangleShape({50,50}),
        sf::RectangleShape({50,50}),
        sf::RectangleShape({50,50})
    };
    std::vector<std::string> in_slots = {"Blank", "Blank", "Blank"};


    int selected_slot = 0;
    bool locked_on = false;

    void setup(){//initial set up
        texture.loadFromFile(ASSETS_DIR"/textures/Equipment.png");
        for(int row = 0 ; row < 4 ; row++){
            for(int column = 0 ; column < 4 ; column++){
                texture_rects.push_back(sf::IntRect(16 * column, 16 * row, 16, 16));
            }
        }
        for(auto& slot : slots){
            slot.setTexture(&texture);
            slot.setTextureRect(texture_rects[15]);
        }
    }

    void align_slots(sf::RenderWindow& window, sf::View& view){//align the slots every frame to the view's position
        sf::FloatRect viewport = view.getViewport();
        sf::Vector2f view_topleft(viewport.left, viewport.top);
        sf::Vector2f world_topleft = window.mapPixelToCoords(sf::Vector2i(view_topleft.x * window.getSize().x, view_topleft.y * window.getSize().y), view);
        
        for(int i = 0 ; i < 3 ; i++){
            slots[i].setPosition({world_topleft.x + (i*60), world_topleft.y+25});
        }
    }

    void load_in_item(std::string type, sf::Vector2f pos){//load an item into the map 
        std::map<std::string,int> indexes = {{"EMF",0}, {"Mirror",1}, {"Bell",2}, {"Voice",3}, {"Lantern",4}, {"Camera",9}, {"UV",10}, {"Chalk",11}, {"Mirror_trapped",12}, {"Mirror_broken",13}};

        item temp;
        temp.shape.setSize({50,50});
        temp.shape.setPosition(pos);
        temp.in_inventory = false;
        temp.shape.setTexture(&texture);
        temp.name = type;
        temp.rect = texture_rects[indexes[type]];
        temp.shape.setTextureRect(temp.rect);
        temp.shape.setOrigin({temp.shape.getSize().x/2, temp.shape.getSize().y/2});
        if(indexes[type] == 10){
            temp.has_light = true;
            temp.light.setRange(100);
            temp.light_filter.setRadius(100);
            temp.light_filter.setOrigin({temp.light_filter.getRadius(), temp.light_filter.getRadius()});
            temp.light_filter.setFillColor(sf::Color(255,0,255,60));
        }
        else if(indexes[type] == 4){
            temp.has_light = true;
            temp.light.setRange(100);
            temp.light_filter.setRadius(100);
            temp.light_filter.setOrigin({temp.light_filter.getRadius(), temp.light_filter.getRadius()});
            temp.light_filter.setFillColor(sf::Color(255,0,0,60));
        }
        loaded_items.emplace_back(temp);
    }

    void pick_up(sf::RectangleShape& locked_item){//check if the player can pick up an item and if so, put it in the next empty inventory slot
        for(int i = 0 ; i < loaded_items.size() ; i++){
            if(loaded_items[i].shape.getGlobalBounds().intersects(locked_item.getGlobalBounds()) && !loaded_items[i].in_inventory){
                if(in_slots[selected_slot] == "Blank"){
                    loaded_items[i].in_inventory = true;
                    in_slots[selected_slot] = loaded_items[i].name;
                    slots[selected_slot].setTextureRect(loaded_items[i].rect);
                    break;
                }
                else{
                    if(in_slots[(selected_slot+1)%3] == "Blank"){
                        loaded_items[i].in_inventory = true;
                        in_slots[(selected_slot+1)%3] = loaded_items[i].name;
                        slots[(selected_slot+1)%3].setTextureRect(loaded_items[i].rect);
                        break;
                    }
                    else if(in_slots[(selected_slot+2)%3] == "Blank"){
                        loaded_items[i].in_inventory = true;
                        in_slots[(selected_slot+2)%3] = loaded_items[i].name;
                        slots[(selected_slot+2)%3].setTextureRect(loaded_items[i].rect);
                        break;
                    }
                }
            }
        }
    }

    void drop(sf::RectangleShape& locked_item){//drop the item the player is currently holding
        if(in_slots[selected_slot] != "Blank"){
            for(auto& item : loaded_items){
                if(item.in_inventory && item.name == in_slots[selected_slot]){
                    item.in_inventory = false;
                    item.shape.setPosition(locked_item.getPosition());
                    in_slots[selected_slot] = "Blank";
                    slots[selected_slot].setTextureRect(texture_rects[15]);
                    break;
                }
            }
        }
    }

    void slot_change(int num){//change the players selected slot
        slots[selected_slot].setFillColor(sf::Color(255,255,255,50));
        switch(num){
            case 1:
              selected_slot = 0;
              break;
            case 2:
              selected_slot = 1;
              break;
            case 3:
              selected_slot = 2;
              break;
            default:
              break;
        }
        slots[selected_slot].setFillColor(sf::Color(255,255,255,255));
    }
    
    void move_item(sf::RenderWindow& window, std::map<int,GH::CREATE::obj> walls, std::map<int,GH::CREATE::obj> objects){//move an item with the players mouse if the player is trying to 
        sf::Vector2f m_pos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        if(!locked_on)
            for(auto& item : loaded_items){
                if(item.shape.getGlobalBounds().contains(m_pos) && !item.in_inventory){
                    locked_on = true;
                    locked_item = &item.shape;
                    break;
                }
            }
            if(!locked_on){
                for(auto& object : objects){
                    if(object.second.shape.getGlobalBounds().contains(m_pos) && !object.second.has_collision){
                        locked_on = true;
                        locked_item = &object.second.shape;
                        break;
                    }
                }
            }
        else{
            sf::Vector2f diff = m_pos - locked_item->getPosition();
            if(abs(diff.x) < 200 && abs(diff.y) < 200){locked_item->move({diff.x/10,diff.y/10});}
            else{locked_on = false;}
            
            sf::FloatRect item_bounds = locked_item->getGlobalBounds();
            for(auto& wall : walls){
                sf::FloatRect bounds = wall.second.hitbox;
                //Calculate overlap on each axis
                float x_overlap = std::min(item_bounds.left + item_bounds.width, bounds.left + bounds.width) - std::max(item_bounds.left, bounds.left);
                float y_overlap = std::min(item_bounds.top + item_bounds.height, bounds.top + bounds.height) - std::max(item_bounds.top, bounds.top);
                //use the overlap sizes to determine which way to prevent movement
                if (x_overlap > 0 && y_overlap > 0) {
                    if (x_overlap < y_overlap) {//prevent movement on the X axis
                        if(item_bounds.left < bounds.left) {
                            locked_item->move(-x_overlap, 0);
                        } 
                        else{
                            locked_item->move(x_overlap, 0);
                        }
                    } 
                    else{//prevent movement on the Y axis
                        if (item_bounds.top < bounds.top) {
                            locked_item->move(0, -y_overlap);
                        } 
                        else{
                            locked_item->move(0, y_overlap);
                        }
                    }
                }
            }
        }
    }

    void blow_lantern(item& lantern, bool blow, std::string ghost){//happens when the ghost uses a lantern, if it doesnt have the evidence for it the color will change based on the ghosts tier, through a merge and binary sort to see if its in a tier list
        if(blow){
            std::cout<<"blowing out"<<std::endl;
            lantern.shape.setTextureRect(texture_rects[8]);
            lantern.has_light = false;
            lantern.name = "Lantern blown";
            std::cout<<"blowing out"<<std::endl;
        }
        else{
            std::vector<std::string> tier1 = {"Spirit", "Ghoul", "Phantom", "Grey Lady", "Revenant", "Imp", "Shade", "Myling", "Doven", "Funnel"};
            std::cout<<"1"<<std::endl;
            tier1 = GH::MATH::merge_sort(tier1);
            std::cout<<"2"<<std::endl;
            if(GH::MATH::binary_search(tier1, 0, 9, ghost)){
                std::cout<<"3"<<std::endl;
                lantern.name = "Lantern green";
                lantern.shape.setTextureRect(texture_rects[5]);
                lantern.light_filter.setFillColor(sf::Color(0,255,0,80));
            }
            
            else{
                std::vector<std::string> tier2 = {"Tsuk", "Preta", "Poltergeist", "Moroi", "Wisp", "Wraith", "Onryo", "Green Lady"};
                tier2 = GH::MATH::merge_sort(tier2);
                if(GH::MATH::binary_search(tier2, 0, 7, ghost)){
                    lantern.name = "Lantern orange";
                    lantern.shape.setTextureRect(texture_rects[6]);
                    lantern.light_filter.setFillColor(sf::Color(255,165,0,80));
                }
                else{
                    std::vector<std::string> tier3 = {"Dalgyal Guishin", "Demon", "Oni", "Red Lady"};
                    tier3 = GH::MATH::merge_sort(tier3);
                    if(GH::MATH::binary_search(tier3, 0, 3, ghost)){
                        lantern.name = "Lantern red";
                        lantern.shape.setTextureRect(texture_rects[7]);
                        lantern.light_filter.setFillColor(sf::Color(255,0,0,80));
                    }
                }
            }
        }
    }

    void align_lights(candle::LightingArea& fog){//align lights from objects to their objects
        for(auto& light_source : loaded_items){
            if(light_source.has_light && !light_source.in_inventory){
                light_source.light.setPosition(light_source.shape.getPosition());
                fog.draw(light_source.light);
            }
        }
    }

    bool check_uv(sf::RectangleShape& UV){//check if a uv light intersects the ghosts uv
        for(auto& equipment : loaded_items){
            if(equipment.name == "UV"){ 
                if(equipment.light_filter.getGlobalBounds().intersects(UV.getGlobalBounds())){
                    return true;
                }
            }
        }
        return false;
    }

    void reset(){//reset the invenotry and items at the end of a game
        for(auto& slot : slots){
            slot.setTextureRect(texture_rects[15]);
        }
        for(auto& slot : in_slots){
            slot = "Blank";
        }
        selected_slot = 0;
        locked_on = false;
        locked_item = nullptr;
        loaded_items = {};
    }
}
}