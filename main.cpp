//library and package includes
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <vector>
#include <ctime>
#include <cstdlib>
#include <iostream>
#include <Candle/Candle.hpp>
#include <fstream>
#include <variant>
#include <unordered_map>
#include <map>
#include <thread>
#include <random>
#include "libs/rapidjson/document.h"
#include "libs/rapidjson/stringbuffer.h"
#include "libs/rapidjson/writer.h"
#include "libs/imgui/imgui.h"
#include "libs/imgui-sfml-2.6.1/imgui-SFML.h"
#include <TGUI/TGUI.hpp>
#include <TGUI/Backend/SFML-Graphics.hpp>
#include <cerrno>
#include <cstring>

//include external files
#include "functions/maths.hpp"
#include "functions/json/json.hpp"
#include "functions/start_menu/start_menu.hpp"
#include "functions/Audio/Audio_recording.hpp"
#include "functions/create_mode/create_mode.hpp"
#include "functions/Textures/player/player_textures.hpp"
#include "functions/Textures/Walls/walls_textures.hpp"
#include "functions/Sounds/ghost-sounds.hpp"
#include "functions/Inventory/inventory.hpp"
#include "functions/Journal/journal.hpp"
#include "functions/Ghosts/behavior.hpp"
#include "functions/struct.hpp"
#include "functions/SQL/SQL.hpp"
#include "functions/API/API.hpp"
#include "functions/Spells/spells.hpp"

using namespace std;
using namespace rapidjson;

//numbers involving the size of the map and window
const int W_WIDTH = sf::VideoMode::getDesktopMode().width;
const int W_HEIGHT = sf::VideoMode::getDesktopMode().height;
int M_WIDTH = 800;
int M_HEIGHT = 600;

//miscellaneous bool variables required globally
map<sf::Keyboard::Key,bool> held = {{sf::Keyboard::F10,false}, {sf::Keyboard::V,false}, {sf::Keyboard::T, false}, {sf::Keyboard::Q, false}, {sf::Keyboard::C, false}};
bool display_game = false;
bool ispythonrunning = false;
bool immovable = false;
bool weight = false;

string mode = "create";
std::pair<std::string,int> user_info;
float user_level = 0;

std::random_device rd;                      
std::mt19937 gen(rd());

sf::Time delta_time;

//Variables containing the sfml window, the gui overlay and the view which is used to display parts of the maps
sf::RenderWindow window(sf::VideoMode(W_WIDTH, W_HEIGHT), "Ghost huntin'");
sf::View view(sf::FloatRect(0,0, W_WIDTH/2, W_HEIGHT/2));
tgui::Gui gui{window};

class audio_class{ //This class stores the buffers for audio used in the main file and the sounds variables itself that load the buffers
    public:
    std::map<std::string, sf::SoundBuffer> buffer;
    std::map<std::string, sf::Sound> sounds;

    audio_class(){
        buffer["Bell"].loadFromFile(ASSETS_DIR"/sounds/Bell.wav");
        sounds["Bell"].setBuffer(buffer["Bell"]);

        buffer["Whisper"].loadFromFile(ASSETS_DIR"/sounds/whisper_1.mp3");
        sounds["Whisper"].setBuffer(buffer["Whisper"]);
        sounds["Whisper"].setLoop(true);
        sounds["Whisper"].setVolume(50);
        sounds["Whisper"].setRelativeToListener(false);
        sounds["Whisper"].setMinDistance(100.f);
        sounds["Whisper"].setAttenuation(1.f); 

        buffer["Enter_spectral"].loadFromFile(ASSETS_DIR"/sounds/Magic and Spells 5 - Universal Sound Effects/MP3/Magic Spell 7.mp3");
        sounds["Enter_spectral"].setBuffer(buffer["Enter_spectral"]);

        buffer["emf"].loadFromFile(ASSETS_DIR"/sounds/emf.mp3");
        sounds["emf"].setBuffer(buffer["emf"]);
        sounds["emf"].setLoop(true);
        sounds["emf"].setVolume(50);
        sounds["emf"].setRelativeToListener(false);
        sounds["emf"].setMinDistance(100.f);
        sounds["emf"].setAttenuation(1.f);

        buffer["rain"].loadFromFile(ASSETS_DIR"/sounds/rain.mp3");
        sounds["rain"].setBuffer(buffer["rain"]);
        sounds["rain"].setLoop(true);

        buffer["glass_good"].loadFromFile(ASSETS_DIR"/sounds/breaking_good.mp3");
        sounds["glass_good"].setBuffer(buffer["glass_good"]);

        buffer["glass_bad"].loadFromFile(ASSETS_DIR"/sounds/breaking_bad.mp3");
        sounds["glass_bad"].setBuffer(buffer["glass_bad"]);
    }
};

audio_class audio;

sf::RectangleShape mouse_hover(sf::Vector2f(50,50));
sf::Texture mouse_hover_txt;

//this class contains any physical/ graphical data of the player such as cosmetics, hitboxes ect
class player_sprites_class {
    private:
    bool istransparent;
    public:
    sf::RectangleShape body; //The body itself of the player
    sf::Texture cold_txt;
    sf::RectangleShape cold; //used for cold evidence
    std::vector<sf::RectangleShape> camera_outline; //shows when the camera is used
    sf::Texture camera_txt;
    std::vector<sf::RectangleShape> hitboxes; //the hitboxes for the body to prevent collision
    std::map<int,std::string> spells; //loaded spells for the loaded game
    std::map<std::string,bool> pics_taken; //used to see what pictures have been used in the current game

    float speed = 200.f; //how fast the player is
    float sanity = 100; //How sane the player is
    float stamina = 100.f; //How much sprint stamina the player has
    bool stam_regen = false; //Used to check when to regen stamina
    bool dead = false; //True when the player is dead
    bool hiding = false; //True when the player is hiding
    sf::Vector2f previous_pos; 
    player_sprites_class() : istransparent(true){ //constructor

        pics_taken = {{"Ethereal residue",false},{"Ghost",false},{"Object thrown",false}};
        
        //basic first manipulation of player body
        body.setSize({80,110});
        body.setOrigin(body.getSize().x / 2, body.getSize().y / 2);
        body.setPosition(100,500);
        cold.setSize({64,64});
        cold.setOrigin({32,32});
        cold.setFillColor(sf::Color(255,255,255,0));
        cold_txt.loadFromFile(ASSETS_DIR"/textures/mist.png");
        cold.setTexture(&cold_txt);
        
        //Setup the overlay for when the camera is equipped
        camera_txt.loadFromFile(ASSETS_DIR"/textures/camera_outline.png");
        camera_outline = {sf::RectangleShape(), sf::RectangleShape(), sf::RectangleShape(), sf::RectangleShape()};
        sf::RectangleShape corner;
        corner.setTexture(&camera_txt);
        corner.setSize({W_HEIGHT/10, W_HEIGHT/10});
        corner.setPosition({0,0});
        corner.setFillColor(sf::Color(0,0,0,0));
        camera_outline[0] = corner;
        camera_outline[1] = corner;
        camera_outline[2] = corner;
        camera_outline[3] = corner;

        camera_outline[1].setRotation(90);
        camera_outline[2].setRotation(180);
        camera_outline[3].setRotation(270);
        
        //initialise hitboxes
        for(int i = 0 ; i < 4 ; i ++){
            hitboxes.push_back(sf::RectangleShape{});
        }
        hitboxes[0].setSize({body.getSize().x*0.6,10});
        hitboxes[1].setSize({body.getSize().x*0.6,10});
        hitboxes[2].setSize({10,body.getSize().y*0.6});
        hitboxes[3].setSize({10,body.getSize().y*0.6});
        for(auto& hitbox : hitboxes){
            hitbox.setOrigin({hitbox.getSize().x/2, hitbox.getSize().y/2});
        }
       



    }
    void draw(sf::RenderWindow& window, std::string facing){  //Draw all sprites in the player class
        if(GH::SPELLS::is_dashing){ //used when the player is dashing to give the illusion of after images behind the player
            sf::Vector2f current = body.getPosition();
            sf::Vector2f velocity = {0,0};
            if(facing == "Forward"){
                velocity = {0,50};
            }
            else if(facing == "Back"){
                velocity = {0,-50};
            }
            else if(facing == "Left"){
                velocity = {50,0};
            }
            else if(facing == "Right"){
                velocity = {-50,0};
            }

            for(int i = 0 ; i < 5 ; i++){
                body.setFillColor(sf::Color(255,0,255,body.getFillColor().a-25));
                body.move(velocity);
                window.draw(body);
            }
            body.setFillColor(sf::Color(255,0,255,150));
            body.setPosition(current);
        }
        window.draw(body);
    }

    void align(){  //Align any sprites in this class with the main body
        hitboxes[0].setPosition({body.getPosition().x,(body.getPosition().y - (body.getSize().y/2)+25)});
        hitboxes[1].setPosition({body.getPosition().x, (body.getGlobalBounds().top + body.getGlobalBounds().height)});
        hitboxes[2].setPosition({body.getGlobalBounds().left,body.getPosition().y+15});
        hitboxes[3].setPosition({(body.getGlobalBounds().left + body.getGlobalBounds().width), body.getPosition().y+15});
        sf::Vector2f view_pos = view.getCenter();
        camera_outline[0].setPosition({view_pos.x - W_WIDTH/4, view_pos.y - W_HEIGHT/4});
        camera_outline[1].setPosition({view_pos.x + W_WIDTH/4, view_pos.y - W_HEIGHT/4});
        camera_outline[2].setPosition({view_pos.x + W_WIDTH/4, view_pos.y + W_HEIGHT/4});
        camera_outline[3].setPosition({view_pos.x - W_WIDTH/4, view_pos.y + W_HEIGHT/4});
    }
    
};
//initialising above class
player_sprites_class player_sprites;

class ghost_class{
    public:
    sf::RectangleShape sprite; //sprite for the ghost that is seen by the player
    sf::Texture sprite_txt; //texture for the ghost
    sf::RectangleShape UV; //UV spot for when the player interacts with something
    sf::Texture UV_txt;
    sf::RectangleShape gate; //The gateway which is randomly placed in the ghost room
    sf::RectangleShape ethereal_residue; //Used for the player to show this evidence whena  picture is taken
    sf::Texture gate_txt; //texture for the gateway
    sf::RectangleShape* gateway_obj; //what object in the ghost room is attatched to the gateway
    sf::CircleShape interaction_radius; //the radius to which the ghost can do stuff in, essentially giving its range
    std::string ghost_type; //the ghost type of the ghost
    std::string evidence[3]; //what evidence the chosen ghost type has
    sf::VertexArray vision_cone; //The vision area of the ghost for spotting the player in hunts
    std::string facing; //which way the ghost is facing
    std::string previous_event; //what did the ghost previously do
    std::vector<sf::CircleShape> walk_path; //the order of path the ghost will wander to

    bool can_UV; //has the ability to show UV
    bool seen_player; //seen the player
    bool override_kill; //if true ghost cant kill the player

    int idle_value;
    int emf; //emf level of the ghost
    int room_index;

    float speed;

    sf::RectangleShape* chosen_room;
    ghost_class() : vision_cone(sf::VertexArray(sf::Triangles, 3)){ //constructor
        //set up any miscellaneous variables and the ghost sprite
        override_kill = false;
        seen_player = false;
        speed = 200;
        can_UV = true;
        sprite.setSize({80,110});
        sprite.setOrigin(sprite.getSize().x / 2, sprite.getSize().y / 2);
        sprite.setPosition(0,0);
        sprite.setFillColor(sf::Color(85,255,230,0));
        std::uniform_int_distribution<> ghost_txt_index(1,2);
        sprite_txt.loadFromFile(ASSETS_DIR"/textures/2_Characters/Ghosts/Ghost_" + to_string(ghost_txt_index(gen)) + ".png");
        sprite.setTexture(&sprite_txt);
        sprite.setTextureRect(sf::IntRect(96,32,16,32));

        facing = "S";
        emf = 0;
        previous_event = "NILL";

        //correctly size and position the ghosts vision cone
        vision_cone[0].position = sprite.getPosition();
        vision_cone[1].position = {vision_cone[0].position.x + 100, vision_cone[0].position.y + 200};
        vision_cone[2].position = {vision_cone[0].position.x - 100, vision_cone[0].position.y + 200};
        vision_cone[0].color = sf::Color(255,0,0);
        vision_cone[1].color = sf::Color(255,0,0);
        vision_cone[2].color = sf::Color(255,0,0);
        
        //correctly size the ghosts interaction radius
        interaction_radius.setRadius(350);
        interaction_radius.setOrigin({interaction_radius.getRadius(), interaction_radius.getRadius()});
        interaction_radius.setFillColor(sf::Color(0,0,255,20));


        //set up the size, position and colors of miscellaneous shapes
        UV.setSize({64,64});
        UV.setPosition({99999,99999});
        UV.setFillColor(sf::Color(255,0,255));
        UV_txt.loadFromFile(ASSETS_DIR"/textures/mist.png");
        UV.setTexture(&UV_txt);

        gate_txt.loadFromFile(ASSETS_DIR"/textures/Gate.png");
        gate.setTexture(&gate_txt);
        gate.setFillColor(sf::Color(255,255,255,0));

        ethereal_residue.setSize({128,256});
        ethereal_residue.setFillColor(sf::Color(255,0,255,0));

        sf::CircleShape temp_circle;
        temp_circle.setFillColor(sf::Color(96,96,96));
        temp_circle.setRadius(10);
        for(int i = 0 ; i < 5 ; i++){
            walk_path.push_back(temp_circle);
        }
    }

    void align(){ //align interaction radius with the ghosts position
        interaction_radius.setPosition(sprite.getPosition());
    }
    void upd_facing(){ //update the vision cones size and position based on which way the ghost is facing
        vision_cone[0].position = sprite.getPosition();
        if(facing == "S"){
            vision_cone[1].position = {vision_cone[0].position.x + 400, vision_cone[0].position.y + 800};
            vision_cone[2].position = {vision_cone[0].position.x - 400, vision_cone[0].position.y + 800};
            sprite.setTextureRect(sf::IntRect(96,32,16,32));
        }
        else if(facing == "N"){
            vision_cone[1].position = {vision_cone[0].position.x + 400, vision_cone[0].position.y - 800};
            vision_cone[2].position = {vision_cone[0].position.x - 400, vision_cone[0].position.y - 800};
            sprite.setTextureRect(sf::IntRect(288,32,16,32));
        }
        else if(facing == "E"){
            vision_cone[1].position = {vision_cone[0].position.x + 800, vision_cone[0].position.y + 400};
            vision_cone[2].position = {vision_cone[0].position.x + 800, vision_cone[0].position.y - 400};
            sprite.setTextureRect(sf::IntRect(0,32,16,32));
        }
        else if(facing == "W"){
            vision_cone[1].position = {vision_cone[0].position.x - 800, vision_cone[0].position.y + 400};
            vision_cone[2].position = {vision_cone[0].position.x - 800, vision_cone[0].position.y - 400};
            sprite.setTextureRect(sf::IntRect(192,32,16,32));
        }
        else if(facing == "EN"){
            vision_cone[1].position = {vision_cone[0].position.x + 800, vision_cone[0].position.y - 200};
            vision_cone[2].position = {vision_cone[0].position.x + 200, vision_cone[0].position.y - 800};
            sprite.setTextureRect(sf::IntRect(0,32,16,32));
        }
        else if(facing == "ES"){
            vision_cone[1].position = {vision_cone[0].position.x + 800, vision_cone[0].position.y + 200};
            vision_cone[2].position = {vision_cone[0].position.x + 200, vision_cone[0].position.y + 800};
            sprite.setTextureRect(sf::IntRect(0,32,16,32));
        }
        else if(facing == "WN"){
            vision_cone[1].position = {vision_cone[0].position.x - 800, vision_cone[0].position.y - 200};
            vision_cone[2].position = {vision_cone[0].position.x - 200, vision_cone[0].position.y - 800};
            sprite.setTextureRect(sf::IntRect(192,32,16,32));
        }
        else if(facing == "WS"){
            vision_cone[1].position = {vision_cone[0].position.x - 800, vision_cone[0].position.y + 200};
            vision_cone[2].position = {vision_cone[0].position.x - 200, vision_cone[0].position.y + 800};
            sprite.setTextureRect(sf::IntRect(192,32,16,32));
        }

    }

    void play_emf(){ //play the emf sound if there is an emf gauge in the radius and an emf is being emitted
        for(auto& equipment : GH::INV::loaded_items){
            if(emf == 1){
                audio.sounds["emf"].setPitch(0.3);
            }
            else if(emf == 2){
                audio.sounds["emf"].setPitch(0.5);
            }
            else if(emf == 3){
                audio.sounds["emf"].setPitch(0.8);
            }
            else if(emf == 4){
                audio.sounds["emf"].setPitch(1);
            }
            else if(emf == 5){
                audio.sounds["emf"].setPitch(1.2);
            }
            if(equipment.name == "EMF" && !equipment.in_inventory && equipment.shape.getGlobalBounds().intersects(interaction_radius.getGlobalBounds())){
                std::cout<<emf<<std::endl;
                audio.sounds["emf"].setPosition(sprite.getPosition().x, sprite.getPosition().y, 0);
                if(emf == 0 && audio.sounds["emf"].getStatus() == sf::Sound::Playing){
                    audio.sounds["emf"].stop();
                }
                else if(audio.sounds["emf"].getStatus() != sf::Sound::Playing && emf != 0){
                    audio.sounds["emf"].play();
                }
            }
            else{
                audio.sounds["emf"].stop();
            }

        }
    }
};

ghost_class ghost;

  std::string path = ASSETS_DIR"/textures/1_Interiors/16x16/Room_Builder_16x16.png";
        sf::Image perm_image;
        sf::Image temp_image;
        sf::Texture temp_texture;
        sf::Texture perm_texture;
//this class wil hold the majority of textures such as the player's textures
class textures_class{
    public:
    sf::RenderTexture compiled_texture;
    sf::Texture player_texture;
    sf::Texture temp;
    map <string, sf::IntRect> player_rect;
    string player_facing = "Back";
    string player_new_facing = "Back";
    string mode = "idle";

    std::map<int,sf::Texture> wall_textures;
    std::map<int,sf::Texture> floor_textures;

    sf::RectangleShape FIREBAWL;
    sf::Texture FIREBAWL_txt;

    int anim_index = 1;

    textures_class(){
     compiled_texture.create(927,656);
     sf::Image img;
     img.loadFromFile(MISC_DIR"/avatar.png");
     temp.loadFromImage(img);
     player_sprites.body.setTexture(&temp);
     GH::TEXTURES::PLAYER::change_rect(player_rect, "idle1");
     player_sprites.body.setTextureRect(player_rect[player_facing]);

     wall_textures = GH::TEXTURES::WALLS::set_up_walls();
     floor_textures = GH::TEXTURES::WALLS::set_up_floors();

     FIREBAWL_txt.loadFromFile(ASSETS_DIR"/textures/FIREBAWL.png");
     FIREBAWL.setTexture(&FIREBAWL_txt);
     FIREBAWL.setSize({64,64});
     FIREBAWL.setOrigin({32,32});
     FIREBAWL.setFillColor(sf::Color(255,255,255,0));
     FIREBAWL.setTextureRect(sf::IntRect(0,0,16,16));
    }

    void change_anim_index(){
        anim_index += 1;
        if(anim_index > 4){
            anim_index = 1;
        }
        FIREBAWL.setTextureRect(sf::IntRect(16*(anim_index-1), 0, 16, 16));
    }

    void update_texture_player(){
        GH::TEXTURES::PLAYER::compile_player_textures(compiled_texture);
        compiled_texture.getTexture().copyToImage().saveToFile(MISC_DIR"/avatar.png");
        player_texture = compiled_texture.getTexture();
        player_sprites.body.setTexture(&player_texture);
        
    }

    void update_player_rect(string mode){
        GH::TEXTURES::PLAYER::change_rect(player_rect, mode);
        player_sprites.body.setTextureRect(player_rect[player_facing]);
    }
    
};

//initialise the above classes
textures_class textures;

class map_collisions_class{
   private:
   public:
    int map_chosen;
    sf::RectangleShape chest;
    sf::RectangleShape rain;
    sf::VertexArray grass;
    std::vector<sf::Texture> rain_txt;
    sf::Texture chest_txt;
    map<int,GH::CREATE::obj> objects;
    map<int,GH::CREATE::obj> walls;
    map<int,GH::CREATE::obj> floors;
    map<int,GH::CREATE::obj> room_zones;
    map<int,GH::CREATE::obj> hiding_zones;
    map<int,GH::CREATE::obj> ambient_zones;
    map<int, GH::CREATE::light_system> light_systems;
    GH::CREATE::obj* object_ptr;
    std::vector<bool> show_zones;
    int id_objects;
    int id_walls;
    int id_floors;
    int id_rooms;
    int id_hiding;
    int id_ambient;
    int priority_object;

    std::vector<std::string> items_in_chest;
    int chest_index;
    int rain_index;
    int rain_intensity;

    bool chest_empty = false;
    bool next_item = true;
    bool animate_chest = false;
    bool unusable_map;
    bool raining = false;

   map_collisions_class() : grass(sf::Quads, 4){
    
    for(int i = 0 ; i < 4 ; i++){
        grass[i].color = sf::Color(90,95,61);
    }
    
    rain.setTextureRect(sf::IntRect(0,0,W_HEIGHT/2,W_WIDTH/2));
    rain_index = 0;
    for(int i = 1 ; i <= 4 ; i++){
        sf::Texture temp;
        temp.loadFromFile(ASSETS_DIR"/textures/rain/rain_drops-0" + std::to_string(i) + ".png");
        rain_txt.push_back(temp);
        rain_txt[i-1].setRepeated(true);
    }
    rain.setTexture(&rain_txt[0]);
    if(true){
        rain_intensity = 20;
        rain.setFillColor(sf::Color(255,255,255,70));
    }
    chest_txt.loadFromFile(ASSETS_DIR"/textures/3_Animated_objects/16x16/spritesheets/chest.png");
    chest.setTexture(&chest_txt);
    chest.setSize({14*5,24*5});
    chest.setTextureRect(sf::IntRect(1,0,14,24));
    chest.setOrigin({chest.getSize().x/2, chest.getSize().y/2});
    chest.setPosition({9999,9999});
    id_objects = 0;
    id_walls = 0;
    id_floors = 0;
    id_rooms = 0;
    id_hiding = 0;
    priority_object = -1;
    show_zones = {false,false,false};
    unusable_map = false;


   }

   void draw(sf::RenderWindow& window){
     for(auto object : floors){
        if(GH::MATH::rect_in_view(object.second.shape.getGlobalBounds(), view)){
            window.draw(object.second.shape);
        }
     }
     for(auto wall : walls){
        if(GH::MATH::rect_in_view(wall.second.shape.getGlobalBounds(), view)){
            window.draw(wall.second.shape);
        }
     }
     for(auto object : objects){
        if(object.first != priority_object){
            if(GH::MATH::rect_in_view(object.second.shape.getGlobalBounds(), view)){
                window.draw(object.second.shape);
            }
        }
     }
     if(priority_object != -1){
        window.draw(objects[priority_object].shape);
     }
     if(show_zones[0]){
        for(auto& zone : room_zones){
            window.draw(zone.second.shape);
        }
     }
     if(show_zones[1]){
        for(auto& zone : hiding_zones){
            window.draw(zone.second.shape);
        }
     }
     if(show_zones[2]){
        for(auto& zone : ambient_zones){
            window.draw(zone.second.shape);
        }
     }
   }
   void new_obj(){
    objects[id_objects-1].shape.setTexture(&objects[id_objects-1].texture);
   }
};

map_collisions_class map_data;

void load_map(){//function for removing the start menu and setting the map up
    display_game = true;

    std::vector<int> map_size = GH::SQL::get_map_size(map_data.map_chosen); //get the amount of each object in the chosen map


    for(int i = 0 ; i < map_size.size() ; i++){ //checks if there isnt enough of certain types for the map to be useable
        if(i == 3){
            if(map_size[3] < 2){
                map_data.unusable_map = true;\
                std::cout<<"Unusable"<<std::endl;
                break;
            }
            else if(i != 5){
                if(map_size[i] == 0){
                    map_data.unusable_map = true;
                    std::cout<<"Unusable"<<std::endl;
                    break;
                }
            }
        }
    }

    //load objects into the map
    std::cout<<"loading objects"<<std::endl;
    pqxx::result objects = GH::SQL::load_asset(map_data.map_chosen, "objects");

                
    for(int i = 0 ; i < objects.size() ; i++){
        std::cout<<"loading "<<std::to_string(i+1)<<"/"<<map_size[0]<<std::endl;
        map_data.objects[i].shape.setSize({objects[i]["sizex"].get<float>().value(), objects[i]["sizey"].get<float>().value()});//get the size
        map_data.objects[i].shape.setOrigin({map_data.objects[i].shape.getSize().x/2, map_data.objects[i].shape.getSize().y/2});//get the origin
        map_data.objects[i].shape.setPosition({objects[i]["posx"].get<float>().value(), objects[i]["posy"].get<float>().value()});//get the position
        map_data.objects[i].theme = objects[i]["theme_id"].get<int>().value();//get which theme the object gets it texture from
        map_data.objects[i].index = objects[i]["texture_id"].get<int>().value();//what index from the chosen theme the texure used is
        map_data.objects[i].has_collision = objects[i]["collision"].get<bool>().value();//does the object have collision with the player
        map_data.objects[i].blocks_light = objects[i]["blocks_light"].get<bool>().value();//does the object block all light
        map_data.objects[i].throwable = objects[i]["throwable"].get<bool>().value();//can the ghost throw this object
        map_data.objects[i].texture = GH::SQL::load_texture(map_data.objects[i].theme, map_data.objects[i].index);//get the objects texture from the database
        map_data.objects[i].shape.setTexture(&map_data.objects[i].texture);

        map_data.objects[i].hitbox = sf::FloatRect(objects[i]["left_h"].get<int>().value(), objects[i]["top_h"].get<int>().value(), objects[i]["width_h"].get<int>().value(), objects[i]["height_h"].get<int>().value());//get the values for the hitbox of the object and set it up
    }
        
    std::cout<<"successfully loaded objects"<<std::endl;
    map_data.id_objects = map_size[0]; //store the amound of objects currently in the map
    std::cout<<"loading walls"<<std::endl;

    //load walls in map
    pqxx::result walls = GH::SQL::load_asset(map_data.map_chosen, "walls");
            
    for(int i = 0 ; i < walls.size() ; i++){
        
        map_data.walls[i].shape.setSize({walls[i]["sizex"].get<float>().value(), walls[i]["sizey"].get<float>().value()});//get the size
        map_data.walls[i].shape.setPosition({walls[i]["posx"].get<float>().value(), walls[i]["posy"].get<float>().value()});//get the position
        map_data.walls[i].index = walls[i]["texture_id"].get<int>().value();//get the index of the texture
        map_data.walls[i].rect = sf::IntRect(0,0,walls[i]["rect_width"].get<int>().value(),walls[i]["rect_height"].get<int>().value());//get the hitbox
        map_data.walls[i].shape.setTextureRect(map_data.walls[i].rect);//make sure it repeats the texture
        map_data.walls[i].has_collision = true;//walls always have collision
        map_data.walls[i].hitbox = map_data.walls[i].shape.getGlobalBounds();//set up the hitboxs
        if(map_data.walls[i].index != -1){//wall is not vertical so needs texture and needs a different hitbox
            map_data.walls[i].texture = textures.wall_textures[map_data.walls[i].index];
            map_data.walls[i].texture.setRepeated(true);
            map_data.walls[i].shape.setTexture(&map_data.walls[i].texture);
            map_data.walls[i].hitbox.height -= 76;
        }
    }
            
    std::cout<<"successfully loaded walls"<<std::endl;
    map_data.id_walls = map_size[1]; //store the amount of walls currently in the map
    std::cout<<"loading floors"<<std::endl;

    //load floors in map
    pqxx::result floors = GH::SQL::load_asset(map_data.map_chosen, "floors"); 
    for(int i = 0 ; i < floors.size() ; i++){
        std::cout<<"loading "<<std::to_string(i+1)<<"/"<<map_size[2]<<std::endl; 
        map_data.floors[i].shape.setSize({floors[i]["sizex"].get<float>().value(), floors[i]["sizey"].get<float>().value()});//get the size
        map_data.floors[i].shape.setPosition({floors[i]["posx"].get<float>().value(), floors[i]["posy"].get<float>().value()});//get the position
        map_data.floors[i].index = floors[i]["texture_id"].get<int>().value();//get the texture index of the floor
        map_data.floors[i].texture = textures.floor_textures[map_data.floors[i].index];//set texture
        map_data.floors[i].shape.setTexture(&map_data.floors[i].texture);
        map_data.floors[i].rect = sf::IntRect(0,0,floors[i]["rect_width"].get<int>().value(),floors[i]["rect_height"].get<int>().value());//rectangle for texture repeating
        map_data.floors[i].shape.setTextureRect(map_data.floors[i].rect);
    }
    std::cout<<"successfully loaded floors"<<std::endl;
    map_data.id_floors = map_size[2];

    std::cout<<"loading rooms"<<std::endl;

    //load room areas in map
    pqxx::result rooms = GH::SQL::load_asset(map_data.map_chosen, "rooms");

    for(int i = 0 ; i < rooms.size() ; i++){ 
        std::cout<<"loading "<<std::to_string(i+1)<<"/"<<map_size[3]<<std::endl;   
        map_data.room_zones[i].shape.setSize({rooms[i]["sizex"].get<float>().value(), rooms[i]["sizey"].get<float>().value()});//get size
        map_data.room_zones[i].shape.setPosition({rooms[i]["posx"].get<float>().value(), rooms[i]["posy"].get<float>().value()});//get position
        map_data.room_zones[i].shape.setFillColor(sf::Color(0,255,0,32));         
        map_data.room_zones[i].room_protection = 0; 
    }

    std::cout<<"successfully loaded rooms"<<std::endl;
    map_data.id_rooms = map_size[3];
    
    std::cout<<"loading light systems"<<std::endl;
    for(int i = 0 ; i < map_size[4] ; i++){  //Load all light systems within the map
        std::cout<<"loading "<<std::to_string(i+1)<<"/"<<map_size[4]<<std::endl;
        std::pair<pqxx::result,pqxx::result> results = GH::SQL::load_internal_system(map_data.map_chosen, i);
        std::cout<<results.first.size()<<std::endl;
        map_data.light_systems[i] = GH::CREATE::light_system{};
        for(auto light : results.first){
            candle::RadialLight temp_light;
            temp_light.setIntensity(light["intensity"].get<float>().value());
            temp_light.setRange(light["range"].get<float>().value());
            temp_light.setPosition({light["posx"].get<float>().value(), light["posy"].get<float>().value()});

            map_data.light_systems[i].lights.emplace_back(temp_light);
        }

        for(auto swich : results.second){
            sf::RectangleShape temp_sw;
            temp_sw.setSize({32,48});
            temp_sw.setOrigin({16,24});
            temp_sw.setPosition({swich["posx"].get<float>().value(), swich["posy"].get<float>().value()});

            map_data.light_systems[i].light_switches.emplace_back(temp_sw);
        }
    }
    std::cout<<"successfully loaded light systems"<<std::endl;

    std::cout<<"loading hiding places"<<std::endl;

    //load hiding areas in map
    pqxx::result hidings = GH::SQL::load_asset(map_data.map_chosen, "hiding");

    for(int i = 0 ; i < hidings.size() ; i++){ 
        std::cout<<"loading "<<std::to_string(i+1)<<"/"<<map_size[5]<<std::endl;   
        map_data.hiding_zones[i].shape.setSize({hidings[i]["sizex"].get<float>().value(), hidings[i]["sizey"].get<float>().value()});
        map_data.hiding_zones[i].shape.setPosition({hidings[i]["posx"].get<float>().value(), hidings[i]["posy"].get<float>().value()}); 
        map_data.hiding_zones[i].shape.setFillColor(sf::Color(0,255,255,30));          
    }

    std::cout<<"successfully loaded hiding places"<<std::endl;
    map_data.id_hiding = map_size[5];

    std::cout<<"loading ambient zones"<<std::endl;
    
    //load ambient areas in map
    pqxx::result ambients = GH::SQL::load_asset(map_data.map_chosen, "ambient");
    
    for(int i = 0 ; i < ambients.size() ; i++){ 
        std::cout<<"loading "<<std::to_string(i+1)<<"/"<<map_size[6]<<std::endl;   
        map_data.ambient_zones[i].shape.setSize({ambients[i]["sizex"].get<float>().value(), ambients[i]["sizey"].get<float>().value()});
        map_data.ambient_zones[i].shape.setPosition({ambients[i]["posx"].get<float>().value(), ambients[i]["posy"].get<float>().value()}); 
        map_data.ambient_zones[i].shape.setFillColor(sf::Color(0,255,255,30));          
    }

    std::cout<<"successfully loaded ambient zones"<<std::endl;
    map_data.id_ambient = map_size[6];
    

    //choose ghost for this game and attatch evidence
    std::ifstream ghosts("evidence.json");
    std::string content = GH::JSON::json_load(ghosts);
    rapidjson::Document doc;
    doc.Parse(content.c_str());
    ghosts.close();
    std::uniform_int_distribution<> dist(0, 21);
    int chosen = dist(gen);
    int count = 0;
    for(auto itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
        const std::string ghost_name = itr->name.GetString();
        const rapidjson::Value& evidences = itr->value;
        
        if(count == chosen){
            ghost.ghost_type = ghost_name;
            ghost.evidence[0] = evidences["Evidence1"].GetString();
            ghost.evidence[1] = evidences["Evidence2"].GetString();
            ghost.evidence[2] = evidences["Evidence3"].GetString();
            break;
        }
        count++;

    }
    std::cout<<"Ghost type has been chosen"<<std::endl;

    if(!map_data.unusable_map){//stuff can only happen if map is usable
        std::uniform_int_distribution<> choose_room(0, map_data.room_zones.size()-1);
        int chosen_room = choose_room(gen);
        ghost.chosen_room = &map_data.room_zones[chosen_room].shape;
        ghost.sprite.setPosition(ghost.chosen_room->getPosition());
        ghost.room_index = chosen_room;

        
        //Set up the gateway object on the map
        std::vector<int> ghost_room_objects;
        for(auto& object : map_data.objects){
            if(object.second.shape.getGlobalBounds().intersects(ghost.chosen_room->getGlobalBounds()) && (object.second.has_collision || object.second.throwable) && (object.second.shape.getSize().x > 16 || object.second.shape.getSize().y > 16)){
                ghost_room_objects.push_back(object.first);
            }
        }
        if(ghost_room_objects.size() != 0){
            std::uniform_int_distribution<> rand_obj(0,ghost_room_objects.size() - 1);

            ghost.gateway_obj = &map_data.objects[ghost_room_objects[rand_obj(gen)]].shape;
        }
        else{
            ghost.gateway_obj = &map_data.objects[0].shape;
        }
        ghost.gate.setSize({ghost.gateway_obj->getSize().x/2, ghost.gateway_obj->getSize().y/2});
        ghost.gate.setOrigin({ghost.gate.getSize().x/2, ghost.gate.getSize().y/2});
        ghost.gate.setPosition(ghost.gateway_obj->getPosition());

        if(ghost.evidence[0] == "Ethereal residue" || ghost.evidence[1] == "Ethereal residue" || ghost.evidence[2] == "Ethereal residue"){
            std::uniform_int_distribution<> rand_x(0, abs(ghost.chosen_room->getSize().x));
            std::uniform_int_distribution<> rand_y(0, abs(ghost.chosen_room->getSize().y));
            ghost.ethereal_residue.setPosition({ghost.chosen_room->getPosition().x + rand_x(gen), ghost.chosen_room->getPosition().y + rand_y(gen)});
        }

       
    }
    map_data.chest.setPosition({0,0});
    map_data.chest_index = 0;
    map_data.items_in_chest = {};
    for(auto& item : GH::START::equipment){
        for(int i = 0 ; i < item.second ; i++){
            map_data.items_in_chest.push_back(item.first);
        }
    }
    if(!map_data.unusable_map){ //Create the ground based on the size of the map from lowest wall to highest wall
        sf::Vector2f max,min;
        max = map_data.walls[0].shape.getTransform().transformPoint(map_data.walls[0].shape.getPoint(0));
        min = map_data.walls[0].shape.getTransform().transformPoint(map_data.walls[0].shape.getPoint(0));

        for(auto& wall : map_data.walls){
            for(int i = 0 ; i < 4 ; i++){
                sf::Vector2f point = wall.second.shape.getTransform().transformPoint(wall.second.shape.getPoint(i));
                if(point.x > max.x){
                    max.x = point.x;
                }
                if(point.y > max.y){
                    max.y = point.y;
                }
                if(point.x < min.x){
                    min.x = point.x;
                }
                if(point.y < min.y){
                    min.y = point.y;
                }
            }
        }
        map_data.grass[0].position = {min.x-1000,min.y-1000};
        map_data.grass[1].position = {max.x+1000,min.y-1000};
        map_data.grass[3].position = {min.x-1000,max.y+1000};
        map_data.grass[2].position = {max.x+1000,max.y+1000};

        //get the weather conditions from the API file to determine if it should be raining or not
        std::ifstream weather_json(MISC_DIR"/moon_phases.json");
        rapidjson::Document doc;
        std::string content = GH::JSON::json_load(weather_json);
        doc.Parse(content.c_str());
        weather_json.close();

        std::string weather = doc["current"]["condition"]["text"].GetString();
        map_data.raining = false;
        std::string temp = "";
        for(char a : weather){
            if(a == ' '){
                std::cout<<temp<<std::endl;
                if(temp == "Drizzle" || temp == "Rain"){
                    map_data.raining = true;
                    break;
                }
                else{
                    temp = "";
                }
            }
            else{
                temp += a;
            }
        }

        if(temp == "drizzle" || temp == "rain"){
            map_data.raining = true;
        }
        
        map_data.rain.setSize({abs(map_data.grass[0].position.x - map_data.grass[1].position.x), abs(map_data.grass[0].position.y - map_data.grass[2].position.y)});
        map_data.rain.setPosition(map_data.grass[0].position);
        map_data.rain.setTextureRect(sf::IntRect(0,0,map_data.rain.getSize().x/2,map_data.rain.getSize().y/2));
    }

    player_sprites.dead = false;
}

void delayed_emf(){//threaded to allow emf to remain active for a short perido of time after
    sf::Clock clock;
    while(clock.getElapsedTime().asSeconds() < 3){

    }
    ghost.emf = 0;
}

void run_speech_to_text(){//FAILED CONCEPT
    ispythonrunning = true;
    string filename = MISC_DIR"/speech_to_text.py";
    string command = "python ";
    command += filename;
    system(command.c_str());
    ispythonrunning = false;
}

void spell_check(std::string spell, int index){//check what spell the player is casting
    if(spell == "Dash"){
        GH::SPELLS::dash(player_sprites.body, delta_time, textures.player_new_facing, map_data.walls, index);
    }
    else if(spell == "Speed"){
        GH::SPELLS::speed(player_sprites.speed, index);
    }
    int room_protection;
    for(auto& room : map_data.room_zones){
        if(player_sprites.body.getGlobalBounds().intersects(room.second.shape.getGlobalBounds())){
            room_protection = room.second.room_protection;
            room.second.shape.setFillColor(sf::Color(255,0,255,32));
        }
        if(spell == "Basic protection"){
            room.second.room_protection = GH::SPELLS::protection(1, room_protection, index);
        }
        else if(spell == "Protection"){
            room.second.room_protection = GH::SPELLS::protection(2, room_protection, index);
        }
        else if(spell == "Enhanced protection"){
            room.second.room_protection = GH::SPELLS::protection(3, room_protection, index);
        }
        else if(spell == "Spirit form"){
            GH::SPELLS::spirit_form(index, player_sprites.body);
        }
        else if(spell == "Chaotic tp"){
            GH::SPELLS::tp(index, true, player_sprites.body, map_data.floors, map_data.walls);
        }
        else if(spell == "Controlled tp"){
            GH::SPELLS::tp(index, false, player_sprites.body, map_data.floors, map_data.walls);
        }
        break;
    }
}

//main
int main(){
    ImGui::SFML::Init(window);
    window.setFramerateLimit(120);
    srand(int(time(0)));//random used for any random based events
    //bool variables for create mode and flashlight
    bool flash_on = true;
    bool create_mode = false;
    bool object_selected = false;
    bool vertical_wall = false;
    bool is_anchored = false;
    bool is_recording = false;
    bool debug_window = false;
    bool debug_fog = false;
    bool start_haunt = true;
    bool changing_spectral_mode = false;
    bool entering_spectral = true;
    bool take_pic = false;
    bool mouse_on_widget = false;
    bool death_anim = false;
    bool ghost_in_light = false;

    int greeny_fog = 0;
    int haunt_length;
    
    sf::Texture wall_texture;
    wall_texture.loadFromFile(ASSETS_DIR"/textures/1_Interiors/16x16/Room_Builder_subfiles/Room_Builder_Walls_16x16.png");
    int placing = 0;

    int frame_count = 1;
    //these lines handle the creation of the lightsources and fog
    candle::RadialLight light;
    light.setRange(150);
    light.setBeamAngle(75);
    candle::EdgeVector edges;
    candle::LightingArea fog(candle::LightingArea::FOG,sf::Vector2f(0.f, 0.f),sf::Vector2f(W_WIDTH, W_HEIGHT));
    fog.setAreaColor(sf::Color::Black);
    fog.setAreaOpacity(20);
    debug_fog = true;

    GH::START::load_maps();
    
    //start the clocks used for several things
    sf::Clock clock;
    sf::Clock sanity_clock;
    sf::Clock haunt_timer;
    sf::Clock emf_change;
    sf::Clock fog_flicker;
    sf::Clock trapped;
    sf::Clock death_anim_delay;
    sf::Clock event_clock;
    sf::Clock phantom_clock;
    sf::Clock object_last_thrown;
    GH::API::get_moon_phase();
    GH::START::set_up();
    GH::JOURNAL::setup(W_WIDTH, W_HEIGHT, gui);
    GH::INV::setup();
    GH::SPELLS::set_up();
    GH::TEXTURES::PLAYER::start_up();
    GH::CREATE::start_up();
    GH::SOUNDS::GHOST::set_up();
    GH::CREATE::load_theme("Living");
    GH::INV::slots[1].setFillColor(sf::Color(255,255,255,50));
    GH::INV::slots[2].setFillColor(sf::Color(255,255,255,50));
    sf::Listener::setPosition(player_sprites.body.getPosition().x, player_sprites.body.getPosition().y, 0.f);
    sf::Listener::setDirection(0.f, 0.f, -1.f);
    sf::Listener::setUpVector(0.f, 1.f, 0.f);
    std::string in_pic;
    //start the game loop

    while(window.isOpen()){
        delta_time = clock.restart();//get the deltatime
        ImGui::SFML::Update(window, delta_time);
        sf::Event event;//used for handling events

        while(window.pollEvent(event)){//event loops
            ImGui::SFML::ProcessEvent(window, event);
            gui.handleEvent(event);
            if(event.type == sf::Event::Closed or sf::Keyboard::isKeyPressed(sf::Keyboard::Escape)){//closes the game
                window.close();
            }
            if(event.type == sf::Event::KeyPressed && display_game){
                if(event.key.code == sf::Keyboard::T){flash_on = !flash_on;}  //Toggle flashlight

                if(event.key.code == sf::Keyboard::N && !is_recording){  //Start recording microphone
                    if(GH::AUDIOREC::start_recording(44100, 2)){is_recording = true;}}

                if(event.key.code == sf::Keyboard::F3){debug_window = !debug_window;}
                if(event.key.code == sf::Keyboard::Num1){GH::INV::slot_change(1);}  //Switch inventory slot to slot 1
                if(event.key.code == sf::Keyboard::Num2){GH::INV::slot_change(2);}  //Switch inventory slot to slot 2
                if(event.key.code == sf::Keyboard::Num3){GH::INV::slot_change(3);}  //Switch inventory slot to slot 3
                if(event.key.code == sf::Keyboard::E){GH::INV::pick_up(player_sprites.body);}  //Pick up the item colliding with the player
                if(event.key.code == sf::Keyboard::G){GH::INV::drop(player_sprites.body);}  //Drop selected item;
                if(event.key.code == sf::Keyboard::J){GH::JOURNAL::toggle_journal(gui);}  //toggle the journal
                if(event.key.code == sf::Keyboard::Z){spell_check(GH::START::slots[2],2);} //cast spells
                if(event.key.code == sf::Keyboard::X){spell_check(GH::START::slots[1],1);}
                if(event.key.code == sf::Keyboard::C){spell_check(GH::START::slots[0],0);}
                if(event.key.code == sf::Keyboard::V){create_mode = true;}
                if(event.key.code == sf::Keyboard::M){GH::GHOST::BEHAV::current = "interact";}
                if(event.key.code == sf::Keyboard::K){GH::GHOST::BEHAV::current = "hunt";}
                if(event.key.code == sf::Keyboard::Q && !held[sf::Keyboard::Q]){ //Enter spectral mode
                    changing_spectral_mode = true; 
                    entering_spectral = true; 
                    held[sf::Keyboard::Q] = true;
                    audio.sounds["Enter_spectral"].play();
                }  
            }

            if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::N && is_recording){  //Stop recording when the N key is released
                is_recording = false;
                GH::AUDIOREC::stop_recording();
                GH::AUDIOREC::write_audio();
                thread run_python(run_speech_to_text);
                run_python.detach();
 
            }
            if(event.type == sf::Event::KeyReleased && event.key.code == sf::Keyboard::Q){
                changing_spectral_mode = true;
                entering_spectral = false;
                held[sf::Keyboard::Q] = false;
                audio.sounds["Enter_spectral"].stop();
            }

            if(event.type == sf::Event::MouseButtonReleased){  //checks if the mouse is being held
                held[sf::Keyboard::F10] = false;
                GH::INV::locked_on = false;
            }


            if(sf::Mouse::isButtonPressed(sf::Mouse::Left) || GH::INV::locked_on){
                GH::INV::move_item(window, map_data.walls, map_data.walls);
            }

            if(event.type == sf::Event::MouseButtonPressed && event.mouseButton.button == sf::Mouse::Left){
                if(map_data.chest.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window)))){
                    if(map_data.animate_chest){
                        GH::INV::load_in_item(map_data.items_in_chest[map_data.chest_index], map_data.chest.getPosition());
                        map_data.chest_index += 1;
                    }
                }
            }


            
            if(event.type == sf::Event::KeyReleased){
                held[event.key.code] = false;

            }

            if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10]){          
                for(auto& system : map_data.light_systems){
                    for(auto& swich : system.second.light_switches){
                        if(swich.getGlobalBounds().contains(window.mapPixelToCoords(sf::Mouse::getPosition(window))) && system.second.flicker_time == -1){
                            held[sf::Keyboard::F10] = true;
                            system.second.lights_on = !system.second.lights_on;
                           
                        }
                    }
                }
            }
            
        }

        mouse_on_widget = false;
        for(auto& widget : gui.getWidgets()){
            if(widget->isMouseOnWidget(window.mapPixelToCoords(sf::Mouse::getPosition()))){
                mouse_on_widget = true;
                break;
            }
        }

        if(debug_window){//debug menu for testing
           ImGui::Begin("Debug window");
           std::string FPS = "Fps: " + std::to_string(1.0f / delta_time.asSeconds());
           ImGui::Text(FPS.c_str());

           int rendered_walls = 0;
           for(auto& wall : map_data.walls){
            if(GH::MATH::rect_in_view(wall.second.shape.getGlobalBounds(), view)){
                rendered_walls += 1;
            }
           }
           ImGui::Text(("Rendered walls: " + std::to_string(rendered_walls)).c_str());

           int rendered_objects = 0;
           for(auto& object : map_data.objects){
            if(GH::MATH::rect_in_view(object.second.shape.getGlobalBounds(), view)){
                rendered_objects += 1;
            }
           }
           ImGui::Text(("Rendered objects: " + std::to_string(rendered_objects)).c_str());

           ImGui::Text(("Ghost's current behavior: " + GH::GHOST::BEHAV::current).c_str());
           ImGui::Text(("Ghost's previous behavior: " + ghost.previous_event).c_str());
           ImGui::Text(("Current EMF lvl: " + to_string(ghost.emf)).c_str());
           ImGui::Text(("Activity level: " + to_string(GH::GHOST::BEHAV::activity_value)).c_str());
           ImGui::Text(("Event level: " + to_string(GH::GHOST::BEHAV::event)).c_str());
           ImGui::Text(("Event weights:\n Sound: " + std::to_string(GH::GHOST::BEHAV::events_map["sound"]) +
           "\n Interact: " + std::to_string(GH::GHOST::BEHAV::events_map["interact"]) +
           "\n Haunt: " + std::to_string(GH::GHOST::BEHAV::events_map["haunt"]) +
           "\n Evidence: " + std::to_string(GH::GHOST::BEHAV::events_map["evidence"]) +
           "\n Communicate: " + std::to_string(GH::GHOST::BEHAV::events_map["communicate"])
            ).c_str());
            ImGui::Text(("Sanity level: " + std::to_string(player_sprites.sanity)).c_str());
            ImGui::Text(("Stamina: " + std::to_string(player_sprites.stamina)).c_str());
            
            ImGui::Text(("Ghost type: " + ghost.ghost_type).c_str());
           ImGui::Text(("Evidences: \n" + ghost.evidence[0] + "\n" + ghost.evidence[1] + "\n" + ghost.evidence[2]).c_str());

           if(ImGui::Button("Toggle fog", ImVec2(80,20))){
            debug_fog = !debug_fog;
           }
           ImGui::End();
        }

        if(create_mode){//contains anything to do with create mode, the windows used for create mod are pulled from the external create_mode file
            debug_fog = true;
            view.setSize({W_WIDTH,W_HEIGHT});
            sf::Vector2i wm_pos = sf::Mouse::getPosition(window);
            sf::Vector2f m_pos = window.mapPixelToCoords(wm_pos);
            

            string create_mode = GH::CREATE::window_mode();
            map_data.show_zones = GH::CREATE::show_zones();
            
            if(create_mode == "Create"){
                mouse_hover.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                mouse_hover.setFillColor(sf::Color(255,255,255,255));
                mouse_hover.setOrigin({mouse_hover.getSize().x/2, mouse_hover.getSize().y/2});
                pair<string, GH::CREATE::obj> info = GH::CREATE::window_create();

                if(info.first != ""){
                    mouse_hover_txt = info.second.texture;
                    mouse_hover.setTexture(&mouse_hover_txt, true);
                    mouse_hover.setSize({info.second.size.first*5, info.second.size.second*5});




                    if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                        held[sf::Keyboard::F10] = true;
                        GH::CREATE::obj base_obj;
                        base_obj.index = info.second.index;
                        base_obj.theme = info.second.theme;
                        base_obj.blocks_light = info.second.blocks_light;
                        base_obj.has_collision = info.second.has_collision;
                        base_obj.throwable = info.second.throwable;
                        base_obj.texture = GH::SQL::load_texture(base_obj.theme, base_obj.index);
                        base_obj.shape.setTexture(&base_obj.texture);
                        base_obj.shape.setOrigin({mouse_hover.getSize().x/2, mouse_hover.getSize().y/2});
                        base_obj.shape.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                        base_obj.shape.setSize({base_obj.texture.getSize().x*5, base_obj.texture.getSize().y*5});
                        base_obj.shape.setScale(mouse_hover.getScale());
                        int id = map_data.id_objects;
                        map_data.objects[id] = base_obj;
                        vector<sf::Vector2f> vertices = GH::MATH::vertex_hitbox(map_data.objects[id].shape, mouse_hover_txt);
                       
                        sf::VertexArray vertex_array(sf::Quads, 4);
                        for(int i = 0 ; i < 4 ; i++){
                            vertex_array[i].position = vertices[i];
                            vertex_array[i].color = sf::Color::Transparent;
                        }
                        float left = vertex_array[0].position.x;
                        float top = vertex_array[0].position.y;
                        float right = left;
                        float bottom = top;

                        for (std::size_t i = 1; i < vertex_array.getVertexCount(); ++i) {
                            const sf::Vector2f& pos = vertex_array[i].position;
                            left = std::min(left, pos.x);
                            top = std::min(top, pos.y);
                            right = std::max(right, pos.x);
                            bottom = std::max(bottom, pos.y);
                        }

                        map_data.objects[id].hitbox = sf::FloatRect(left, top, right - left, bottom - top);

                        map_data.id_objects += 1;
                        map_data.new_obj();

                    }

                    if(sf::Keyboard::isKeyPressed(sf::Keyboard::T) && !held[sf::Keyboard::T]){
                        held[sf::Keyboard::T] = true;
                        if(mouse_hover.getScale().x != -1){
                            mouse_hover.setScale(-1,1);
                        }
                        else{
                            mouse_hover.setScale(1,1);
                        }
                    }
                }
            }
        
            else if(create_mode == "Delete"){
                mouse_hover.setTexture(NULL, true);
                mouse_hover.setSize({10,10});

                mouse_hover.setPosition(m_pos);
                mouse_hover.setOrigin({mouse_hover.getSize().x/2, mouse_hover.getSize().y/2});
                bool has_deleted = false;

                if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                    held[sf::Keyboard::F10] = true;
                    for(auto& object : map_data.objects){
                        if(object.second.shape.getGlobalBounds().intersects(mouse_hover.getGlobalBounds())){
                            map_data.objects.erase(object.first);
                            has_deleted = true;
                            break;
                        }
                    }
                    if(!has_deleted){
                        for(auto& object : map_data.walls){
                            if(object.second.shape.getGlobalBounds().intersects(mouse_hover.getGlobalBounds())){
                                map_data.walls.erase(object.first);
                                has_deleted = true;
                                break;
                            }
                        }
                    }
                    if(!has_deleted){
                        for(auto& object : map_data.floors){
                            if(object.second.shape.getGlobalBounds().intersects(mouse_hover.getGlobalBounds())){
                                map_data.floors.erase(object.first);
                                has_deleted = true;
                                break;
                            }
                        }
                    }
                }
            }
            
            else if(create_mode == "Walls"){
                int info = GH::CREATE::window_walls();
                mouse_hover.setOrigin(0,0);
                mouse_hover.setScale(1,1);
                mouse_hover.setTextureRect(sf::IntRect(0,0,32,32));
                if(!vertical_wall){
                    if(placing == 0){
                        mouse_hover.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                        mouse_hover.setSize({16*10,16*10});
                    }
                    
                    if(mouse_hover.getTexture() != &textures.wall_textures[info]){
                    textures.wall_textures[info].setRepeated(true);
                    mouse_hover.setTexture(&textures.wall_textures[info]);
                    }
                }

                else{
                    if(placing == 0){
                        mouse_hover.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                        mouse_hover.setSize({2*10,2*10});
                    }
                    
                    if(mouse_hover.getTexture() != NULL){
                    mouse_hover.setTexture(NULL);
                    }
                }

                if(placing == 1){
                     if(!vertical_wall){
                        mouse_hover.setSize({(std::floor(m_pos.x / 10) * 10 + 10/2) - mouse_hover.getPosition().x, mouse_hover.getSize().y});
                        mouse_hover.setTextureRect(sf::IntRect(0, 0, mouse_hover.getSize().x/5, mouse_hover.getSize().y/5));
                     }
                     else{
                        mouse_hover.setSize({mouse_hover.getSize().x, (std::floor(m_pos.y / 5) * 5 + 5/2) - mouse_hover.getPosition().y});
                        mouse_hover.setTextureRect(sf::IntRect(0, 0, mouse_hover.getSize().x/5, mouse_hover.getSize().y/5));       
                     }
                }
              
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                    held[sf::Keyboard::F10] = true;

                    if(placing == 0){
                        placing = 1;
                    }
                    else if(placing == 1){
                        GH::CREATE::obj temp_obj;
                        temp_obj.texture = textures.wall_textures[info];
                        temp_obj.rect = mouse_hover.getTextureRect();
                        int id = map_data.id_walls;
                        map_data.walls[id] = temp_obj;
                        map_data.walls[id].shape.setPosition(mouse_hover.getPosition());
                        map_data.walls[id].shape.setSize(mouse_hover.getSize());
                        map_data.walls[id].texture.setRepeated(true);
                        map_data.walls[id].shape.setTexture(&map_data.walls[id].texture);
                        map_data.walls[id].shape.setTextureRect(map_data.walls[id].rect);
                        map_data.walls[id].index = info;
                        map_data.walls[id].rect = mouse_hover.getTextureRect();

                        sf::VertexArray vertex_array(sf::Quads, 4);
                        vertex_array[0].position = map_data.walls[id].shape.getTransform().transformPoint(map_data.walls[id].shape.getPoint(0));
                        vertex_array[1].position = map_data.walls[id].shape.getTransform().transformPoint(map_data.walls[id].shape.getPoint(1));
                        vertex_array[2].position = map_data.walls[id].shape.getTransform().transformPoint(map_data.walls[id].shape.getPoint(2));
                        vertex_array[3].position = map_data.walls[id].shape.getTransform().transformPoint(map_data.walls[id].shape.getPoint(3));

                        if(vertical_wall){
                            map_data.walls[id].index = -1;
                            map_data.walls[id].shape.setTexture(NULL);
                        }
                        else{
                            vertex_array[2].position.y -= 76;
                            vertex_array[3].position.y -= 76;
                        }
                        placing = 0;

                        float left = vertex_array[0].position.x;
                        float top = vertex_array[0].position.y;
                        float right = left;
                        float bottom = top;

                        for (std::size_t i = 1; i < vertex_array.getVertexCount(); ++i) {
                            const sf::Vector2f& pos = vertex_array[i].position;
                            left = std::min(left, pos.x);
                            top = std::min(top, pos.y);
                            right = std::max(right, pos.x);
                            bottom = std::max(bottom, pos.y);
                        }

                        map_data.walls[id].hitbox = sf::FloatRect(left, top, right - left, bottom - top);

                        map_data.id_walls += 1;

                    }
                }
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::V) && !held[sf::Keyboard::V] && placing == 0){
                    held[sf::Keyboard::V] = true;
                    vertical_wall = !vertical_wall;
                }


            }
        
            else if(create_mode == "Select"){
                mouse_hover.setTexture(NULL, true);
                mouse_hover.setSize({10,10});

                mouse_hover.setPosition(m_pos);
                mouse_hover.setOrigin({mouse_hover.getSize().x/2, mouse_hover.getSize().y/2});

                if(object_selected){
                    GH::CREATE::window_select(map_data.object_ptr);
                }

                if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                   held[sf::Keyboard::F10] = true;
                   bool found_obj = false;
                   for(auto& object : map_data.objects){
                       if(object.second.shape.getGlobalBounds().intersects(mouse_hover.getGlobalBounds())){
                          map_data.object_ptr = &object.second;
                          object_selected = true;
                          found_obj = true;
                          break;
                       }
                   }
                   if(!found_obj){
                        for(auto& object : map_data.walls){
                            if(object.second.shape.getGlobalBounds().intersects(mouse_hover.getGlobalBounds())){
                                map_data.object_ptr = &object.second;
                                object_selected = true;
                                found_obj = true;
                                break;
                            }
                        }
                   }
                   if(!found_obj){
                        for(auto& object : map_data.floors){
                            if(object.second.shape.getGlobalBounds().intersects(mouse_hover.getGlobalBounds())){
                                map_data.object_ptr = &object.second;
                                object_selected = true;
                                break;
                            }
                        }
                   }
                }

                if(sf::Keyboard::isKeyPressed(sf::Keyboard::V) && !held[sf::Keyboard::V]){
                    held[sf::Keyboard::V] = true;
                    
                    is_anchored = !is_anchored;
                }

                if(is_anchored){
                        map_data.object_ptr->shape.setPosition(mouse_hover.getPosition());
                }
            }

            else if(create_mode == "Floors"){
                int info = GH::CREATE::window_floors();
                mouse_hover.setOrigin(0,0);
                mouse_hover.setScale(1,1);
                mouse_hover.setTextureRect(sf::IntRect(0,0,32,32));
                if(placing == 0){
                    mouse_hover.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                    mouse_hover.setSize({16*10,16*10});
                }
                    
                if(mouse_hover.getTexture() != &textures.floor_textures[info]){
                    textures.wall_textures[info].setRepeated(true);
                    mouse_hover.setTexture(&textures.floor_textures[info]);
                }
        
                if(placing == 1){
                    mouse_hover.setSize({(std::floor(m_pos.x / 10) * 10 + 10/2) - mouse_hover.getPosition().x, (std::floor(m_pos.y / 10) * 10 + 10/2) - mouse_hover.getPosition().y});
                    mouse_hover.setTextureRect(sf::IntRect(0, 0, mouse_hover.getSize().x/5, mouse_hover.getSize().y/5));       
                }
              
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                    held[sf::Keyboard::F10] = true;
                    if(placing == 0){
                        placing = 1;
                    }
                    else if(placing == 1){
                        GH::CREATE::obj temp_obj;
                        temp_obj.texture = textures.floor_textures[info];
                        temp_obj.rect = mouse_hover.getTextureRect();
                        int id = map_data.id_floors;
                        map_data.floors[id] = temp_obj;
                        map_data.floors[id].shape.setPosition(mouse_hover.getPosition());
                        map_data.floors[id].shape.setSize(mouse_hover.getSize());
                        map_data.floors[id].texture.setRepeated(true);
                        map_data.floors[id].shape.setTexture(&map_data.floors[id].texture);
                        map_data.floors[id].shape.setTextureRect(map_data.floors[id].rect);
                        map_data.floors[id].index = info;
                        map_data.id_floors+=1;
                        placing = 0;
                    }
                }

            }
            
            else if(create_mode == "Save"){
                GH::SQL::delete_map_data(map_data.map_chosen);
                GH::SQL::save_map_size(map_data.map_chosen, map_data.objects.size(), map_data.walls.size(), map_data.floors.size(), map_data.room_zones.size(), map_data.light_systems.size(), map_data.hiding_zones.size(), map_data.ambient_zones.size());
                for(auto& object : map_data.objects){  //save objects to database
                    GH::SQL::save_asset("object", object.second, map_data.map_chosen, object.first);
                }
                for(auto& wall : map_data.walls){  //save wall to database
                    GH::SQL::save_asset("wall", wall.second, map_data.map_chosen, wall.first);
                }
                for(auto& floor : map_data.floors){  //save floor to database 
                    GH::SQL::save_asset("floor", floor.second, map_data.map_chosen, floor.first);
                }
                for(auto& room : map_data.room_zones){  //save rooms to database
                    GH::SQL::save_asset("room", room.second, map_data.map_chosen, room.first);
                }
                for(auto& hiding : map_data.hiding_zones){  //save hiding zones to database
                    GH::SQL::save_asset("hiding", hiding.second, map_data.map_chosen, hiding.first);
                }
                for(auto& ambient : map_data.ambient_zones){  //save hiding zones to database
                    GH::SQL::save_asset("ambient", ambient.second, map_data.map_chosen, ambient.first);
                }
                GH::SQL::save_light_systems(map_data.light_systems, map_data.map_chosen);  //save light systems to database
                GH::CREATE::mode = "create";
            }
        
            else if(create_mode == "Rooms"){
                mouse_hover.setOrigin(0,0);
                mouse_hover.setScale(1,1);
                mouse_hover.setTextureRect(sf::IntRect(0,0,32,32));
                mouse_hover.setFillColor(sf::Color(0,255,0,32));
                mouse_hover.setTexture(NULL);
                if(placing == 0){
                    mouse_hover.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                    mouse_hover.setSize({16*10,16*10});
                }
                    
                if(placing == 1){
                    mouse_hover.setSize({(std::floor(m_pos.x / 10) * 10 + 10/2) - mouse_hover.getPosition().x, (std::floor(m_pos.y / 10) * 10 + 10/2) - mouse_hover.getPosition().y});  
                }
              
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                    held[sf::Keyboard::F10] = true;
                    if(placing == 0){
                        placing = 1;
                    }
                    else if(placing == 1){
                        GH::CREATE::obj temp_obj;
                        int id = map_data.id_rooms;
                        map_data.room_zones[id] = temp_obj;
                        map_data.room_zones[id].shape.setPosition(mouse_hover.getPosition());
                        map_data.room_zones[id].shape.setSize(mouse_hover.getSize());
                        map_data.room_zones[id].shape.setFillColor(sf::Color(0,255,0,32));
                        map_data.id_rooms+=1;
                        placing = 0;
                    }
                }
            }
            
            else if(create_mode == "Hiding"){
                mouse_hover.setOrigin(0,0);
                mouse_hover.setScale(1,1);
                mouse_hover.setTextureRect(sf::IntRect(0,0,32,32));
                mouse_hover.setFillColor(sf::Color(0,255,255,32));
                mouse_hover.setTexture(NULL);
                if(placing == 0){
                    mouse_hover.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                    mouse_hover.setSize({16*10,16*10});
                }
                    
                if(placing == 1){
                    mouse_hover.setSize({(std::floor(m_pos.x / 10) * 10 + 10/2) - mouse_hover.getPosition().x, (std::floor(m_pos.y / 10) * 10 + 10/2) - mouse_hover.getPosition().y});  
                }
              
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                    held[sf::Keyboard::F10] = true;
                    if(placing == 0){
                        placing = 1;
                    }
                    else if(placing == 1){
                        GH::CREATE::obj temp_obj;
                        int id = map_data.id_rooms;
                        map_data.hiding_zones[id] = temp_obj;
                        map_data.hiding_zones[id].shape.setPosition(mouse_hover.getPosition());
                        map_data.hiding_zones[id].shape.setSize(mouse_hover.getSize());
                        map_data.hiding_zones[id].shape.setFillColor(sf::Color(0,255,255,32));
                        map_data.id_hiding+=1;
                        placing = 0;
                    }
                }
            }

            else if(create_mode == "Ambient"){
                mouse_hover.setOrigin(0,0);
                mouse_hover.setScale(1,1);
                mouse_hover.setTextureRect(sf::IntRect(0,0,32,32));
                mouse_hover.setFillColor(sf::Color(255,0,255,32));
                mouse_hover.setTexture(NULL);
                if(placing == 0){
                    mouse_hover.setPosition({std::floor(m_pos.x / 10) * 10 + 10/2, std::floor(m_pos.y / 10) * 10 + 10/2});
                    mouse_hover.setSize({16*10,16*10});
                }
                    
                if(placing == 1){
                    mouse_hover.setSize({(std::floor(m_pos.x / 10) * 10 + 10/2) - mouse_hover.getPosition().x, (std::floor(m_pos.y / 10) * 10 + 10/2) - mouse_hover.getPosition().y});  
                }
              
                if(sf::Mouse::isButtonPressed(sf::Mouse::Left) && !held[sf::Keyboard::F10] && !ImGui::GetIO().WantCaptureMouse){
                    held[sf::Keyboard::F10] = true;
                    if(placing == 0){
                        placing = 1;
                    }
                    else if(placing == 1){
                        GH::CREATE::obj temp_obj;
                        int id = map_data.id_ambient;
                        map_data.ambient_zones[id] = temp_obj;
                        map_data.ambient_zones[id].shape.setPosition(mouse_hover.getPosition());
                        map_data.ambient_zones[id].shape.setSize(mouse_hover.getSize());
                        map_data.ambient_zones[id].shape.setFillColor(sf::Color(255,0,255,32));
                        map_data.id_ambient+=1;
                        placing = 0;
                    }
                }
            }

            else if(create_mode == "Lights"){
                debug_fog = false;
                GH::CREATE::lights(map_data.light_systems);
                if(sf::Keyboard::isKeyPressed(sf::Keyboard::V) && !held[sf::Keyboard::V]){
                    held[sf::Keyboard::V] = true;
                    is_anchored = !is_anchored;
                }
                if(is_anchored && GH::CREATE::chosen_light != nullptr){
                    GH::CREATE::chosen_light->setPosition(m_pos);
                }
                else if(is_anchored && GH::CREATE::chosen_switch != nullptr){
                    GH::CREATE::chosen_switch->setPosition(m_pos);
                }
            }
        }
        else if(!create_mode && view.getSize() != sf::Vector2f(W_WIDTH/2,W_HEIGHT/2)){
            view.setSize({W_WIDTH/2,W_HEIGHT/2});
        }

        light.setPosition(player_sprites.body.getPosition());//set the flashlight position to the player's position
        sf::Vector2i mousepos = sf::Mouse::getPosition();//get the psotion of the mouse
        sf::Vector2f center = {window.getSize().x / 2.0f, window.getSize().y / 2.0f};//get the center of the window
        light.setRotation(GH::MATH::getmouseang(mousepos, center));//set the flashlight so it is always facing the mouse, allowing easy rotation
        edges.clear();

        for(auto& shape : map_data.objects){//make sure the flashlight and other lights doesnt pass through any objects/sprites
          if(shape.second.blocks_light){
            map<int, pair<sf::Vector2f, sf::Vector2f>> points = GH::MATH::get_points(shape.second.shape);
            for(auto& edge : points){
                edges.emplace_back(edge.second.first, edge.second.second);
            }
          }
        }

        for(auto& wall : map_data.walls){ //make sure the flashlight and other lights doesnt pass through any walls
          if(wall.second.index == -1){
            map<int, pair<sf::Vector2f, sf::Vector2f>> points = GH::MATH::get_points(wall.second.shape);
            for(auto& edge : points){
                edges.emplace_back(edge.second.first, edge.second.second);
            }
          }
          else{
            map<int, pair<sf::Vector2f, sf::Vector2f>> points = GH::MATH::get_points(wall.second.shape);
            edges.emplace_back(points[0].first, points[0].second);
          }
        }

        light.castLight(edges.begin(), edges.end());//cast the flashlight
        for(auto& system : map_data.light_systems){//cast other lights on the map
            for(auto& light : system.second.lights){
                light.castLight(edges.begin(), edges.end());
            }
        }
        if(display_game){
            if(GH::JOURNAL::end){  //End game
                    map_data.id_floors = 0;
                    map_data.id_walls = 0;
                    map_data.id_objects = 0;
                    map_data.id_rooms = 0;
                    map_data.floors = {};
                    map_data.walls = {};
                    map_data.objects = {};
                    map_data.room_zones = {};

                    player_sprites.sanity = 100;
                    player_sprites.stamina = 100;
                    player_sprites.dead = false;
                    flash_on = true;
                    create_mode = false;
                    object_selected = false;
                    vertical_wall = false;
                    is_anchored = false;
                    is_recording = false;
                    debug_window = false;
                    debug_fog = true;
                    start_haunt = true;
                    changing_spectral_mode = false;
                    entering_spectral = true;
                    take_pic = false;
                    mouse_on_widget = false;
                    map_data.chest_empty = false;
                    map_data.next_item = true;
                    map_data.animate_chest = false;
                    greeny_fog = 0;

                    player_sprites.body.setPosition(100,500);
                    player_sprites.body.setFillColor(sf::Color(255,255,255,255));
                    ghost.sprite.setFillColor(sf::Color(85,255,230,0));
                    window.setView(window.getDefaultView());
                    GH::GHOST::BEHAV::current = "NILL";

                    GH::INV::reset();
                    GH::JOURNAL::reset(false);

                    display_game = false;
            }
            sf::Vector2i wm_pos = sf::Mouse::getPosition(); //The mouse's position is used to make sure items are only ever taken out of the chest if the mouse is on the chest
            sf::Vector2f m_pos = window.mapPixelToCoords(wm_pos);

            
            
            if(abs(player_sprites.body.getPosition().x - map_data.chest.getPosition().x) < 100 && abs(player_sprites.body.getPosition().y - map_data.chest.getPosition().y) < 100){
                map_data.animate_chest = true;
            }
            else{
                map_data.animate_chest = false;
            }
            if(map_data.chest_index == map_data.items_in_chest.size()){
                map_data.animate_chest = false;
            }
            
            //This if statement, when the chest should be opening, activates every 20 frames and changes the chests texture by one, giving a smooth animation
            if(map_data.chest.getTextureRect() != sf::IntRect(97,0,14,24) && frame_count%5 == 0 && map_data.animate_chest){
                sf::IntRect new_rect = map_data.chest.getTextureRect();
                new_rect.left += 16;
                map_data.chest.setTextureRect(new_rect);
            }
            
            //When the chest is empty this if statement becomes active every 20 frames to instead close the chest
            if(map_data.chest.getTextureRect() != sf::IntRect(1,0,14,24) && frame_count%5 == 0 && !map_data.animate_chest){
                sf::IntRect new_rect = map_data.chest.getTextureRect();
                new_rect.left -= 16;
                map_data.chest.setTextureRect(new_rect);
            }
           
            /*END OF EQUIPMENT CHEST OPENING*/
            if(!player_sprites.dead && !GH::SPELLS::is_dashing){
                GH::SPELLS::spell_check({{0,"Speed"},{1,"e"},{2,"e"}}, player_sprites.speed);
                GH::MATH::stamina_check(player_sprites.stamina, player_sprites.speed);
                auto[a,b] = GH::MATH::check_velocity(delta_time, player_sprites.speed, M_WIDTH, M_HEIGHT, player_sprites.body, map_data.walls, player_sprites.hitboxes, view, player_sprites.previous_pos);//check if the player is trying to move, and then move them
                player_sprites.align();
                textures.player_new_facing = a;
                textures.mode = b;
            }
            else if(GH::SPELLS::is_dashing){
                for(int i = 0 ; i < 3 ; i++){
                    if(GH::START::slots[i] == "Dash"){
                        GH::SPELLS::dash(player_sprites.body, delta_time, textures.player_new_facing, map_data.walls, i);
                    }
                }
            }
            if(GH::SPELLS::is_spirit){
                for(int i = 0 ; i < 3 ; i++){
                    if(GH::START::slots[i] == "Spirit form"){
                        GH::SPELLS::spirit_form(i, player_sprites.body);
                    }
                }
            }

            if(textures.player_new_facing != textures.player_facing){
                textures.player_facing = textures.player_new_facing;
                textures.update_player_rect(textures.mode+to_string(textures.anim_index));
            }

            if(frame_count%15 == 0){
                textures.change_anim_index();
                textures.update_player_rect(textures.mode+to_string(textures.anim_index));
            }
            
            if(!GH::SPELLS::is_spirit){
                GH::MATH::prevent_collisions(player_sprites.hitboxes, view, player_sprites.body, delta_time, player_sprites.speed, map_data.objects, player_sprites.previous_pos);//prevents the player from clipping into collision
                GH::MATH::prevent_collisions(player_sprites.hitboxes, view, player_sprites.body, delta_time, player_sprites.speed, map_data.walls, player_sprites.previous_pos);
            }
            player_sprites.align();
            view.setCenter(player_sprites.body.getPosition());//center the view to the player
            sf::Listener::setPosition(player_sprites.body.getPosition().x, player_sprites.body.getPosition().y, 0.f);
            fog.setPosition(sf::Vector2f(view.getCenter().x-W_WIDTH/2,view.getCenter().y-W_HEIGHT/2));//set the fog to follow the window
            //Change color of fog based on in game events/interactions
            std::uniform_int_distribution<> chance_flick(1, 10);
            if(!player_sprites.dead){
                if(changing_spectral_mode){
                    if(entering_spectral){
                        greeny_fog += 1;
                        if(greeny_fog == 50){
                            changing_spectral_mode = false;
                        }
                    }
                    else{
                        greeny_fog -= 1;
                        if(greeny_fog == 0){
                            changing_spectral_mode = false;
                        }
                    }
                }
                if(chance_flick(gen) == 10 && greeny_fog == 50 && fog_flicker.getElapsedTime().asMilliseconds() >= 30 && player_sprites.sanity < 50){
                    fog_flicker.restart();
                    greeny_fog -= chance_flick(gen);
                    changing_spectral_mode = true;

                }
                else if(fog_flicker.getElapsedTime().asMilliseconds() >= 30){
                    fog_flicker.restart();
                }
                fog.setAreaColor(sf::Color(0,greeny_fog,0));
            }


            window.setView(view);//set the view to the window
            ghost.play_emf();
            if(!map_data.unusable_map && !player_sprites.dead && ghost.ghost_type != "Funnel" && !create_mode){
                ghost.speed = 200;
                ghost_in_light = false;
                for(auto& item : GH::INV::loaded_items){  //Check if the ghost is in lantern light
                    if(item.has_light){
                        if(item.light_filter.getGlobalBounds().intersects(ghost.sprite.getGlobalBounds())){
                            ghost_in_light = true;
                            break;
                        }
                    }
                }

                GH::GHOST::BEHAV::ghost_ability_check(ghost_in_light, ghost.seen_player, ghost.ghost_type, ghost.speed, player_sprites.sanity);
                if(GH::GHOST::BEHAV::current != "trapped" && GH::GHOST::BEHAV::current != "hunt" && event_clock.getElapsedTime().asSeconds() > 1){
                    event_clock.restart();
                    GH::GHOST::BEHAV::event_tick(delta_time, player_sprites.sanity, ghost.ghost_type, ghost.previous_event);
                    if(player_sprites.sanity <= 50){
                        GH::GHOST::BEHAV::hunt_tick(delta_time, player_sprites.sanity);
                    }
                }
                if(GH::GHOST::BEHAV::current != "haunt" && GH::GHOST::BEHAV::current != "trapped" && GH::GHOST::BEHAV::current != "hunt"){
                    ghost.facing = GH::GHOST::BEHAV::wander(ghost.speed, ghost.sprite, ghost.walk_path, *ghost.chosen_room, delta_time, map_data.room_zones, false);
                }
                ghost.upd_facing();
                if(GH::GHOST::BEHAV::current == "sound"){
                    GH::SOUNDS::GHOST::play_sound();
                    ghost.previous_event = "sound";
                    GH::GHOST::BEHAV::current = "NILL";
                }
                else if(GH::GHOST::BEHAV::current == "interact"){
                    if(GH::GHOST::BEHAV::current != "trapped"){
                        for(auto& equipment : GH::INV::loaded_items){
                            if(equipment.name == "Mirror" && equipment.shape.getGlobalBounds().intersects(ghost.interaction_radius.getGlobalBounds()) && (ghost.evidence[0] == "Silver mirror" || ghost.evidence[1] == "Silver mirror" || ghost.evidence[2] == "Silver mirror")){
                                equipment.name = "Trapped Mirror";
                                equipment.shape.setTextureRect(GH::INV::texture_rects[12]);
                                GH::GHOST::BEHAV::current = "trapped";
                                audio.sounds["Whisper"].setPosition({equipment.shape.getPosition().x, equipment.shape.getPosition().y, 0});
                                audio.sounds["Whisper"].play();
                                trapped.restart();
                                break;
                            }
                        }
                    }
                    if(GH::GHOST::BEHAV::current != "trapped"){
                        if(!GH::GHOST::BEHAV::currently_throwing){
                            ghost.emf = 3;
                            std::uniform_int_distribution<> emf5(1,10);
                            if(emf5(gen) == 1 && (ghost.evidence[0] == "EMF 5" || ghost.evidence[1] == "EMF 5" || ghost.evidence[2] == "EMF 5")){
                                ghost.emf = 5;
                            }
                            thread delay_emf(delayed_emf);
                            delay_emf.detach();
                        }
                        object_last_thrown.restart();
                        map_data.priority_object = GH::GHOST::BEHAV::throw_object(map_data.objects, ghost.interaction_radius, delta_time, map_data.walls, map_data.floors, ghost.can_UV);

                        if(ghost.evidence[0] == "UV" || ghost.evidence[1] == "UV" || ghost.evidence[2] == "UV" && ghost.can_UV){
                            ghost.UV.setPosition(map_data.objects[map_data.priority_object].shape.getPosition());
                        }
                    }
                }
                else if(GH::GHOST::BEHAV::current == "haunt"){
                    std::uniform_int_distribution<> emf_dist(1,4);
                    if(start_haunt){
                        haunt_timer.restart();
                        start_haunt = false;
                        std::uniform_int_distribution<> dist(2,10);
                        haunt_length = dist(gen);
                    }

                    if(haunt_timer.getElapsedTime().asSeconds() <= haunt_length){
                        GH::GHOST::BEHAV::flicker(ghost.sprite);

                        if(ghost.ghost_type == "Preta" && player_sprites.sanity > 50){
                            ghost.sprite.setFillColor(sf::Color(85,255,230,0));
                        }
                        else if(ghost.ghost_type == "Preta"){
                            ghost.sprite.setFillColor(sf::Color(85,255,230,175));
                        }
                        
                        if(emf_change.getElapsedTime().asSeconds() >= 0.5){
                            emf_change.restart();
                            ghost.emf = emf_dist(gen);
                        }
                    }
                    else{
                        ghost.previous_event = "haunt";
                        ghost.sprite.setFillColor(sf::Color(85,255,230,0));
                        GH::GHOST::BEHAV::current = "NILL";
                        ghost.emf = 0;
                        if((ghost.evidence[0] == "EMF 5" || ghost.evidence[1] == "EMF 5" || ghost.evidence[2] == "EMF 5")){
                            ghost.emf = 5;
                            thread delay_emf(delayed_emf);
                            delay_emf.detach();
                        }
                        ghost.sprite.setFillColor(sf::Color(255,255,255,0));
                        start_haunt = true;
                    }
                }
                else if(GH::GHOST::BEHAV::current == "communicate"){
                    for(auto& equipment : GH::INV::loaded_items){
                        if(equipment.name == "Voice" && !equipment.in_inventory && equipment.shape.getGlobalBounds().intersects(ghost.interaction_radius.getGlobalBounds())){
                            if(ghost.evidence[0] == "Spirit box" || ghost.evidence[1] == "Spirit box" || ghost.evidence[2] == "Spirit box"){
                                audio.sounds["Whisper"].play();
                                break;
                            }
                        }
                    }
                    ghost.previous_event = "communicate";
                    GH::GHOST::BEHAV::current = "NILL";
                }
                else if(GH::GHOST::BEHAV::current == "trapped" && trapped.getElapsedTime().asSeconds() >= 10){
                    GH::GHOST::BEHAV::current = "NILL";
                    for(auto& equipment : GH::INV::loaded_items){
                        if(equipment.name == "Trapped Mirror"){
                            equipment.name = "Broken Mirror";
                            equipment.shape.setTextureRect(GH::INV::texture_rects[13]);
                            audio.sounds["Whisper"].stop();
                            ghost.sprite.setPosition(equipment.shape.getPosition());
                            std::uniform_int_distribution<> angry(1,2);
                            if(angry(gen) == 1){
                                audio.sounds["glass_bad"].play();
                                if(player_sprites.sanity > 50){
                                    player_sprites.sanity = 50;
                                }
                                ghost.sprite.setPosition(equipment.shape.getPosition());
                                GH::GHOST::BEHAV::current = "hunt";
                            }
                            else{
                                audio.sounds["glass_good"].play();
                            }
                            break;
                        }
                    }
                }
                else if(GH::GHOST::BEHAV::current == "evidence"){
                    std::uniform_int_distribution<> chosen_evidence(0,2);
                    int chosen = chosen_evidence(gen);

                    bool blow = false;
                    if(ghost.evidence[0] == "Lantern blown" || ghost.evidence[1] == "Lantern blown" || ghost.evidence[2] == "Lantern blown"){
                        blow = true;
                    }
                    for(auto& equipment : GH::INV::loaded_items){
                        if(equipment.name == "Lantern" && equipment.shape.getGlobalBounds().intersects(ghost.interaction_radius.getGlobalBounds())){
                            GH::INV::blow_lantern(equipment, blow, ghost.ghost_type);
                        }
                        break;
                    }
                    std::cout<<ghost.evidence[chosen]<<std::endl;
                    if(ghost.evidence[chosen] == "Cold"){
                        player_sprites.cold.setFillColor(sf::Color(255,255,255,254));
                        player_sprites.cold.setPosition(player_sprites.body.getPosition());
                    }
                    else if(ghost.evidence[chosen] == "Bell"){
                        for(auto& equipment : GH::INV::loaded_items){
                            if(equipment.name == "Bell" && equipment.shape.getGlobalBounds().intersects(ghost.interaction_radius.getGlobalBounds())){
                                audio.sounds["Bell"].play();
                            }
                            break;
                        }                       
                    }
                    ghost.previous_event = "evidence";
                    GH::GHOST::BEHAV::current = "NILL";
                }       
                bool outside = true;
                for(auto& zone : map_data.ambient_zones){
                    if(zone.second.shape.getGlobalBounds().intersects(player_sprites.body.getGlobalBounds())){
                        outside = false;
                        break;
                    }
                }
                if(outside){
                    GH::GHOST::BEHAV::current == "NILL";
                    ghost.sprite.setFillColor(sf::Color(85,255,230,0));
                }   
                else if(GH::GHOST::BEHAV::current == "hunt" && !ghost.override_kill && !outside){  //When the ghost hunts it chases the player down
                    std::cout<<map_data.room_zones[ghost.room_index].room_protection<<std::endl;
                    if(map_data.room_zones[ghost.room_index].room_protection > 0){
                        map_data.room_zones[ghost.room_index].room_protection = GH::SPELLS::protection(-1, map_data.room_zones[ghost.room_index].room_protection, -1);
                        map_data.room_zones[ghost.room_index].shape.setFillColor(sf::Color(255,0,255));
                        GH::GHOST::BEHAV::current = "NILL";
                    }

                    for(auto& light_system : map_data.light_systems){
                        for(auto& light : light_system.second.lights){
                            if(light.getGlobalBounds().intersects(ghost.interaction_radius.getGlobalBounds()) && (light_system.second.flicker_time != 1 || light_system.second.lights_on)){
                                light_system.second.flicker_lights(gen);
                            }
                            else if(light_system.second.flicker_time != -1){
                                light_system.second.flicker_time = -1;
                                light_system.second.lights_on = false;
                            }
                        }
                    }

                    ghost.sprite.setFillColor(sf::Color(85,255,230,175));

                    if(ghost.ghost_type == "Preta" && player_sprites.sanity > 50){
                        ghost.sprite.setFillColor(sf::Color(85,255,230,0));
                    }
                    else if(ghost.ghost_type == "Preta"){
                        ghost.sprite.setFillColor(sf::Color(85,255,230,175));
                    }

                    GH::GHOST::BEHAV::flicker(ghost.sprite);
                    if(ghost.seen_player){
                        ghost.facing = GH::GHOST::BEHAV::chase(ghost.sprite, player_sprites.body, delta_time);
                    }
                    else if(!ghost.seen_player){
                        ghost.facing = GH::GHOST::BEHAV::wander(ghost.speed, ghost.sprite, ghost.walk_path, *ghost.chosen_room, delta_time, map_data.floors, true);
                    }
                    if(ghost.vision_cone.getBounds().intersects(player_sprites.body.getGlobalBounds())){
                        sf::Vector2f cardinal_direction = GH::MATH::cardinal_direction(player_sprites.body.getPosition(), ghost.sprite.getPosition());
                        sf::Vector2f current_pos = ghost.sprite.getPosition();
                        bool wall_there = false;
                        while(std::sqrt((player_sprites.body.getPosition().x - current_pos.x) * (player_sprites.body.getPosition().x - current_pos.x) + (player_sprites.body.getPosition().y - current_pos.y) * (player_sprites.body.getPosition().y - current_pos.y)) > 10){
                            for(auto& wall : map_data.walls){
                                if(wall.second.shape.getGlobalBounds().contains(current_pos)){
                                    wall_there = true;
                                    break;
                                }
                            }
                            if(wall_there){
                                break;
                            }
                            else{
                                current_pos += cardinal_direction;
                            }
                        }
                        if(!wall_there){
                            ghost.seen_player = true;
                        }
                    }
                    
                    if(player_sprites.body.getGlobalBounds().intersects(ghost.sprite.getGlobalBounds())){
                        if(ghost.ghost_type == "Doven"){
                            GH::GHOST::BEHAV::current = "NILL";
                            ghost.sprite.setFillColor(sf::Color(85,255,230,0));
                        }
                        else{
                            player_sprites.dead = true;
                        }
                    }
                }
                
            }
            ghost.align();
            GH::INV::align_slots(window, view);
            GH::SPELLS::align_slots(window, view, GH::START::slots);

            if(ghost.sprite.getFillColor() != sf::Color(85,255,230,0) && (GH::GHOST::BEHAV::current != "haunt" || GH::GHOST::BEHAV::current != "hunt")){
                ghost.sprite.setFillColor(sf::Color(85,255,230,0));
            }

            //figure out sanity drain
            if(sanity_clock.getElapsedTime().asSeconds() >= 1 && player_sprites.sanity > 0 && !create_mode){
                sanity_clock.restart();
                for(auto& floor : map_data.floors){
                    if(floor.second.shape.getGlobalBounds().intersects(player_sprites.body.getGlobalBounds())){
                        float drain = 0.12;
                        if(!flash_on){drain *= 2;}
                        if(greeny_fog == 50){drain *= 1.5;}
                        if(ghost.ghost_type == "Oni"){drain *= 2;}
                        if(GH::GHOST::BEHAV::current == "haunt" && player_sprites.body.getGlobalBounds().intersects(ghost.interaction_radius.getGlobalBounds())){drain*= 1.8;}
                        player_sprites.sanity -= drain;
                        break;
                    }
                }
            }
            

            sf::Vector2f diff = {abs(ghost.gate.getPosition().x - player_sprites.body.getPosition().x), abs(ghost.gate.getPosition().y - player_sprites.body.getPosition().y)};
            float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y);
            distance = 255 - distance;
            if(distance < 0){
                distance = 0;
            }
            else if(distance > 255){
                distance = 255;
            }
            ghost.gate.setFillColor(sf::Color(255,255,255,distance));
            if(greeny_fog < 50){
                ghost.gate.setFillColor(sf::Color(255,255,255,0));
            }
            
            //Handles anything to do with the player using the camera
            if(GH::INV::in_slots[GH::INV::selected_slot] == "Camera" && !player_sprites.dead){
                if(!take_pic && player_sprites.camera_outline[0].getFillColor().a == 0){
                    for(auto& corner : player_sprites.camera_outline){  //Show the outline that the camera is being used
                        corner.setFillColor(sf::Color(255,255,255,255));
                    }
                }
                if(((!held[sf::Keyboard::F10] && sf::Mouse::isButtonPressed(sf::Mouse::Left)) || take_pic) && !GH::JOURNAL::journal){  //When left click is pressed save the current window as a png
                    held[sf::Keyboard::F10] = true;
                    if(!take_pic){  //The screenshot is delayed by a frame so the camera outline can be removed from the photo
                        for(auto& corner : player_sprites.camera_outline){
                            corner.setFillColor(sf::Color(255,255,255,0));
                        }
                        for(auto& slot : GH::INV::slots){
                            slot.setFillColor(sf::Color(255,255,255,0));
                        }
                        take_pic = true;

                        if(ghost.evidence[0] == "Ethereal residue" || ghost.evidence[1] == "Ethereal residue" || ghost.evidence[2] == "Ethereal residue" && GH::MATH::rect_in_view(ghost.ethereal_residue.getGlobalBounds(), view)){
                            ghost.ethereal_residue.setFillColor(sf::Color(255,0,255,255));
                        }

                        if(GH::GHOST::BEHAV::current == "hunt" && ghost.ghost_type == "Phantom" && GH::MATH::rect_in_view(ghost.sprite.getGlobalBounds(), view)){
                            ghost.sprite.setFillColor(sf::Color(ghost.sprite.getFillColor().r, ghost.sprite.getFillColor().g, ghost.sprite.getFillColor().b,0));
                            phantom_clock.restart();
                            ghost.override_kill = true;
                        }
                    }
                    else{
                        sf::Texture screenshot;

                        if(GH::GHOST::BEHAV::current == "haunt" ||(GH::GHOST::BEHAV::current == "interact" && GH::MATH::rect_in_view(map_data.objects[map_data.priority_object].shape.getGlobalBounds(), view)) && !player_sprites.pics_taken["Ghost"]){
                            in_pic = "Ghost";
                            ghost.sprite.setFillColor(sf::Color(85,255,230,175));
                            player_sprites.pics_taken["Ghost"] = true;
                        }
                        else if(ghost.ethereal_residue.getFillColor().a > 0 && GH::MATH::rect_in_view(ghost.ethereal_residue.getGlobalBounds(), view) && !player_sprites.pics_taken["Ethereal residue"]){
                            in_pic = "Ethereal residue";
                            player_sprites.pics_taken["Ethereal residue"] = true;
                        }
                        else if(object_last_thrown.getElapsedTime().asSeconds() <= 5 && GH::MATH::rect_in_view(map_data.objects[map_data.priority_object].shape.getGlobalBounds(), view) && !player_sprites.pics_taken["Object thrown"]){
                            in_pic = "Object thrown";
                            player_sprites.pics_taken["Object thrown"] = true;
                        }
                        std::cout<<in_pic<<std::endl;

                        screenshot.create(window.getSize().x, window.getSize().y);
                        screenshot.update(window);

                        std::thread save_screenshot([screenshot, &take_pic, in_pic](){  //Saving the screenshot is threaded as it causes a short window freeze
                            if(GH::JOURNAL::image_empty[0]){
                                screenshot.copyToImage().saveToFile(MISC_DIR"/screenshots/pic0.png");
                                GH::JOURNAL::store_image(0, in_pic);
                            }
                            else if(GH::JOURNAL::image_empty[1]){
                                screenshot.copyToImage().saveToFile(MISC_DIR"/screenshots/pic1.png");
                                GH::JOURNAL::store_image(1, in_pic);
                            }
                            else if(GH::JOURNAL::image_empty[2]){
                                screenshot.copyToImage().saveToFile(MISC_DIR"/screenshots/pic2.png");
                                GH::JOURNAL::store_image(2, in_pic);
                            }
                            else if(GH::JOURNAL::image_empty[3]){
                                screenshot.copyToImage().saveToFile(MISC_DIR"/screenshots/pic3.png");
                                GH::JOURNAL::store_image(3, in_pic);
                            }
                   
                            for(auto& corner : player_sprites.camera_outline){
                                corner.setFillColor(sf::Color(255,255,255,255));
                            }
                            for(auto& slot : GH::INV::slots){
                                slot.setFillColor(sf::Color(255,255,255,255));
                            }
                        });
                        save_screenshot.detach();
                        in_pic = "NILL";
                        take_pic = false;
                    }
                }
            }
            else if(player_sprites.camera_outline[0].getFillColor().a == 255){  //Hide the camera outline
                for(auto& corner : player_sprites.camera_outline){
                    corner.setFillColor(sf::Color(255,255,255,0));
                }
            }
            
            if(ghost.override_kill && phantom_clock.getElapsedTime().asSeconds() >= 10){ //Disable phantoms ability caused by camera
                ghost.sprite.setFillColor(sf::Color(ghost.sprite.getFillColor().r, ghost.sprite.getFillColor().g, ghost.sprite.getFillColor().b,175));
                ghost.override_kill = false;
            }
        }
        if(ghost.ethereal_residue.getFillColor().a > 0){
            ghost.ethereal_residue.setFillColor(sf::Color(255,0,255,ghost.ethereal_residue.getFillColor().a-1));
        }

        if(GH::INV::locked_on){ //Load in the aniamtion for giving the floating object some bedazzle
            textures.FIREBAWL.setFillColor(sf::Color(255,255,255,200));
            textures.FIREBAWL.setPosition(GH::INV::locked_item->getPosition());
        }
        else if(GH::SPELLS::controlled_tp){
            textures.FIREBAWL.setFillColor(sf::Color(255,255,255,200));
            textures.FIREBAWL.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
            if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !held[sf::Keyboard::F10] && GH::SPELLS::tp_left != 0){
                held[sf::Keyboard::F10] = true;
                GH::SPELLS::tp_left -= 1;
                player_sprites.body.setPosition(window.mapPixelToCoords(sf::Mouse::getPosition(window)));
                if(GH::SPELLS::tp_left == 0){
                    GH::SPELLS::controlled_tp = false;
                }
            }
        }
        if(textures.FIREBAWL.getFillColor().a > 0){
            textures.FIREBAWL.setFillColor(sf::Color(255,255,255,textures.FIREBAWL.getFillColor().a-10)); 
        }
        

        if(player_sprites.dead){ //do the death animation and reset the game
            if(!death_anim){
                player_sprites.body.setFillColor(sf::Color(255,player_sprites.body.getFillColor().g-5,player_sprites.body.getFillColor().b-5));
            }
            else if(death_anim){
                player_sprites.body.setFillColor(sf::Color(255,0,0,player_sprites.body.getFillColor().a - 5));
            }
            if(player_sprites.body.getFillColor() == sf::Color(255,0,0)){
                death_anim = true;
            }
            else if(player_sprites.body.getFillColor() == sf::Color(255,0,0,0)){
                death_anim = false;
                map_data.id_floors = 0;
                map_data.id_walls = 0;
                map_data.id_objects = 0;
                map_data.id_rooms = 0;
                map_data.floors = {};
                map_data.walls = {};
                map_data.objects = {};
                map_data.room_zones = {};

                player_sprites.sanity = 100;
                player_sprites.stamina = 100;
                player_sprites.dead = false;
                player_sprites.pics_taken = {{"Ethereal residue",false},{"Ghost",false},{"Object thrown",false}};
                flash_on = true;
                create_mode = false;
                object_selected = false;
                vertical_wall = false;
                is_anchored = false;
                is_recording = false;
                debug_window = false;
                debug_fog = true;
                start_haunt = true;
                changing_spectral_mode = false;
                entering_spectral = true;
                take_pic = false;
                mouse_on_widget = false;
                map_data.chest_empty = false;
                map_data.next_item = true;
                map_data.animate_chest = false;
                greeny_fog = 0;

                player_sprites.body.setPosition(100,500);
                player_sprites.body.setFillColor(sf::Color(255,255,255,255));
                ghost.sprite.setFillColor(sf::Color(85,255,230,0));
                window.setView(window.getDefaultView());
                GH::GHOST::BEHAV::current = "NILL";

                GH::INV::reset();
                GH::JOURNAL::reset(true);

                display_game = false;
            }

        }
        if(!display_game){

            if(GH::JOURNAL::end){
                ImGui::Begin("Summary");
                ImGui::Text(("Ghost type: " + ghost.ghost_type).c_str());
                if(GH::JOURNAL::correct_ghost(ghost.ghost_type)){
                    ImGui::Text("Correct Ghost Type: +20xp");
                }
                else{
                    ImGui::Text("Correct Ghost Type: X");
                }

                if(ImGui::Button("Submit")){
                    GH::JOURNAL::end = false;
                    GH::JOURNAL::reset(true);
                    player_sprites.pics_taken = {{"Ethereal residue",false},{"Ghost",false},{"Object thrown",false}};
                }
                ImGui::End();
            }
            else{
                user_info = GH::START::log_in();
                //handling any interactions with the start menu
                if(user_info.second != -1){
                    GH::START::run_map_menu();
                    unordered_map<string, variant<int, sf::Color, string>> info;

                    map_data.map_chosen = GH::START::start_window();

                    if(map_data.map_chosen != -1){
                        debug_window = false;
                        debug_fog = false;
                        if(GH::START::go_create){
                            create_mode = true;
                        }
                        GH::SPELLS::set_slot_txt(GH::START::slots);
                        load_map();
                    }

                    GH::START::spells();

                    //handling any interactions with the avatar menu
                    info = GH::START::run_avatar_menu(GH::START::current_skintone);
                    if(get<string>(info["Info"]) == "Change skin"){
                        GH::TEXTURES::PLAYER::change_skin();
                        textures.update_texture_player();
                        GH::TEXTURES::PLAYER::save_avatar();
                    }
                    if(get<string>(info["Info"]) == "Hair-up"){
                        GH::TEXTURES::PLAYER::cycle_hair(1);
                        textures.update_texture_player();
                        GH::TEXTURES::PLAYER::save_avatar();
                    }

                    else if(get<string>(info["Info"]) == "Hair-down"){
                        GH::TEXTURES::PLAYER::cycle_hair(-1);
                        textures.update_texture_player();
                        GH::TEXTURES::PLAYER::save_avatar();
                    }

                    else if(get<string>(info["Info"]) == "Outfit-up"){
                        GH::TEXTURES::PLAYER::cycle_outfit(1);
                        textures.update_texture_player();
                        GH::TEXTURES::PLAYER::save_avatar();
                    }

                    else if(get<string>(info["Info"]) == "Outfit-down"){
                        GH::TEXTURES::PLAYER::cycle_outfit(-1);
                        textures.update_texture_player();
                        GH::TEXTURES::PLAYER::save_avatar();
                    }


                    else if(get<string>(info["Info"]) == "Accessories-up"){
                        GH::TEXTURES::PLAYER::cycle_accessories(1);
                        textures.update_texture_player();
                        GH::TEXTURES::PLAYER::save_avatar();
                    }

                    else if(get<string>(info["Info"]) == "Accessories-down"){
                        GH::TEXTURES::PLAYER::cycle_accessories(-1);
                        textures.update_texture_player();
                        GH::TEXTURES::PLAYER::save_avatar();
                    }

                    //Handle the players choice of equipment
                    GH::START::run_equip_menu();
                }
            }
            

        }
        
        
        fog.clear();//clear fog
        if(flash_on){
            fog.draw(light);//add flashlight to fog
        }
        for(auto& system : map_data.light_systems){
            if(system.second.lights_on){
                for(auto& light : system.second.lights){
                    fog.draw(light);
                }
            }
        }
        GH::INV::align_lights(fog);
        fog.display();
     
        window.clear();//clear window
        
        if(display_game){
            window.draw(map_data.grass);
        }
        map_data.draw(window);
        for(auto& system : map_data.light_systems){
            for(auto& swich : system.second.light_switches){
                window.draw(swich);
            }
        }
        for(auto& room : map_data.room_zones){
            if(room.second.shape.getFillColor() != sf::Color(0,255,0,32)){
                sf::Color c = room.second.shape.getFillColor();
                window.draw(room.second.shape);
                room.second.shape.setFillColor(sf::Color(c.r - 1, c.g + 1, c.b - 1, 32));
            }
        }
        
        
        window.draw(ghost.gate);
        
        if(display_game){
            window.draw(map_data.chest); //Draw the equipment chest
        }
        
        window.draw(textures.FIREBAWL);
        for(auto item : GH::INV::loaded_items){
            if(!item.in_inventory){
                window.draw(item.shape);
            }
        }
        if(display_game && !player_sprites.dead){
            window.draw(ghost.sprite);
            // window.draw(ghost.vision_cone);
            // window.draw(ghost.interaction_radius);
            // for(int i = 0 ; i < 5 ; i++){
            //     window.draw(ghost.walk_path[i]);
            // }
        }
        
        if(user_info.second != -1){
            player_sprites.draw(window, textures.player_new_facing);//draw the player

            if(player_sprites.cold.getFillColor().a > 0){
                player_sprites.cold.setFillColor(sf::Color(255,255,255,player_sprites.cold.getFillColor().a-2));
                player_sprites.cold.setRotation(player_sprites.cold.getRotation()+2);
                window.draw(player_sprites.cold);
            }
        }
        window.draw(ghost.ethereal_residue);
        if(create_mode){
            window.draw(mouse_hover);
        }
        if(GH::INV::check_uv(ghost.UV)){
            window.draw(ghost.UV);
        }
        
        for(auto& light_source : GH::INV::loaded_items){
            if(light_source.has_light && !light_source.in_inventory){
                light_source.light_filter.setPosition(light_source.shape.getPosition());
                window.draw(light_source.light_filter);
            }
        }

        if(!debug_fog){window.draw(fog);}  //draw the fog

        window.draw(ghost.gate);
        for(auto& corner : player_sprites.camera_outline){
            window.draw(corner);
        }

        if(frame_count%map_data.rain_intensity == 0){
            map_data.rain_index = (map_data.rain_index+1)%4;
            map_data.rain.setTexture(&map_data.rain_txt[map_data.rain_index]);
        }
        
        
        if(display_game && map_data.raining){
            int outside = true;
            for(auto& zone : map_data.ambient_zones){
                if(player_sprites.body.getGlobalBounds().intersects(zone.second.shape.getGlobalBounds())){
                    outside = false;
                    audio.sounds["rain"].setVolume(5);
                    break;
                }
            }
            if(outside && !map_data.unusable_map){
                window.draw(map_data.rain);
                if(audio.sounds["rain"].getStatus() != sf::Sound::Playing){
                    audio.sounds["rain"].play();
                }
                if(audio.sounds["rain"].getVolume() != 80){
                    audio.sounds["rain"].setVolume(80);
                }
            }
        }

        for(int i = 0 ; i < 3 ; i++){
            window.draw(GH::INV::slots[i]);
            window.draw(GH::SPELLS::slots[i]);
            window.draw(GH::SPELLS::slots_overlay[i]);
        }
        ImGui::SFML::Render(window);
        
        gui.draw();
        window.display();//display the window
        frame_count += 1;
        if(frame_count == 121){
            frame_count == 0;
        }
    }
    ImGui::SFML::Shutdown();
    return 0;
}

