#pragma once
#include <SFML/Graphics.hpp>
#include <Candle/Candle.hpp>
#include <iostream>
#include "imgui/imgui.h"
#include "imgui-sfml-2.6.1/imgui-SFML.h"
#include <tuple>
#include <map>
#include <regex>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/ostreamwrapper.h"
#include <fstream>
#include <filesystem>
#include <pqxx/pqxx>
#include "../SQL/SQL.hpp"


#include "create_mode.hpp"

namespace GH{
namespace CREATE{
  
  //These variables mostly determine the state of each mode in the map creator
  namespace fs = std::filesystem;
  static std::vector<std::string> paths;
  static std::vector<sf::Texture> textures;
  std::string mode = "Create";
  static std::vector<sf::IntRect> wall_rect;
  light_system* chosen_system = nullptr;
  candle::RadialLight* chosen_light = nullptr;
  sf::RectangleShape* chosen_switch = nullptr;

  //All variables that store data from the customise options for lights
  int range_int;
  float intensity_float;
  int light_temp;
  int theme_index;

  bool show_rooms;
  bool show_hiding;
  bool show_ambient;

  
  std::pair<std::string,obj> info_create;// Used for creating objects, the objects attributes will be stored in here and passed to the main file
  int info_wall = 0;// Used for creating walls, gets passed to the main file and used for the index of the wall texture
  int info_floors = 0;// Used for creating floor, gets passed to the main file and used for the index of the floor texture


  void start_up(){
    info_create.first = "";
  }

  //Runs when the players chooses a new theme of objects, replaces any old textures with new ones from the chosen theme's folder
  void load_theme(std::string theme){
    textures = {};
    pqxx::result theme_data = GH::SQL::load_all_theme(theme);
    sf::Texture temp;
    
    for(auto img : theme_data){
      pqxx::binarystring blob(img["img"]);
      temp.loadFromMemory(blob.data(),blob.size());
      textures.push_back(temp);
    }
  }
  

  //This window shows when create mode has been selected, its used for creating objects
  std::pair<std::string, GH::CREATE::obj> window_create(){
      ImGui::Begin("Create");
      
      static const char* themes[]{"Living","Bathroom", "Kitchen", "Bedroom", "Basement", "Japanese", "Hospital", "Prison", "Museum", "Music & sport", "Classroom & library", "Art", "Gym", "Fishing", "Birthday", "Halloween", "Christmas", "Grocery store", "Clothing store", "Flim studio", "Ice cream", "Shooting range"};
      static int selected_theme = 0;
      if (ImGui::Combo("##", &selected_theme, themes, IM_ARRAYSIZE(themes))){  //Combo box for selecting object theme
          load_theme(themes[selected_theme]);
      }

      static bool has_collision = true; //On first initialisation always make objects have collision

      ImGui::Checkbox("Has collision", &has_collision);  //Check box for toggling an object's collision

      if(has_collision){
        info_create.second.has_collision = true;
      }
      else{
        info_create.second.has_collision = false;
      }

      static bool blocks_light = false;

      ImGui::Checkbox("Blocks light", &blocks_light);  //Check box for toggling an object's light blocking

      if(blocks_light){
        info_create.second.blocks_light = true;
      }
      else{
        info_create.second.blocks_light = false;
      }

      static bool throwable = false;

      ImGui::Checkbox("Throwable", &throwable);  //Check box for toggling an object's light blocking

      if(throwable){
        info_create.second.throwable = true;
      }
      else{
        info_create.second.throwable = false;
      }


      for(int i = 0 ; i < textures.size() ; i++){  //For loop iterates for every texture in the currently selected them and creates a button for that texture
        ImGui::PushID(static_cast<int>(i));
        if(ImGui::ImageButton("Bootoon",(ImTextureID)(uintptr_t)textures[i].getNativeHandle(), ImVec2(50,50))){  //Upon a press of that button the texture and size of the object is passed to the main file
             info_create.second.texture = textures[i];
             info_create.second.theme = selected_theme + 1;
             info_create.second.index = i + 1;
             info_create.second.size.first = textures[i].getSize().x;
             info_create.second.size.second = textures[i].getSize().y;
             info_create.first = "active";
        }
       
        if((i+1)%4 != 0){
          ImGui::SameLine();
        }
        ImGui::PopID();
      }

      ImGui::End();

      return info_create;
  }
  
  //Window shows when wall mode is selected
  int window_walls(){
    ImGui::Begin("Walls");
    
    //Button for decreasing the wall texture index
    if(ImGui::Button("<--", ImVec2(20,20))){
      info_wall -= 1;
      if(info_wall <= -1){
        info_wall = 47;
      }
    }
    ImGui::SameLine();

    //Button for increasing the wall texture index
    if(ImGui::Button("-->", ImVec2(20,20))){
      info_wall += 1;
      if(info_wall >= 48){
        info_wall = 0;
      }
    }

    ImGui::Text("V - toggle vertical walls");  //Letting the player know they can press the V key to toggle vertical walls
    
    ImGui::End();
    
    return info_wall;
  }
  
  //Window is shown when floors mode is selected
  int window_floors(){
    ImGui::Begin("Floors");
    
    //Button for decreasing the floor index
    if(ImGui::Button("<--", ImVec2(20,20))){
      info_floors -= 1;
      if(info_floors <= -1){
        info_floors = 71;
      }
    }
    ImGui::SameLine();

    //Button for increasing the floor index
    if(ImGui::Button("-->", ImVec2(20,20))){
      info_floors += 1;
      if(info_floors >= 72){
        info_floors = 0;
      }
    }

    
    ImGui::End();
    
    return info_floors;
  }
  
  //Window for selecting the mode in the map creator, therefore unlike every window this window will always show until create mode is disabled
  std::string window_mode(){
     ImGui::Begin("Mode");
     if(mode == "Save"){
      mode == "Create";
     }
     if(ImGui::Button("Create", ImVec2(100,50))){
       mode = "Create";
     }
     if(ImGui::Button("Lights", ImVec2(100,50))){
       mode = "Lights";
     }
     if(ImGui::Button("Delete", ImVec2(100,50))){
       mode = "Delete";
     }
    if(ImGui::Button("Walls", ImVec2(100,50))){
       mode = "Walls";
     }
    if(ImGui::Button("Floors", ImVec2(100,50))){
       mode = "Floors";
     }
    if(ImGui::Button("Rooms", ImVec2(100,50))){
       mode = "Rooms";
     }
    if(ImGui::Button("Hiding", ImVec2(100,50))){
       mode = "Hiding";
     }
    if(ImGui::Button("Ambient", ImVec2(100,50))){
       mode = "Ambient";
     }
    if(ImGui::Button("Select", ImVec2(100,50))){
       mode = "Select";
     }
    if(ImGui::Button("Save", ImVec2(100,50))){
       mode = "Save";
     }
     ImGui::End();

     return mode;
  }
  
  //Window shows when select mode is selected
  void window_select(obj* object){
    ImGui::Begin("Selection");
    
    //Button for moving the selected object upwards
    if(ImGui::Button("^")){
       object->shape.move({0,-10});
    }
    
    //Button for moving the selected object left
    if(ImGui::Button("<")){
       object->shape.move({-10, 0});
    }

    ImGui::SameLine();
    
    //Button for moving the selected object right
    if(ImGui::Button(">")){
       object->shape.move({10,0});
    }
    
    //Button for moving the selected object downwards
    if(ImGui::Button("v")){
       object->shape.move({0, 10});
    }

    ImGui::Text("V - anchor/unanchor to mouse");  //Letting the player know the can toggle the selected object moving with the mouse by pressing the V key

    ImGui::End();
  }
  
  void lights(std::map<int,light_system>& light_systems){//interface for creating light systems on the map
      ImGui::Begin("Lights");
      if(ImGui::Button("Create new light system")){
        light_system temp;
        temp.lights_on = true;
        light_systems[light_systems.size()] = std::move(temp);
      }
      for(int i = 0 ; i < light_systems.size() ; i++){
        if(ImGui::Button(("Light system " + std::to_string(i)).c_str())){
          chosen_system = &light_systems[i];
          chosen_light = nullptr;
          light_temp = 0;
        }
      }
      ImGui::End();
      if(chosen_system != nullptr){
        ImGui::Begin("Customise lights");
        if(ImGui::Button("Create new light")){
          chosen_system->lights.push_back(candle::RadialLight{});
          chosen_light = &chosen_system->lights[chosen_system->lights.size()-1];
        }
        ImGui::Text("Lights in system");
        for(int i = 0 ; i < chosen_system->lights.size() ; i++){
          if(ImGui::Button(("Light " + std::to_string(i)).c_str())){
            chosen_switch = nullptr;
            chosen_light = &chosen_system->lights[i];
            chosen_light->setRange(20);
            range_int = 0;
            intensity_float = 0;
          }
        }
        if(ImGui::Button("Create new switch")){
          chosen_system->create_light_switch();
        }
        ImGui::Text("Switches in system");
        for(int i = 0 ; i < chosen_system->light_switches.size() ; i++){
          if(ImGui::Button(("Switch " + std::to_string(i)).c_str())){
            chosen_light = nullptr;
            chosen_switch = &chosen_system->light_switches[i];
          }
        }
        if(chosen_light != nullptr){
          ImGui::Text("Press V to toggle mouse anchoring");
          ImGui::SliderInt("Range", &range_int, 0, 1000);
          if(chosen_light->getRange() != range_int){
            chosen_light->setRange(range_int);
          }
          ImGui::SliderFloat("Intensity", &intensity_float, 0, 1);
          if(chosen_light->getIntensity() != intensity_float){
            chosen_light->setIntensity(intensity_float);
          }
          ImGui::SliderInt("Temp", &light_temp, 0, 5);
          if(chosen_system->light_temp != light_temp){
             chosen_system->light_temp = light_temp;
             chosen_system->update_temp();
          }
        }
        ImGui::End();
      }

  }
  
  std::vector<bool> show_zones(){//toggle which zones can be visible on the screen for testing purposes
    ImGui::Begin("Show zones");
    ImGui::Checkbox("Rooms", &show_rooms);
    ImGui::Checkbox("Hiding", &show_hiding);
    ImGui::Checkbox("Ambient", &show_ambient);
    ImGui::End();
    return {show_rooms, show_hiding, show_ambient};
  }

  std::string window_tools(){
    ImGui::Begin("Tools");
    static std::string tool = "Null";
    sf::Texture chest_txt;
    chest_txt.loadFromFile(ASSETS_DIR"/textures/3_Animated_objects/16x16/spritesheets/animated_toybox_empty_.png");
    if(ImGui::ImageButton("Bootoon",(ImTextureID)(uintptr_t)chest_txt.getNativeHandle(), ImVec2(50,50))){
      tool = "Chest";
    }
    ImGui::End();
    return tool;
  }
}
}

