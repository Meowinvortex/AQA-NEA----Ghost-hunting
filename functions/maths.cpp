#include <SFML/Graphics.hpp>
#include <cmath>
#include <Candle/Candle.hpp>
#include <cstdlib>
#include <iostream>
#include <box2d/box2d.h>
#include <map>
#include <vector>
#include "struct.hpp"

#include "maths.hpp"

/*This file contains various functions with mathmatical operations used in various files across the game*/

namespace GH{
namespace MATH{

    static std::string facing = "Back";
    int running = 0;
    sf::Clock stamina_regen;

    void stamina_check(float& stamina, float& speed){ //checking if the player is running, handling stamina draining and regening
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::LShift) && stamina > 0 && running == 0){
            speed += 200;
            running = 1;
            stamina_regen.restart();
        }
        if(running == 1 && stamina_regen.getElapsedTime().asSeconds() >= 0.2){
            stamina -= 2 * std::floor(stamina_regen.restart().asSeconds()/0.2);
        }
        if((stamina <= 0 || !sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) && running == 1){
            if(stamina <= 0){
                stamina = 0;
                running = 2;
            }
            else{
                running = 0;
            }
            speed -= 200;
            stamina_regen.restart();
        }
        if(stamina < 100 && running != 1 && stamina_regen.getElapsedTime().asSeconds() >= 0.5){
            stamina += std::floor((stamina_regen.restart().asSeconds()/0.5)) * 2;
            if(stamina >= 100){
                stamina = 100;
                running = 0;
            }
        }
    }

    float getmouseang(sf::Vector2i mousePos, sf::Vector2f center) { //get the angle relative from the centre of the window and the mouse position
        const float PI = 3.14159265358979323846f;
        sf::Vector2f delta(mousePos.x - center.x, mousePos.y - center.y);
        float angleRad = std::atan2(delta.y, delta.x);
        float angleDeg = angleRad * 180.0f / PI;
        if(angleDeg < 0) angleDeg += 360.0f;
        
        return angleDeg;
    }

    bool prevent_collisions(std::vector<sf::RectangleShape>& hitboxes, sf::View& view, //prevent the player colliding from objects and walls
        sf::RectangleShape& body,
        sf::Time deltaTime, float speed,
        std::map<int, GH::CREATE::obj>& collision_sprites, sf::Vector2f previous_pos){
        
        bool collide;
        for (auto& sprite_pair : collision_sprites){
            collide = false;
            sf::FloatRect object_bounds = sprite_pair.second.hitbox;
            if(GH::MATH::rect_in_view(object_bounds, view) && sprite_pair.second.has_collision){
                for(auto& hitbox : hitboxes){
                    if(hitbox.getGlobalBounds().intersects(object_bounds)){
                        collide = true;
                        break;
                    }
                }
                if(collide){
                    if(object_bounds.intersects(hitboxes[0].getGlobalBounds())){
                        body.setPosition({body.getPosition().x, (object_bounds.top + object_bounds.height) - 20 + (body.getSize().y/2)});
                    }  
                    else if(object_bounds.intersects(hitboxes[1].getGlobalBounds())){
                        body.setPosition({body.getPosition().x, object_bounds.top - 5 - (body.getSize().y/2)});
                    } 
                    else if(object_bounds.intersects(hitboxes[2].getGlobalBounds())){
                        body.setPosition({(object_bounds.left+object_bounds.width) + 5 + (body.getSize().x/2), body.getPosition().y});
                    } 
                    else if(object_bounds.intersects(hitboxes[3].getGlobalBounds())){
                        body.setPosition({object_bounds.left - 5 - (body.getSize().x/2), body.getPosition().y});
                    } 
                    return true;
                }
            }
        }
        return false;
    }
    
    //Handles player movement, returning the way the player is facing
    std::tuple <std::string,std::string> check_velocity(sf::Time delta_time, float speed, int M_WIDTH, int M_HEIGHT, sf::RectangleShape& body, std::map<int, GH::CREATE::obj>& collision_sprites, std::vector<sf::RectangleShape>& hitboxes, sf::View& view, sf::Vector2f previous_pos){
        sf::Vector2f velocity = {0.f, 0.f};
        std::string mode;
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::W)){velocity.y -= speed; facing = "Forward";}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::S)){velocity.y += speed; facing = "Back";}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::A)){ velocity.x -= speed; facing = "Left";}
        if(sf::Keyboard::isKeyPressed(sf::Keyboard::D)){ velocity.x += speed; facing = "Right";}
        
        body.move(velocity*delta_time.asSeconds());

        if(velocity.x == 0.f && velocity.y == 0.f){
            mode = "idle";
        }
        else{
            mode = "moving";
        }
        return {facing, mode};
    }

    std::map<int, std::pair<sf::Vector2f, sf::Vector2f>> get_points(sf::RectangleShape& shape){//get point positions on a rectangle
        int point_amnt = 4;
        std::map<int, std::pair<sf::Vector2f, sf::Vector2f>> points_pos;
        for(size_t i = 0; i < point_amnt; ++i) {
            int next = (i + 1) % point_amnt;
            points_pos[i] = {shape.getTransform().transformPoint(shape.getPoint(i)),
                            shape.getTransform().transformPoint(shape.getPoint(next))};
        }
        return points_pos;
    }

    bool rect_in_view(const sf::FloatRect& rect, const sf::View& view){//check if the object is in view of the player, used for optimisising as objects that are out of view are not loaded
        sf::Vector2f center = view.getCenter();
        sf::Vector2f size = view.getSize();

        sf::FloatRect view_rect(
            center.x - size.x / 2.f,
            center.y - size.y / 2.f,
            size.x,
            size.y
        );

        return view_rect.intersects(rect);
    }


    std::vector<sf::Vector2f> vertex_hitbox(sf::RectangleShape& shape, sf::Texture& texture){
        // Get texture image
        sf::Image img = texture.copyToImage();
        sf::Vector2u texSize = img.getSize();
        sf::Vector2f shapeSize = shape.getSize();

        //Calculate scale factors (texture might be stretched in shape)
        sf::Vector2f scale = {shapeSize.x / texSize.x, shapeSize.y / texSize.y};

        //Find non-transparent bounds in texture space
        sf::Vector2f min = {texSize.x, texSize.y};
        sf::Vector2f max = {0, 0};

        for (unsigned int y = 0; y < texSize.y; y++) {
            for (unsigned int x = 0; x < texSize.x; x++) {
                if (img.getPixel(x, y).a == 255) { //Check for opaque pixels
                    if (x < min.x) min.x = x;
                    if (x > max.x) max.x = x;
                    if (y < min.y) min.y = y;
                    if (y > max.y) max.y = y;
                }
            }
        }

        //Convert texture bounds to shape coordinates
        float left = min.x * scale.x;
        float right = max.x * scale.x;
        float top = min.y * scale.y;
        float bottom = max.y * scale.y;

        //Get the base rectangles points
        sf::Vector2f basePoints[4] = {
            {left+2, top+2},
            {right+2, top+2},
            {right+2, bottom-76},
            {left+2, bottom-76}
        };

        //Transform each point
        std::vector<sf::Vector2f> hitbox(4);
        const sf::Transform& transform = shape.getTransform();
        for (int i = 0; i < 4; i++) {
            hitbox[i] = transform.transformPoint(basePoints[i]);
        }

        return hitbox;
    }

    std::vector<std::string> merge_sort(std::vector<std::string> unsorted){//merge sort algorithm
        std::vector<std::vector<std::string>> sub_lists;

        for(auto word : unsorted){  //Create a sublist for each individual word
            sub_lists.push_back({word});
        }

        while (sub_lists.size() > 1) {  //When there is only 1 list left in sublist that means all the sublists have been merged and sorted
            std::vector<std::vector<std::string>> merged_lists;  //All lists that have been created by sorting and merging in this loop moves to here
            for(int i = 0; i < sub_lists.size(); i += 2){  
                if(i + 1 < sub_lists.size()) {
            
                    std::vector<std::string> merged;  //The final list for each loop which will be both lists merged and sorted
                    auto list1 = sub_lists[i];  //Get the first list of this loop
                    auto list2 = sub_lists[i+1];  //Get the second list for this loop
                    int index1 = 0, index2 = 0;  //Indexes for both lists to know where in each list the sort is currently at

                    while(index1 < list1.size() && index2 < list2.size()){  //Iterate over each index in both lists and insert them in order into the merge list
                        if(list1[index1] < list2[index2]){
                            merged.push_back(list1[index1++]);
                        } 
                        else{
                            merged.push_back(list2[index2++]);
                        }
                    }
                    while(index1 < list1.size()) merged.push_back(list1[index1++]);  //Both these loop insert any left over numbers into the merge list if one list was smaller and cause the loop to stop early
                    while(index2 < list2.size()) merged.push_back(list2[index2++]);
                    merged_lists.push_back(merged);
                } 
                else{
                    merged_lists.push_back(sub_lists[i]);  //If theres an odd number push the last one in
                }
            }
            sub_lists = merged_lists; //Move over the newly merged lists to sub lists to be sorted and merged again
        }

        return sub_lists[0];
    }   

    bool binary_search(vector<std::string>& arr, int low, int high, std::string target){ //recursive algorithm for a binary search

        if(low > high){
            return false;
        }

        int mid = low + (high - low) / 2;

        if(arr[mid] == target){
            return true;
        }

        else if (arr[mid] < target) {
            return binary_search(arr, mid + 1, high,target); //Recursive the left side
        }

        else{
            return binary_search(arr, low, mid - 1,target);  //Recursive the right side
        }
    }

    sf::Vector2f cardinal_direction(sf::Vector2f player_pos, sf::Vector2f ghost_pos){//get the cardinal direction of the ghost and player, prevents the ghost from seeing the player through walls
        float dx = player_pos.x - ghost_pos.x;
        float dy = player_pos.y - ghost_pos.y;
        
        const float threshold = 5.0f;
        
        int x = 0;
        int y = 0;
        
        if(std::abs(dx) > threshold){
            x = (dx > 0) ? 1 : -1;
        }
        
        if(std::abs(dy) > threshold){
            y = (dy > 0) ? 1 : -1;
        }
        
        return {x,y};
    }
}
}