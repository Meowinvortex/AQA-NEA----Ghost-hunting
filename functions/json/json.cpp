#include <iostream>
#include <map>
#include <fstream>
#include <SFML/Graphics.hpp>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/ostreamwrapper.h"

#include "json.hpp"

namespace GH{
namespace JSON{
    std::string json_load(std::ifstream& infile){
    std::string content((std::istreambuf_iterator<char>(infile)),
    std::istreambuf_iterator<char>());
    return content;
}
}
}

