
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

#include "player_textures.hpp"

/*This file handles loading and dressing the players avatar*/

namespace GH{
namespace TEXTURES{
namespace PLAYER{
    static std::map <std::string,int> main_indexes = { {"Skin",1}, {"Hair",1}, {"Outfit",1}, {"Accessories",1} };
    static std::map <std::string,sf::Texture> player_textures = { {"Skin",sf::Texture()}, {"Hair",sf::Texture()}, {"Outfit",sf::Texture()}, {"Accessories",sf::Texture()}};
    static std::map <std::string,sf::RectangleShape> player_shapes = { {"Skin",sf::RectangleShape({927, 656})}, {"Hair",sf::RectangleShape({896, 656})}, {"Outfit",sf::RectangleShape({896, 656})}, {"Accessories",sf::RectangleShape({896, 656})} };


    void save_avatar(){//save the currently made avatar locally so it doesnt need to be recreate on relaunch
        rapidjson::Document doc(rapidjson::kObjectType);
        rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();

        doc.AddMember("Skin", main_indexes["Skin"], allocator);
        doc.AddMember("Hair", main_indexes["Hair"], allocator);
        doc.AddMember("Outfit", main_indexes["Outfit"], allocator);
        doc.AddMember("Accessories", main_indexes["Accessories"], allocator);
    
        std::ofstream outfile(MISC_DIR "/avatar.json");
        rapidjson::OStreamWrapper osw(outfile);
        rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
        doc.Accept(writer);
        outfile.close();
    }
    void load_avatar(){//load the saved avatar
        std::ifstream infile(MISC_DIR"/avatar.json");
        std::string content((std::istreambuf_iterator<char>(infile)),
        std::istreambuf_iterator<char>());
        
        infile.close();
        rapidjson::Document doc;
        doc.Parse(content.c_str());
        main_indexes["Hair"] = doc["Hair"].GetInt();
        main_indexes["Outfit"] = doc["Outfit"].GetInt();
        main_indexes["Accessories"] = doc["Accessories"].GetInt();

    }

    void change_skin(){//change skin color
    main_indexes["Skin"] += 1;
    
    if(main_indexes["Skin"] > 9){
        main_indexes["Skin"] = 1;
    }
    }

    void compile_player_textures(sf::RenderTexture& compiled_texture){//compile all the parts of the avatar (body,hat,clothes,ect) into one texture
        std::string BODY_DIR = ASSETS_DIR "/textures/2_Characters/Character_Generator/Bodies/16x16";

        player_textures["Skin"].loadFromFile(BODY_DIR + "/Body_0" + std::to_string(main_indexes["Skin"]) + ".png");
        player_shapes["Skin"].setTexture(&player_textures["Skin"]);

        compiled_texture.clear(sf::Color(0, 0, 0, 0));

        compiled_texture.draw(player_shapes["Skin"]);
        compiled_texture.draw(player_shapes["Hair"]);
        compiled_texture.draw(player_shapes["Outfit"]);
        compiled_texture.draw(player_shapes["Accessories"]);
        compiled_texture.display();

        save_avatar();
    }



    void change_rect(std::map <std::string, sf::IntRect>& player_rect, std::string mode){//change the animation frame of the player depending on if they are still or moving
        if(mode == "idle1"){
            player_rect["Right"] = sf::IntRect(0,32,16,32);
            player_rect["Forward"] = sf::IntRect(96,32,16,32);
            player_rect["Left"] = sf::IntRect(192,32,16,32);
            player_rect["Back"] = sf::IntRect(288,32,16,32);
        }
        if(mode == "idle2"){
            player_rect["Right"] = sf::IntRect(16,32,16,32);
            player_rect["Forward"] = sf::IntRect(112,32,16,32);
            player_rect["Left"] = sf::IntRect(208,32,16,32);
            player_rect["Back"] = sf::IntRect(304,32,16,32);
        }
        if(mode == "idle3"){
            player_rect["Right"] = sf::IntRect(32,32,16,32);
            player_rect["Forward"] = sf::IntRect(128,32,16,32);
            player_rect["Left"] = sf::IntRect(224,32,16,32);
            player_rect["Back"] = sf::IntRect(320,32,16,32);
        }
        if(mode == "idle4"){
            player_rect["Right"] = sf::IntRect(48,32,16,32);
            player_rect["Forward"] = sf::IntRect(144,32,16,32);
            player_rect["Left"] = sf::IntRect(240,32,16,32);
            player_rect["Back"] = sf::IntRect(336,32,16,32);
        }

        if(mode == "moving1"){
            player_rect["Right"] = sf::IntRect(0,64,16,32);
            player_rect["Forward"] = sf::IntRect(96,64,16,32);
            player_rect["Left"] = sf::IntRect(192,64,16,32);
            player_rect["Back"] = sf::IntRect(288,64,16,32);
        }
        if(mode == "moving2"){
            player_rect["Right"] = sf::IntRect(16,64,16,32);
            player_rect["Forward"] = sf::IntRect(112,64,16,32);
            player_rect["Left"] = sf::IntRect(208,64,16,32);
            player_rect["Back"] = sf::IntRect(304,64,16,32);
        }
        if(mode == "moving3"){
            player_rect["Right"] = sf::IntRect(32,64,16,32);
            player_rect["Forward"] = sf::IntRect(128,64,16,32);
            player_rect["Left"] = sf::IntRect(224,64,16,32);
            player_rect["Back"] = sf::IntRect(320,64,16,32);
        }
        if(mode == "moving4"){
            player_rect["Right"] = sf::IntRect(48,64,16,32);
            player_rect["Forward"] = sf::IntRect(144,64,16,32);
            player_rect["Left"] = sf::IntRect(240,64,16,32);
            player_rect["Back"] = sf::IntRect(336,64,16,32);
        }


    }

    void cycle_hair(int num){//cycle through hair types
        namespace fs = std::filesystem;
        std::vector<fs::path> paths;

        if(player_shapes["Hair"].getFillColor() == sf::Color::Transparent){
            player_shapes["Hair"].setFillColor(sf::Color::White);
        }

        for (const auto& file : fs::recursive_directory_iterator(ASSETS_DIR"/textures/2_Characters/Character_Generator/Hairstyles/16x16")){
            paths.push_back(file);
        }
        switch (num){
        case 1:
            main_indexes["Hair"] += 1;
            if(main_indexes["Hair"] > paths.size() - 1){
                main_indexes["Hair"] = 1;
            }
            break;

        case -1:
            main_indexes["Hair"] -= 1;
            if(main_indexes["Hair"] == 0){
                main_indexes["Hair"] = paths.size() - 1;
            }
            break;
        
        default:
            break;
        }

        std::string path_str(paths[main_indexes["Hair"]].c_str());
        player_textures["Hair"].loadFromFile(path_str);
        player_shapes["Hair"].setTexture(&player_textures["Hair"]);
        player_shapes["Hair"].setFillColor(sf::Color::White);
    }

    void cycle_outfit(int num){//cycle through outfits
        namespace fs = std::filesystem;
        std::vector<fs::path> paths;

        if(player_shapes["Outfit"].getFillColor() == sf::Color::Transparent){
            player_shapes["Outfit"].setFillColor(sf::Color::White);
        }

        for (const auto& file : fs::recursive_directory_iterator(ASSETS_DIR"/textures/2_Characters/Character_Generator/Outfits/16x16")){
            paths.push_back(file);
        }
        switch (num){
        case 1:
            main_indexes["Outfit"] += 1;
            if(main_indexes["Outfit"] > paths.size() - 1){
                main_indexes["Outfit"] = 1;
            }
            break;
            
        case -1:
            main_indexes["Outfit"] -= 1;
            if(main_indexes["Outfit"] == 0){
                main_indexes["Outfit"] = paths.size() - 1;
            }
            break;

        default:
            break;
        }

        std::string path_str(paths[main_indexes["Outfit"]].c_str());
        player_textures["Outfit"].loadFromFile(path_str);
        player_shapes["Outfit"].setTexture(&player_textures["Outfit"]);
        player_shapes["Outfit"].setFillColor(sf::Color::White);
    }

    void cycle_accessories(int num){//cycle through accessories
        namespace fs = std::filesystem;
        std::vector<fs::path> paths;

        if(player_shapes["Accessories"].getFillColor() == sf::Color::Transparent){
            player_shapes["Accessories"].setFillColor(sf::Color::White);
        }

        for (const auto& file : fs::recursive_directory_iterator(ASSETS_DIR"/textures/2_Characters/Character_Generator/Accessories")){
            paths.push_back(file);
        }
        switch (num){
        case 1:
            main_indexes["Accessories"] += 1;
            if(main_indexes["Accessories"] > paths.size() - 1){
                main_indexes["Accessories"] = 1;
            }
            break;
            
        case -1:
            main_indexes["Accessories"] -= 1;
            if(main_indexes["Accessories"] == 0){
                main_indexes["Accessories"] = paths.size() - 1;
            }
            break;

        default:
            break;
        }

        std::string path_str(paths[main_indexes["Accessories"]].c_str());
        player_textures["Accessories"].loadFromFile(path_str);
        player_shapes["Accessories"].setTexture(&player_textures["Accessories"]);
        player_shapes["Accessories"].setFillColor(sf::Color::White);
    }


    void start_up(){//intial setup
        load_avatar();

        player_shapes["Outfit"].setFillColor(sf::Color::Transparent);
        player_shapes["Outfit"].setTexture(nullptr);

        player_shapes["Hair"].setFillColor(sf::Color::Transparent);
        player_shapes["Hair"].setTexture(nullptr);

        player_shapes["Accessories"].setFillColor(sf::Color::Transparent);
        player_shapes["Accessories"].setTexture(nullptr);
    }
}
}
}