#ifndef CREATE_MODE_H
#define CREATE_MODE_H

#include <SFML/Graphics.hpp>
#include <iostream>
#include "imgui/imgui.h"
#include "imgui-sfml-2.6.1/imgui-SFML.h"
#include <variant>
#include <map>
#include <vector>
#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "rapidjson/ostreamwrapper.h"
#include <fstream>
#include "../struct.hpp"

namespace GH{
namespace CREATE{
   extern light_system* chosen_system;
   extern candle::RadialLight* chosen_light;
   extern sf::RectangleShape* chosen_switch;
   extern std::string mode;
   void start_up();
  

   void load_theme(std::string theme);
  

   std::pair<std::string, GH::CREATE::obj> window_create();
  

   std::string window_mode();
  

   int window_walls();
  

   int window_floors();
  
  
   void window_select(obj* object);

   void lights(std::map<int,light_system>& light_systems);

   std::vector<bool> show_zones();

   std::string window_tools();
  
}
}

#endif