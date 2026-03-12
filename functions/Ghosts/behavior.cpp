#include <SFML/Graphics.hpp>
#include <map>
#include <iostream>
#include <random>
#include <Candle/Candle.hpp>


#include "behavior.hpp"

namespace GH{
namespace GHOST{
namespace BEHAV{
    std::random_device rd;                      
    std::mt19937 gen(rd());                     
    sf::Vector2f goal = {999999,999996};
    bool eh_close_enough = false;
    std::string current = "NILL";
    int ability_value = 100;
    float event = -1;
    float h_event = -1;
    float hunt_value = 0;
    float activity_value = 0;
    std::string facing = "S";
    sf::Clock flicker_clock;
    sf::Clock full_wander_clock;
    int flicker_time = 0;
    int flicker_level = 0;
    int walk_index = -1;
    std::map<std::string,int> events_map = {{"sound",200}, {"haunt",200}, {"interact",200}, {"evidence",200}, {"communicate",200}};
    std::vector<std::string> weighted_random;

    bool currently_throwing = false;
    int current_index_throwing;
    sf::Vector2f throw_goal;
    
    void event_tick(sf::Time delta, float sanity, std::string ghost_type, std::string prev_behav){//checks when the ghost can do something, using a randomly generated number that another number must be above to initiate, this number increments every second by a variable amount
        if(current == "NILL"){
            if(event == -1){
                std::uniform_int_distribution<> chance(1, ability_value);
                event = chance(gen);
            }
            if(event <= activity_value){
                activity_value = 0;
                event = -1;
                
                std::uniform_int_distribution<> ran(0, 1000);
                for(auto e : events_map){
                    for(int i = 0 ; i < e.second ; i++){
                        weighted_random.push_back(e.first);
                    }
                }
                current = weighted_random[ran(gen)];
                std::cout<<current<<std::endl;
                weighted_random = {};
                for(auto& e : events_map){
                    if(e.first == current){
                        e.second -= 20;
                    }
                    else{
                        e.second += 5;
                    }
                }

                std::string more_likely = "N";

                if(ghost_type == "Myling"){
                    more_likely = "sound";
                }
                
                if(more_likely != "N" && prev_behav != more_likely){
                    for(auto& e : events_map){
                        if(e.first == more_likely){
                            e.second += 12;
                        }
                        else{
                            e.second -= 3;
                        }
                    }
                }
               
            }
            float sanity_mult = 0.45;
            if(ghost_type == "Preta"){
                sanity_mult += 0.35;
            }
            activity_value += 1 + ((100 - sanity)*sanity_mult);
        }
    }

    void hunt_tick(sf::Time delta, float sanity){//same as event tick but for the ghost hunting
        std::cout<<"h_event: "<<h_event<<"  hunt_value: "<<hunt_value<<std::endl;
        if(current == "NILL"){
            if(h_event == -1){
                std::uniform_int_distribution<> chance(1, ability_value);
                h_event = chance(gen);
            }
            if(h_event <= hunt_value){
                hunt_value = 0;
                h_event = -1;
                current = "hunt";
            }
            hunt_value += (((100 - sanity)*0.03));
        }
    }

    std::string wander(float speed, sf::RectangleShape& sprite, std::vector<sf::CircleShape>& walk_path, sf::RectangleShape& chosen_room, sf::Time delta, std::map<int,GH::CREATE::obj> rooms, bool full_wander){//used for the ghost to wander, it has 5 circles it follows, each circle changing position with a chance of one moving outside the ghost room
        if((eh_close_enough || goal == sf::Vector2f(999999,999996))){
            walk_index = (walk_index + 1)%5;

            int move;
            if(walk_index == 0){
                move = 4;
            }
            else{
                move = walk_index - 1;
            }

            std::uniform_int_distribution<> leave_room(1,20);
            int new_room = -1;
            if(leave_room(gen) == 5 || full_wander){
                std::uniform_int_distribution<> nm(0,rooms.size()-1);
                new_room = nm(gen);
            }
            
            int minx,miny,maxx,maxy;
 
            if(new_room == -1){
                int x_1 = chosen_room.getTransform().transformPoint(chosen_room.getPoint(0)).x;
                int x_2 = chosen_room.getTransform().transformPoint(chosen_room.getPoint(1)).x;
                int y_1 = chosen_room.getTransform().transformPoint(chosen_room.getPoint(0)).y;
                int y_2 = chosen_room.getTransform().transformPoint(chosen_room.getPoint(2)).y;
                if(x_1 < x_2){minx = x_1; maxx = x_2;}
                else{minx = x_2; maxx = x_1;}
                if(y_1 < y_2){miny = y_1; maxy = y_2;}
                else{miny = y_2; maxy = y_1;}
            }
            else{
                int x_1 = rooms[new_room].shape.getTransform().transformPoint(rooms[new_room].shape.getPoint(0)).x;
                int x_2 = rooms[new_room].shape.getTransform().transformPoint(rooms[new_room].shape.getPoint(1)).x;
                int y_1 = rooms[new_room].shape.getTransform().transformPoint(rooms[new_room].shape.getPoint(0)).y;
                int y_2 = rooms[new_room].shape.getTransform().transformPoint(rooms[new_room].shape.getPoint(2)).y;
                if(x_1 < x_2){minx = x_1; maxx = x_2;}
                else{minx = x_2; maxx = x_1;}
                if(y_1 < y_2){miny = y_1; maxy = y_2;}
                else{miny = y_2; maxy = y_1;}               
            }

            std::uniform_int_distribution<> dist_x(minx, maxx);
            std::uniform_int_distribution<> dist_y(miny, maxy);

            walk_path[move].setPosition({dist_x(gen),dist_y(gen)});

            goal = walk_path[walk_index].getPosition();

        }
        else{
            facing = "";
            sf::Vector2f diff = goal - sprite.getPosition();
            sf::Vector2f velocity = {0.f, 0.f};

            if (std::abs(diff.x) > 1) {
                velocity.x = (diff.x > 0) ? speed : -speed;
                facing += (diff.x > 0) ? "E" : "W";
            }

            if (std::abs(diff.y) > 1) {
                velocity.y = (diff.y > 0) ? speed : -speed;
                facing += (diff.y > 0) ? "S" : "N";
            }

            if (velocity.x != 0 || velocity.y != 0) {
                sprite.move(velocity * delta.asSeconds());
            }

        }
        if(abs(goal.x - sprite.getPosition().x) < 20 && abs(goal.y - sprite.getPosition().y < 20) || (full_wander && full_wander_clock.getElapsedTime().asSeconds() >= 5)){
            eh_close_enough = true;
            full_wander_clock.restart();
            return "NILL";
        }
        else{
            eh_close_enough = false;
        }
        return facing;
        
    }

    void flicker(sf::RectangleShape& sprite){
        if(flicker_time == 0){
            std::uniform_int_distribution<> dist(100,400);
            flicker_time = dist(gen);
            flicker_time += ((flicker_time/4)*flicker_level);
            flicker_clock.restart();
        }
        if(flicker_clock.getElapsedTime().asMilliseconds() >= flicker_time){
            if(sprite.getFillColor().a == 175){
                sprite.setFillColor(sf::Color(85,255,230,0));
            }
            else{
                sprite.setFillColor(sf::Color(85,255,230,175));
            }
            flicker_time = 0;
        }

    }

    int throw_object(std::map<int,GH::CREATE::obj>& objects, sf::CircleShape radius, sf::Time delta, std::map<int,GH::CREATE::obj>& walls, std::map<int,GH::CREATE::obj>& floors, bool& can_UV) {// Calculate and execute the math for throwing an object
        if(!currently_throwing && current == "interact"){
            can_UV = true;
            // this for loop gets all the objects that can be thrown
            std::vector<int> throwable_objects;
            for(auto& object : objects){
                if(object.second.throwable && radius.getGlobalBounds().intersects(object.second.shape.getGlobalBounds())){
                    throwable_objects.push_back(object.first);
                }
            }
            
            if(!throwable_objects.empty()){
                std::uniform_int_distribution<> index(0, throwable_objects.size()-1);
                current_index_throwing = throwable_objects[index(gen)]; // choose a random object to throw
                currently_throwing = true;
                int floor_index;
                bool empty_space;
                for(auto& floor : floors){
                    if(floor.second.shape.getGlobalBounds().intersects(objects[current_index_throwing].shape.getGlobalBounds())){
                        int x_1 = floor.second.shape.getTransform().transformPoint(floor.second.shape.getPoint(0)).x;
                        int x_2 = floor.second.shape.getTransform().transformPoint(floor.second.shape.getPoint(1)).x;
                        int y_1 = floor.second.shape.getTransform().transformPoint(floor.second.shape.getPoint(0)).y;
                        int y_2 = floor.second.shape.getTransform().transformPoint(floor.second.shape.getPoint(2)).y;
                        int minx,miny,maxx,maxy;
                        if(x_1 < x_2){minx = x_1; maxx = x_2;}
                        else{minx = x_2; maxx = x_1;}
                        if(y_1 < y_2){miny = y_1; maxy = y_2;}
                        else{miny = y_2; maxy = y_1;}

                        std::uniform_int_distribution<> dist_x(minx, maxx);
                        std::uniform_int_distribution<> dist_y(miny, maxy);

                        
                        for(int i = 0 ; i < 100 ; i++){
                            empty_space = true;
                            int x = dist_x(gen);
                            int y = dist_y(gen);
                            sf::FloatRect space = {x,y,objects[current_index_throwing].shape.getSize().x, objects[current_index_throwing].shape.getSize().y};
                            for(auto& object : objects){
                                if(object.second.shape.getGlobalBounds().intersects(space)){
                                    empty_space = false;
                                    break;
                                }
                            }
                            if(empty_space){
                                for(auto& wall : walls){
                                    if(wall.second.shape.getGlobalBounds().intersects(space)){
                                        empty_space = false;
                                        break;
                                    }
                                }
                                if(empty_space){
                                    throw_goal = {x,y};
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
            else{
                current = "NILL";
                event = 0;
            }
        }
        else if(currently_throwing && current == "interact"){
            can_UV = false;
            sf::Vector2f diff = throw_goal - objects[current_index_throwing].shape.getPosition();
            sf::Vector2f velocity = {0.f, 0.f};

            float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y); // get distance

            if(distance > 10.0f){
                sf::Vector2f direction = diff / distance;
                float speed = distance * 0.8; // allows the object to slow down the closer it is to its goal 
                velocity = direction * speed;
                objects[current_index_throwing].shape.move(velocity * delta.asSeconds());
                for(auto& wall : walls){
                    if(wall.second.shape.getGlobalBounds().intersects(objects[current_index_throwing].shape.getGlobalBounds())){
                        if(wall.second.index == -1){
                            objects[current_index_throwing].shape.move(-velocity * delta.asSeconds());
                            current = "NILL";
                            currently_throwing = false;
                        }
                        else{
                            if(objects[current_index_throwing].shape.getGlobalBounds().intersects(wall.second.shape.getGlobalBounds())){
                                if(throw_goal.y > objects[current_index_throwing].shape.getPosition().y){
                                    throw_goal.y = objects[current_index_throwing].shape.getPosition().y + 5;
                                }
                                else{
                                    throw_goal.y = objects[current_index_throwing].shape.getPosition().y - 5;
                                }
                            }
                        }
                    }
                }
            }
            else{
                currently_throwing = false;
                current = "NILL";
            }
        }
        return current_index_throwing;
    }

    std::string chase(sf::RectangleShape& ghost, sf::RectangleShape& player, sf::Time delta){//overruns wander when the ghost sees the player and starts chasing
        
        facing = "";
        sf::Vector2f diff = player.getPosition() - ghost.getPosition();
        
        sf::Vector2f velocity = {0.f, 0.f};

        if(std::abs(diff.x) > 1){
            velocity.x = (diff.x > 0) ? 250.f : -250.f;
            facing += (diff.x > 0) ? "E" : "W";
        }

        if(std::abs(diff.y) > 1){
            velocity.y = (diff.y > 0) ? 250.f : -250.f;
            facing += (diff.y > 0) ? "S" : "N";
        }

        if (velocity.x != 0 || velocity.y != 0){
            
            ghost.move(velocity * delta.asSeconds());
        }
        
        return facing;

    }

    void ghost_ability_check(bool in_light, bool seen_player, std::string ghost_type, float& speed, float sanity){//checking if any of the ghosts abilities need to be initiated
        speed = 200;
        if(ghost_type == "Dalgyal Guishin"){
            if(seen_player){
                speed += 200;
            }
            else if(current == "hunt"){
                speed -= 150;
            }
        }
        if(ghost_type == "Ghoul" && in_light){
            speed -= 100;
            flicker_level = 2;
        }
        else if(!in_light){
            flicker_level = 0;
        }

        if(ghost_type == "Oni"){
            speed += (100 - (sanity*0.85));
        }
    }
}
}
}