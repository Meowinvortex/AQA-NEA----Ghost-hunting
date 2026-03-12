
#include <variant>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <pqxx/pqxx>
#include <map>
#include "imgui/imgui.h"
#include "imgui-sfml-2.6.1/imgui-SFML.h"
#include <SFML/Graphics.hpp>

#include "start_menu.hpp"
#include "../SQL/SQL.hpp"

/*This files handles all the windows that appear for the player before they start a game*/

namespace GH{
namespace START{
    int current_skintone = 0;
    int chosen_map = -1;
    std::map<int,std::pair<std::string,std::string>> dev_maps;
    std::map<int,std::pair<std::string,std::string>> own_maps;
    char username[24];
    char password[128];
    char new_name[32];
    char import_id[256];
    char import_share_code[32];
    std::string log_in_msg = "";
    std::pair<std::string,int> logged_in_info = {"", -1};
    float user_level = 0;
    std::vector<std::string> slots = {" "," "," "};
    std::vector<sf::IntRect> hiero_rects;
    sf::Texture hiero_txt;
    sf::Sprite hiero_sprite;
    std::vector<sf::Sprite> slot_sprites = {sf::Sprite{}, sf::Sprite{},sf::Sprite{}};
    bool map_ownership = false;
    bool go_create = false;

    sf::Texture spell_sheet;
    std::map<std::string,sf::IntRect> spell_rects = {{"Speed",sf::IntRect(0,0,32,32)},{"Dash",sf::IntRect(32,0,32,32)},{"Basic protection",sf::IntRect(64,0,32,32)},
                                                    {"Chaotic tp",sf::IntRect(0,32,32,32)},{"Spirit form",sf::IntRect(32,32,32,32)},{"Protection",sf::IntRect(64,32,32,32)},
                                                    {"Controlled tp",sf::IntRect(0,64,32,32)},{"Disorientation",sf::IntRect(32,64,32,32)},{"Enhanced protection",sf::IntRect(64,64,32,32)}};


    std::map<std::string,int> equipment = {{"EMF",0}, {"Mirror",0}, {"Bell",0}, {"Voice",0}, {"Lantern",0}, {"Camera",0}, {"UV",0}, {"Chalk",0}};

    void set_up(){//initial setup
        spell_sheet.loadFromFile(ASSETS_DIR"/textures/spells.png");
        hiero_txt.loadFromFile(ASSETS_DIR"/textures/hieros.png");
        for(int i = 0 ; i < 3 ; i++){
            for(int j = 0 ; j < 3 ; j++){
                hiero_rects.push_back(sf::IntRect(0 + (j*32),0 + (i*31),32,32));
            }
        }
        hiero_sprite.setTexture(hiero_txt);

        for(auto& sprite : slot_sprites){
            sprite.setColor(sf::Color(255,255,255,0));
            sprite.setTexture(hiero_txt);
        }
    }

    void load_maps(){//load the developer maps and the currently logged in players map for selection
        dev_maps = {};
        own_maps = {};
        pqxx::result dev_results = GH::SQL::get_map_names("DEV");
        pqxx::result own_results = GH::SQL::get_map_names(logged_in_info.first);
        
        for(auto map : dev_results){
            dev_maps[map["id"].get<int>().value()] = {map["name"].get<std::string>().value(),map["share_code"].get<std::string>().value()};
        }
        for(auto map : own_results){
            own_maps[map["id"].get<int>().value()] = {map["name"].get<std::string>().value(),map["share_code"].get<std::string>().value()};
        }
    }

    int start_window(){//window for starting or editing the selected map
        ImGui::Begin("Start");
        if(ImGui::Button("Start", {100,50}) && chosen_map != -1){
            ImGui::End();
            go_create = false;
            return chosen_map;
        }
        if(chosen_map != - 1){
            if(map_ownership){
                if(ImGui::Button("Edit map", {100,50}) && chosen_map != -1){
                    ImGui::End();
                    go_create = true;
                    return chosen_map;
                }
                ImGui::Text(("Chosen map: " + own_maps[chosen_map].first).c_str());
                ImGui::Text(("Map ID: " + std::to_string(chosen_map)).c_str());
                ImGui::Text(("Share code: " + own_maps[chosen_map].second).c_str());
            }
            else{
                ImGui::Text(("Chosen map: " + dev_maps[chosen_map].first).c_str());
            }
            
        }

        if(map_ownership){
            ImGui::InputText("##1", new_name, 24);
            if(ImGui::Button("Change name", {100,75})){
                bool used = false;
                for(auto& map : own_maps){
                    if(map.second.first == std::string(new_name)){
                        strcpy(new_name, "Map name already used");
                        used = true;
                        break;
                    }
                }
                if(!used){
                    GH::SQL::change_map_name(chosen_map, std::string(new_name));
                    load_maps();
                }
            }
            ImGui::SameLine();
            if(ImGui::Button("Change share code", {150,75})){
                GH::SQL::change_share_code(chosen_map, std::string(new_name));
                own_maps[chosen_map].second = std::string(new_name);
                load_maps();
            }
        }
        
        ImGui::End();
        return -1;
    }

    void run_map_menu(){//menu for map selection
        ImGui::Begin("Choose map", nullptr, 
            ImGuiWindowFlags_NoResize);
            int i = 0;
            ImGui::Text("Official maps");
        for(auto map : dev_maps){
            if(ImGui::Button(map.second.first.c_str(), ImVec2(100,50))){
                chosen_map = map.first;
                map_ownership = false;
            }
            if((i+1) % 4 != 0){
                ImGui::SameLine();
            }
            i++;
        }
        ImGui::NewLine();
        ImGui::Text(("Personal maps (" + std::to_string(own_maps.size()) + "/5)").c_str());
        if(ImGui::Button("+", {50,50}) && own_maps.size() < 5){
            GH::SQL::make_map(logged_in_info.first, own_maps.size());
            load_maps();
        }
        for(auto map : own_maps){
            if(ImGui::Button(map.second.first.c_str(), ImVec2(100,50))){
                chosen_map = map.first;
                map_ownership = true;
            }
            if((i+1) % 4 != 0){
                ImGui::SameLine();
            }
            i++;
        }
        ImGui::NewLine();
        ImGui::Text(("Import map"));
        ImGui::Text("ID:");
        ImGui::InputText("##4", import_id, 256);
        ImGui::Text("Share code:");
        ImGui::InputText("##3", import_share_code, 32);
        if(ImGui::Button("Import map", ImVec2(100,75))){
            std::string import_name = GH::SQL::load_foreign_map(atoi(import_id), std::string(import_share_code));
            if(import_name != "0000000000000000000000000000000000000000"){
                chosen_map = atoi(import_id);
                map_ownership = false;
                dev_maps[chosen_map] = {import_name,""};
            }
        }
        ImGui::End();
    }

    std::unordered_map<std::string, std::variant<int, sf::Color, std::string>> run_avatar_menu(int current_skintone){//menu for avatar customisation
        ImGui::Begin("Avatar customisation", nullptr,
        ImGuiWindowFlags_NoResize);
        std::unordered_map<std::string, std::variant<int, sf::Color, std::string>> info;
        info["Info"] = "";
        ImGui::Text("Hair");
        if(ImGui::Button("<-##1", ImVec2(20,20))){
        info["Info"] = "Hair-down";
        };
        ImGui::SameLine();
        if(ImGui::Button("->##1", ImVec2(20,20))){
            info["Info"] = "Hair-up";
        }
        
        ImGui::Text("Outfit");
        if(ImGui::Button("<-##2", ImVec2(20,20))){
        info["Info"] = "Outfit-down";
        };
        ImGui::SameLine();
        if(ImGui::Button("->##2", ImVec2(20,20))){
            info["Info"] = "Outfit-up";
        }
        ImGui::Text("Accessories");
        if(ImGui::Button("<-##3", ImVec2(20,20))){
        info["Info"] = "Accessories-down";
        };
        ImGui::SameLine();
        if(ImGui::Button("->##3", ImVec2(20,20))){
            info["Info"] = "Accessories-up";
        }
        


        if(ImGui::Button("Skin", ImVec2(50,50))){
            sf::Color skin[5] = {
                sf::Color(255, 229, 204),  //light skin tone
                sf::Color(224, 187, 151),  //medium-light
                sf::Color(191, 143, 105),  //medium
                sf::Color(142,  93,  57),  //medium-dark
                sf::Color( 89,  52,  30)   //dark skin tone
            };
            current_skintone++;
            current_skintone = current_skintone%5;
            
            std::pair<int,sf::Color> skin_info = {current_skintone, skin[current_skintone]};
            info["Info"] = "Change skin";
            info["Color"] = skin_info.second;
            info["Color_int"] = skin_info.first;
        }

        ImGui::End();
        return info;
    }
   
    void run_equip_menu(){
        sf::Texture sprite_sheet;
        ImGui::Begin("Equipment");
        
        ImGui::BeginTable("Table", 8);
        ImGui::TableNextRow();
        int count = 0;
        for(auto& equip : equipment){
            ImGui::TableSetColumnIndex(count);
            ImGui::Text((equip.first + ": " + std::to_string(equip.second)).c_str());
            count++;
        }

        ImGui::TableNextRow();

        count = 0;
        for(auto& equip : equipment){
            ImGui::PushID(count);
            ImGui::TableSetColumnIndex(count);
            if(ImGui::Button("<", ImVec2(20,20))){
               equip.second -= 1;
               if(equip.second < 0){
                equip.second = 0;
               }
            }
            ImGui::SameLine();
            if(ImGui::Button(">", ImVec2(20,20))){
               equip.second += 1;
            }
            count++;
            ImGui::PopID();
        }

        ImGui::EndTable();
        
        ImGui::End();
    }

    std::pair<std::string,int> log_in(){
        ImGui::Begin("Log-in");
        if(logged_in_info.second == -1){
            ImGui::Text("Username");
            ImGui::InputText("##1", username, 24);
            ImGui::Text("Password");
            ImGui::InputText("##2", password, 128, ImGuiInputTextFlags_Password);
            ImGui::Text(log_in_msg.c_str());
            if(ImGui::Button("Sign in", {100,40})){
                log_in_msg = GH::SQL::log_in(username,password);
                if(log_in_msg[0] == 'H'){
                    pqxx::result login_info = GH::SQL::get_user_info(username);
                    logged_in_info.first = login_info[0]["username"].get<std::string>().value();
                    logged_in_info.second = login_info[0]["id"].get<int>().value();
                    user_level = login_info[0]["level"].get<float>().value();
                    user_level = 9;
                    load_maps();
                }
            }
            if(ImGui::Button("Register", {100,40})){
                log_in_msg = GH::SQL::reg(username,password);
            }
        }
        else{
            ImGui::Text(log_in_msg.c_str());
            if(ImGui::Button("Sign Out", {100,40})){
                logged_in_info = {"",-1};
            }
        }
        ImGui::End();
        return logged_in_info;
    }
    
    void spells(){//window for selecting spells
        ImGui::Begin("Spells");
        std::map<std::string,std::string> spell_combos = {{"Speed","111"},{"Basic protection","212"}, {"Dash","121"}, {"Chaotic tp","776"}, {"Spirit form","653"}, {"Protection","232"}, {"Enhanced protection","923"}, {"Controlled tp", "862"},{"Disorientation","141"}};
        ImGui::ProgressBar(user_level - (int)(user_level), {500,20});
        ImGui::Text(("Level: " + std::to_string((int)user_level)).c_str());
        std::string hieroglyphs[9] = {"1","2","3","4","5","6","7","8","9"};
        for(int i = 0 ; i < 9 ; i++){
            if(i%3 != 0){
                ImGui::SameLine();
            }
            hiero_sprite.setTextureRect(hiero_rects[i]);
            if(user_level >= i){
                if(ImGui::ImageButton(hieroglyphs[i].c_str(), hiero_sprite, {100,100})){
                    for(int j = 0 ; j < 3 ; j++){
                        if(slots[j] == " "){
                            slots[j] = hieroglyphs[i];
                            slot_sprites[j].setColor(sf::Color(255,255,255,255));
                            slot_sprites[j].setTextureRect(hiero_rects[i]);  
                            break;
                        }
                        else if(slots[j].size() <= 3){
                            slots[j] += hieroglyphs[i];
                            slot_sprites[j].setColor(sf::Color(255,255,255,255));
                            slot_sprites[j].setTextureRect(hiero_rects[i]);  
                            break;
                        }
                        
                    }
                }
            }
            else{
                ImGui::Button(("Locked: Level " + std::to_string(i)).c_str(), {108,108});
            }
        }
        for(int i = 0 ; i < 3 ; i++){
            if(slot_sprites[i].getColor().a != 0 && slots[i].size() < 3){
                slot_sprites[i].setColor(sf::Color(255,255,255,slot_sprites[i].getColor().a-5));
                if(slot_sprites[i].getColor().a < 0){
                    slot_sprites[i].setColor(sf::Color(255,255,255,0));
                }
            }
            else if(slot_sprites[i].getColor().a == 0 && slot_sprites[i].getTexture() == &spell_sheet){
                slot_sprites[i].setTexture(hiero_txt);
            }
        }
        for(int i = 0 ; i < 3 ; i++){
            std::string combo = "";
            if(slots[i].size() == 3){
                for(auto spell : spell_combos){
                    if(slots[i] == spell.second){
                        combo = spell.first;
                    }
                }
                if(slots[i].size() == 3 && combo == ""){
                    slots[i] = " ";
                }
                if(slots[(i+1)%3] != combo && slots[(i+2)%3] != combo){
                    slots[i] = combo;
                    slot_sprites[i].setTexture(spell_sheet);
                    slot_sprites[i].setTextureRect(spell_rects[combo]);
                    slot_sprites[i].setColor(sf::Color(255,255,255,255));
                }
                else{
                    slots[i] = " ";
                }
            }
            if(slot_sprites[i].getColor().a == 0){
                if(ImGui::Button((slots[i] + "##" + std::to_string(i)).c_str(), {108,108})){
                    slots[i] = " ";
                }
            }
            else if(slots[i].size() < 3){
                if(ImGui::ImageButton(("##" + std::to_string(i)).c_str(), slot_sprites[i], {100,100}, sf::Color::Transparent, sf::Color(255,255,255,slot_sprites[i].getColor().a - 255))){
                    slots[i] = " ";
                }
            }
            else{
                if(ImGui::ImageButton(("##" + std::to_string(i)).c_str(), slot_sprites[i], {100,100}, sf::Color::Transparent, sf::Color(255,255,255,255))){
                    slots[i] = " ";
                }
            }
        }
        

        
        
        ImGui::End();
    }
}
}