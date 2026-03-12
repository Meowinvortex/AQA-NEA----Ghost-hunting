#ifndef JSON_H
#define JSON_H

#include <iostream>
#include <map>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/ostreamwrapper.h"


namespace GH{
namespace JSON{
    std::string json_load(std::ifstream& infile);
}
}

#endif