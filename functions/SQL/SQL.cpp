#include <pqxx/pqxx>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <map>
#include "SQL.hpp"
#include "../struct.hpp"

/*This file contains all functions interacting with the database, the databsed stores maps,objects and players
  
  Maps use a one to many relationship, with all object tables using map_id as a foreign key to link to the correct map described in the map table
*/

namespace GH{
namespace SQL{

    std::string load_foreign_map(int map_id, std::string share_code){
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);    
        
        pqxx::result r = work.exec("SELECT share_code, name FROM maps WHERE id = $1", {map_id});
        if(r[0]["share_code"].get<std::string>().value() == share_code){
            return r[0]["name"].get<std::string>().value();
        }
        else{
            return "0000000000000000000000000000000000000000";
        }
    }

    void change_share_code(int map_id, std::string share_code){
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);
        work.exec("UPDATE maps SET share_code = $1 WHERE id = $2", {share_code,map_id});
        work.commit();
    }
    

    sf::Texture load_texture(int theme, int index){ //load a texture from the databse based on which theme it is from and its index in that theme
        std::cout<<"1"<<std::endl;
        sf::Texture test;
        
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        
        pqxx::work w(c);
        
        c.prepare("get_img", "SELECT img FROM Textures WHERE theme = $1 AND index = $2");
        
        pqxx::result r = w.exec(pqxx::prepped{"get_img"}, {theme,index});
        w.commit();

        pqxx::binarystring blob(r[0]["img"]);

        test.loadFromMemory(blob.data(), blob.size());
        
        
        return test;
    }

    pqxx::result load_all_theme(std::string theme){//get all the textures from a theme for displaying in the menu
        std::cout<<"2"<<std::endl;
        pqxx::result r;
        
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        
        pqxx::work w(c);
        
        
        
        r = w.exec("SELECT img FROM Textures WHERE theme = (SELECT id FROM Themes WHERE name = $1)", pqxx::params(theme));
        w.commit();

        
        return r;
    }
    
    void delete_map_data(int map_id){//delete a maps data from the databse to allow it to be overwritten with a new save
        std::cout<<"3"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        
        

        pqxx::work work(c);
        work.exec("DELETE FROM objects WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM walls WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM floors WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM rooms WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM hiding WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM ambient_zones WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM light_systems WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM lights WHERE map_id = $1", pqxx::params(map_id));
        work.exec("DELETE FROM switches WHERE map_id = $1", pqxx::params(map_id));
        work.commit();
    }

    void save_map_size(int map_id, int objects, int walls, int floors, int rooms, int light_systems, int hiding, int ambient){ //save the amount of each object type in the map
        std::cout<<"4"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);
 
        work.exec("UPDATE maps SET object_amount = $1 WHERE id = $2", pqxx::params(objects, map_id));
        work.exec("UPDATE maps SET walls_amount = $1 WHERE id = $2", pqxx::params(walls, map_id));
        work.exec("UPDATE maps SET floors_amount = $1 WHERE id = $2", pqxx::params(floors, map_id));
        work.exec("UPDATE maps SET rooms_amount = $1 WHERE id = $2", pqxx::params(rooms, map_id));
        work.exec("UPDATE maps SET light_system_amount = $1 WHERE id = $2", pqxx::params(light_systems, map_id));
        work.exec("UPDATE maps SET hiding_amount = $1 WHERE id = $2", pqxx::params(hiding, map_id));
        work.exec("UPDATE maps SET ambient_amount = $1 WHERE id = $2", pqxx::params(ambient, map_id));

        work.commit();
    }

    void save_asset(std::string asset_type, GH::CREATE::obj& asset, int map_id, int asset_index){//save an asset in the correct table with all the correct attributes
        std::cout<<"6"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);
        pqxx::params asset_values;
           asset_values.append(map_id);
           asset_values.append(asset_index);
           asset_values.append(asset.shape.getPosition().x);
           asset_values.append(asset.shape.getPosition().y);
           asset_values.append(asset.shape.getSize().x);
           asset_values.append(asset.shape.getSize().y);

        if(asset_type == "object"){
           asset_values.append(asset.theme);
           asset_values.append(asset.index);
           asset_values.append(asset.has_collision);
           asset_values.append(asset.blocks_light);
           asset_values.append(asset.throwable);
           asset_values.append(asset.hitbox.left);
           asset_values.append(asset.hitbox.top);
           asset_values.append(asset.hitbox.width);
           asset_values.append(asset.hitbox.height);
           work.exec("INSERT INTO Objects (map_id,index,posX,posY,sizeX,sizeY,theme_id,texture_id,collision,blocks_light,throwable,left_h,top_h,width_h,height_h) VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15)", asset_values);
        }
        else if(asset_type == "wall"){
            asset_values.append(asset.index);
            asset_values.append(asset.hitbox.left);
            asset_values.append(asset.hitbox.top);
            asset_values.append(asset.hitbox.width);
            asset_values.append(asset.hitbox.height);
            asset_values.append(asset.rect.width);
            asset_values.append(asset.rect.height);
            work.exec("INSERT INTO walls (map_id,index,posx,posy,sizex,sizey,texture_id,left_h,top_h,width_h,height_h,rect_width,rect_height) VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9,$10,$11,$12,$13)", asset_values);
            
        }
        else if(asset_type == "floor"){
            asset_values.append(asset.index);
            asset_values.append(asset.rect.width);
            asset_values.append(asset.rect.height);
            work.exec("INSERT INTO floors (map_id,index,posx,posy,sizex,sizey,texture_id,rect_width,rect_height) VALUES ($1,$2,$3,$4,$5,$6,$7,$8,$9)", asset_values);
        }
        else if(asset_type == "hiding"){
            work.exec("INSERT INTO hiding (map_id,index,posx,posy,sizex,sizey) VALUES ($1,$2,$3,$4,$5,$6)", asset_values);
        }
        else if (asset_type == "ambient"){
            work.exec("INSERT INTO ambient_zones (map_id,index,posx,posy,sizex,sizey) VALUES ($1,$2,$3,$4,$5,$6)", asset_values);
        }
        else{
            work.exec("INSERT INTO rooms (map_id,index,posx,posy,sizex,sizey) VALUES ($1,$2,$3,$4,$5,$6)", asset_values);
        }
        work.commit();
    }
    
    pqxx::result load_asset(int map_id, std::string asset_type){//load an asset from a table based on its type and what map it comes from
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);
        pqxx::result results;
        if(asset_type == "objects"){
            results = work.exec("SELECT * FROM objects WHERE map_id = $1 ORDER BY index", pqxx::params(map_id));
        }
        else if(asset_type == "walls"){
            results = work.exec("SELECT * FROM walls WHERE map_id = $1 ORDER BY index", pqxx::params(map_id));
        }
        else if(asset_type == "floors"){
            results = work.exec("SELECT * FROM floors WHERE map_id = $1 ORDER BY index", pqxx::params(map_id));
        }
        else if(asset_type == "rooms"){
            results = work.exec("SELECT * FROM rooms WHERE map_id = $1 ORDER BY index", pqxx::params(map_id));
        }
        else if(asset_type == "hiding"){
            results = work.exec("SELECT * FROM hiding WHERE map_id = $1 ORDER BY index", pqxx::params(map_id));
        }
        else if(asset_type == "ambient"){
            results = work.exec("SELECT * FROM ambient_zones WHERE map_id = $1 ORDER BY index", pqxx::params(map_id));
        }
        work.commit();

        return results;
    }
    
    std::vector<int> get_map_size(int map_id){//get the size of the chosen map
        std::cout<<"8"<<std::endl;
        std::cout<<std::to_string(map_id)<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);
        
        pqxx::result results = work.exec("SELECT object_amount, walls_amount, floors_amount, rooms_amount, light_system_amount, hiding_amount, ambient_amount FROM maps WHERE id = $1", pqxx::params(map_id));
        
        work.commit();
 
        return std::vector<int>({results[0]["object_amount"].get<int>().value(), results[0]["walls_amount"].get<int>().value(), results[0]["floors_amount"].get<int>().value(), results[0]["rooms_amount"].get<int>().value(), results[0]["light_system_amount"].get<int>().value(), results[0]["hiding_amount"].get<int>().value(), results[0]["ambient_amount"].get<int>().value()});
    }
    
    pqxx::result get_map_names(std::string author){//get the maps created by <author>
        std::cout<<"9"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);

        pqxx::result result = work.exec("SELECT name, id, share_code FROM maps WHERE author = $1",pqxx::params(author));
        work.commit();

        return result;
    }

    void save_light_systems(std::map<int,GH::CREATE::light_system>& light_systems, int map_id){//specific function for saving light systems as there are seperate tables for systems, lights and switches 
        std::cout<<"10"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);

        c.prepare("light_system", "INSERT INTO light_systems (map_id, index) VALUES ($1,$2)");
        c.prepare("light", "INSERT INTO lights (map_id, light_system_id, posx, posy, range, intensity) VALUES ($1,$2,$3,$4,$5,$6)");
        c.prepare("switch", "INSERT INTO switches (map_id, light_system_id, posx, posy) VALUES ($1,$2,$3,$4)");
        pqxx::params light_system_params;
        pqxx::params light_params;
        pqxx::params switch_params;
        for(auto& light_system : light_systems){
            light_system_params = {};
            light_system_params.append(map_id);
            light_system_params.append(light_system.first);
            work.exec(pqxx::prepped{"light_system"}, light_system_params);

            for(auto& light : light_system.second.lights){
                light_params = {};
                light_params.append(map_id);
                light_params.append(light_system.first);
                light_params.append(light.getPosition().x);
                light_params.append(light.getPosition().y);
                light_params.append(light.getRange());
                light_params.append(light.getIntensity());
                work.exec(pqxx::prepped{"light"}, light_params);
            }

            for(auto& l_switch : light_system.second.light_switches){
                switch_params = {};
                switch_params.append(map_id);
                switch_params.append(light_system.first);
                switch_params.append(l_switch.getPosition().x);
                switch_params.append(l_switch.getPosition().y);
                work.exec(pqxx::prepped{"switch"}, switch_params);
            }

            work.commit();
        }
    }
    
    std::pair<pqxx::result,pqxx::result> load_internal_system(int map_id, int light_system_id){//load all lights and switches from a light system in a map
        std::cout<<"11"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);
    
        pqxx::result lights = work.exec("SELECT * FROM lights WHERE map_id = $1 and light_system_id = $2", pqxx::params(map_id, light_system_id));
        pqxx::result switches = work.exec("SELECT * FROM switches WHERE map_id = $1 and light_system_id = $2", pqxx::params(map_id, light_system_id));

        return {lights,switches};
    }

    std::string reg(char u[], char p[]){//register a new user
        std::cout<<"12"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");

        std::string username(u);
        std::string password(p);
        
        pqxx::work work(c);

        pqxx::result check = work.exec("SELECT * FROM users WHERE username = $1", pqxx::params(username));
        work.commit();
        
        if(!check.empty()){
            return "Username Taken";
        }
        if(username.size() < 6){
            return "Usernames must be 6 characters or more";
        }
        if(password.size() < 8){
            return "Password must be 8 characters or more";
        }


        if (username.find_first_not_of("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234567890_") != std::string::npos){
            return "Only special character allowed is '_'";
        }

        work.exec("INSERT INTO users (username,password) VALUES ($1, encrypt($2))", pqxx::params(username,password));
        pqxx::result result = work.exec("SELECT id FROM users WHERE username = $1", pqxx::params(username));
        work.commit();

        int id = result[0]["id"].get<int>().value();

        work.exec("INSERT INTO userlevel (level,user_id) VALUES (0,$1)", pqxx::params(id));
        work.commit();

        return "";
    }

    std::string log_in(char u[], char p[]){//log in a user
        std::cout<<"13"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");

        std::string username(u);
        std::string password(p);
        
        pqxx::work work(c);

        pqxx::result user = work.exec("SELECT * FROM users WHERE username = $1", pqxx::params(username));
        work.commit();
        if(user.empty()){
            return "Username not found";
        }
        

        pqxx::result encrypt = work.exec("SELECT encrypt($1)", pqxx::params(password));
        work.commit();

        if(encrypt[0]["encrypt"].get<std::string>().value() == user[0]["password"].get<std::string>().value()){
            return "Hello, " + username + "!";
        }
        else{
            return "Incorrect password";
        }

    }

    pqxx::result get_user_info(char u[]){//get the info of a logged in user
        std::cout<<"14"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);
                                 
        std::string username(u);
        pqxx::result info = work.exec("SELECT users.username, users.id, userlevel.level FROM users INNER JOIN userlevel ON users.id = userlevel.user_id WHERE username = $1", pqxx::params(username));
        work.commit();
         
        return info;
    }

    void make_map(std::string user, int map_amount){//add a new map to the map table
        std::cout<<"15"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);

        work.exec("INSERT INTO maps (name, author) VALUES ($1,$2)", pqxx::params("custom map: " + std::to_string(map_amount), user));

        work.commit();
    }

    void change_map_name(int map_id, std::string name){//change a maps name
        std::cout<<"16"<<std::endl;
        pqxx::connection c("user= password= host=localhost port=5432 dbname=gh target_session_attrs=read-write");
        pqxx::work work(c);

        work.exec("UPDATE maps SET name = $1 WHERE id = $2;", pqxx::params(name,map_id));

        work.commit();
    }
    
}
}
