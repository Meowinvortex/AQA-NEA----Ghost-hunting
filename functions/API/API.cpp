
#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <ctime>

#include "rapidjson/document.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/prettywriter.h"

namespace GH {
namespace API {

    void get_moon_phase(){//ignore name, used to get the current weather used for immersive weather in game
        try{
            curlpp::Easy request;
            std::stringstream response;
            
            request.setOpt(curlpp::options::Url("http://api.weatherapi.com/v1/current.json?key=KEY&q=London&aqi=no"));
            request.setOpt(curlpp::options::WriteStream(&response));

            request.perform();
            

            rapidjson::Document doc;
            doc.Parse(response.str().c_str());
        

            std::ofstream out(MISC_DIR "/moon_phases.json");

            rapidjson::OStreamWrapper osw(out);
            rapidjson::PrettyWriter<rapidjson::OStreamWrapper> writer(osw);
            doc.Accept(writer);
        }
        catch(...){
            std::cout<<"No internet"<<std::endl;
        }
    }

}
} 
