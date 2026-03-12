#include <SFML/Graphics.hpp>
#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>
#include<vector>
#include <map>
#include <fstream>
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/ostreamwrapper.h"
#include "rapidjson/error/en.h"
#include "journal.hpp"
#include <random>

/*This files has anything to do with the journal, the journal is used to:
-end the game
-select the correct ghost for the game
-taken photos
-information
*/

namespace GH{
namespace JOURNAL{

    tgui::Gui* gui_ptr; //Pointer to the gui in the main file

    tgui::Font font(ASSETS_DIR"/textures/fonts/Silkscreen-Bold.ttf"); //main font used for all text in the journal

    tgui::Group::Ptr ghost_selecting = tgui::Group::create();  //Holds the tri-state and labels for the evidence page
    tgui::Group::Ptr help = tgui::Group::create();  //Holds text for the help page
    tgui::Group::Ptr photos = tgui::Group::create();  //Holds the picture widgets for photos
    tgui::Picture::Ptr backdrop = tgui::Picture::create();  //The main journal back 
    tgui::Group::Ptr dividers = tgui::Group::create();  //Divider for turning the back into 2 pages
    tgui::Group::Ptr navigators = tgui::Group::create();  //Hold the buttons for turning page

    tgui::Group::Ptr tier1 = tgui::Group::create();
    tgui::Group::Ptr tier2 = tgui::Group::create();
    tgui::Group::Ptr tier3 = tgui::Group::create();
    tgui::Group::Ptr* chosen_tier;
    
    std::vector<tgui::Picture::Ptr> images = {tgui::Picture::create(), tgui::Picture::create(), tgui::Picture::create(), tgui::Picture::create()}; //Stores the pictures for the photo page
    std::vector<sf::Texture> image_txt = {sf::Texture(), sf::Texture(), sf::Texture(), sf::Texture()};  //Stores the textures for the pictures
    std::vector<tgui::Label::Ptr> image_labels; //Stores the labels used below each picture
    std::vector<tgui::Button::Ptr> image_staple;

    bool journal = false;
    bool end = false;
    int page_index = 2;
    std::vector<std::string> pages = {"Help", "Photos", "Ghosts"};
    std::vector<bool> image_empty = {true, true, true, true};

    void check_ghosts();  //Must be defined earlier on so the tristate class can use it without any errors

    void end_game(){
        end = true;
    }

    class tristate{  //Custom made widget that is like a check box but with three states (yes, no, blank)
        public:
        int state;
        tgui::Button::Ptr box;
        tgui::Label::Ptr text;
        tgui::Theme theme;
        tgui::Font font;
        bool ghost;

        tristate(bool isghost, tgui::Vector2f pos, const std::string& txt): state(0), theme(ASSETS_DIR"/textures/GUI/Theme/theme_misc.txt"), 
        font(ASSETS_DIR"/textures/fonts/Silkscreen-Bold.ttf"){
            ghost = isghost;

            box = tgui::Button::create();
            box->setPosition(pos);
            box->setSize({48, 48});
            box->setRenderer(theme.getRenderer("tristate-none"));
            box->onPress([this]() {change_state((state + 1) % 3);});

            text = tgui::Label::create();
            text->setSize(200,100);
            text->setTextSize(24);
            text->setText(txt);
            text->setPosition({pos.x + 48, pos.y});

            text->setInheritedFont(font);
        }

        void change_state(int state_){
            state = state_;
            switch (state) {
                case 0:
                    box->setRenderer(theme.getRenderer("tristate-none"));
                    break;
                case 1:
                    box->setRenderer(theme.getRenderer("tristate-check"));
                    break;
                case 2:
                    box->setRenderer(theme.getRenderer("tristate-cross"));
                    break;
            }
            if(!ghost){
                check_ghosts();
            }
        }
    };

    std::map<std::string,std::shared_ptr<tristate>> tri_states;
    std::map<std::string,std::vector<std::string>> ghost_evidences;

    void check_ghosts(){
        std::vector<std::string> evidences = {"EMF 5", "Lantern blown", "Spirit box", "Silver mirror", "Bell", "Ethereal residue", "UV", "Cold"};
        for(auto& ghost : ghost_evidences){
            if(ghost.first == "Funnel"){
                continue;
            }
            bool eliminate = false;
            for(auto evidence : ghost.second){  //First check if one of the ghost's evidences is crossed off cause if so the ghost is eliminated without further checks
                if(tri_states[evidence]->state == 2){
                    tri_states[ghost.first]->change_state(2);
                    eliminate = true;
                }
            }
            if(!eliminate){  //If first check didnt eliminate the ghost then assume t could be that ghost until the next check
                tri_states[ghost.first]->change_state(0);
            }
            for(auto evidence : evidences){  //Final check to see if any evidences this ghost doesnt have is true
                if(evidence != ghost.second[0] && evidence != ghost.second[1] && evidence != ghost.second[2]){  //Make sure it only checks evidences the ghost doesnt have
                    if(tri_states[evidence]->state == 1){  //Checks if the state of that evidence is a tick, if so this ghost needs to be eliminated
                        tri_states[ghost.first]->change_state(2);
                        break;
                    }
                }    
            }
        }
    }


    tgui::Vector2f coords_to_journal(tgui::Vector2f coords, const tgui::Picture::Ptr& backdrop){//transform coordinates to a relative position on the journal
        tgui::Vector2f origin = backdrop->getOrigin();
        tgui::Vector2f size = backdrop->getSize();
        tgui::Vector2f scale = backdrop->getScale();
        tgui::Vector2f position = backdrop->getPosition();

        tgui::Vector2f scaledSize = {size.x * scale.x, size.y * scale.y};

        tgui::Vector2f topLeft = position - tgui::Vector2f(origin.x * scaledSize.x, origin.y * scaledSize.y);

        return topLeft + coords;
    }


    void change_tab(bool direction){//change page of the journal
        gui_ptr->removeAllWidgets();
        gui_ptr->add(backdrop);
        gui_ptr->add(dividers);
        gui_ptr->add(navigators);
        if(direction){
            page_index = (page_index + 1)%4;
        }
        else{
            page_index -= 1;
            if(page_index < 0){
                page_index = 0;
            }
        }
        if(pages[page_index] == "Help"){
            gui_ptr->add(help);
        }
        else if(pages[page_index] == "Photos"){
            gui_ptr->add(photos);
        }
        else if(pages[page_index] == "Ghosts"){
            gui_ptr->add(ghost_selecting);
        }
    }
    
    void change_tier(int tier){//change sub page of the ghost page, to see the different tiers of ghost
        gui_ptr->remove(ghost_selecting);
        ghost_selecting->remove(*chosen_tier);
        switch(tier){
            case 1: chosen_tier = &tier1; break;
            case 2: chosen_tier = &tier2; break;
            case 3: chosen_tier = &tier3; break;
        }
        ghost_selecting->add(*chosen_tier);
        gui_ptr->add(ghost_selecting);
    }

    void store_image(int index, std::string action){//store the image thats been saved in the journal, along with an action that has happend in the photo if one has occured
            if(index == 0){
                image_txt[0].loadFromFile(MISC_DIR"/screenshots/pic0.png");
            }
            else if(index == 1){
                image_txt[1].loadFromFile(MISC_DIR"/screenshots/pic1.png");
            }
            else if(index == 2){
                image_txt[2].loadFromFile(MISC_DIR"/screenshots/pic2.png");
            }
            else if(index == 3){
                image_txt[3].loadFromFile(MISC_DIR"/screenshots/pic3.png");
            }
            images[index]->getRenderer()->setTexture(image_txt[index]);
            image_empty[index] = false;
            if(action != "NILL"){
                image_labels[index]->setText(action);
            }
    }

    void align_straps(tgui::Picture::Ptr image){  //Creates 4 straps in each corner of the images
        tgui::Button::Ptr strap = tgui::Button::create();
        tgui::Theme theme(ASSETS_DIR"/textures/GUI/Theme/theme_misc.txt");
        strap->setSize({48,48});
        strap->setRenderer(theme.getRenderer("pic_strap"));
        strap->setPosition({image->getPosition().x-12, image->getPosition().y-12});
        image_staple.push_back(tgui::Button::copy(strap));
        strap->setPosition({image->getPosition().x+image->getSize().x+12, image->getPosition().y-12});
        strap->setRotation(90);
        image_staple.push_back(tgui::Button::copy(strap));
        strap->setRotation(180);
        strap->setPosition({image->getPosition().x+image->getSize().x+12, image->getPosition().y+image->getSize().y+12});
        image_staple.push_back(tgui::Button::copy(strap));
        strap->setRotation(270);
        strap->setPosition({image->getPosition().x-12, image->getPosition().y+image->getSize().y+12});
        image_staple.push_back(tgui::Button::copy(strap));
    }
  
    void setup(int W_W, int W_H, tgui::Gui& gui){//intial set up of the journal and all its widgets
        gui_ptr = &gui;
        tgui::Theme backdrop_theme{ASSETS_DIR"/textures/GUI/Theme/theme_backdrop.txt"};
        tgui::Theme button_theme(ASSETS_DIR"/textures/GUI/Theme/theme_buttons.txt");
        
        //Load the json containing all the ghosts' evidences
        std::ifstream infile("evidence.json");
        std::string content((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>());
        infile.close();
        
        //Load all the ghost evidences
        rapidjson::Document doc;
        doc.Parse(content.c_str());

        for(auto itr = doc.MemberBegin(); itr != doc.MemberEnd(); ++itr) {
            const std::string ghost_name = itr->name.GetString();
            const rapidjson::Value& evidences = itr->value;

            ghost_evidences[ghost_name] = {
                evidences["Evidence1"].GetString(),
                evidences["Evidence2"].GetString(),
                evidences["Evidence3"].GetString()
            };
        }
        //Set up the backdrop used for all the journal menus
        backdrop->setRenderer(backdrop_theme.getRenderer("menu_backdrop"));
        backdrop->setSize({256,128});
        backdrop->setScale({5.0f,5.0f});
        backdrop->setOrigin({0.5,0.5});
        backdrop->setPosition({W_W/2,W_H/2});

        for(int i = 0 ; i < 13 ; i++){
            tgui::Theme theme(ASSETS_DIR"/textures/GUI/Theme/theme_misc.txt");
            tgui::Picture::Ptr pic = tgui::Picture::create();
            pic->setRenderer(theme.getRenderer("journal_divider"));
            pic->setSize({15, 40});
            pic->setPosition({coords_to_journal({backdrop->getSize().x/2*5-20, 30+(i*45)}, backdrop)});
            dividers->add(pic);
        }
   

        //Set up the buttons for the ghost/evidence section of the journal
        std::vector<std::string> evidences = {"EMF 5", "Lantern blown", "Spirit box", "Silver mirror", "Bell", "Ethereal residue", "UV", "Cold"};
        int count = 0;
        for(int row = 0 ; row < 4 ; row++){
            for(int column = 0 ; column < 2 ; column++){
                auto temp = std::make_shared<tristate>(false, coords_to_journal({50+(column*300), 50+(row*100)}, backdrop) ,evidences[count]);
                tri_states[evidences[count]] = temp;
                ghost_selecting->add(tri_states[evidences[count]]->box);
                ghost_selecting->add(tri_states[evidences[count]]->text);
                count++;
            }
        }
        
        std::vector<std::string> tier1_ghosts = {"Spirit", "Ghoul", "Phantom", "Grey Lady", "Revenant", "Imp", "Shade", "Myling", "Doven", "Funnel"};
        std::vector<std::string> tier2_ghosts = {"Tsuk", "Preta", "Poltergeist", "Moroi", "Wisp", "Wraith", "Onryo", "Green Lady"};
        std::vector<std::string> tier3_ghosts = {"Dalgyal Guishin", "Demon", "Oni", "Red Lady"};

        count = 0;
        for(int row = 0 ; row < 5 ; row++){  //Adds tri-state boxes for then tier 1 ghosts
            for(int column = 0 ; column < 2 ; column++){
                auto temp = std::make_shared<tristate>(true, coords_to_journal({700+(column*300), 50+(row*100)}, backdrop), tier1_ghosts[count]);
                tri_states[tier1_ghosts[count]] = temp;
                tier1->add(tri_states[tier1_ghosts[count]]->box);
                tier1->add(tri_states[tier1_ghosts[count]]->text);
                count++;
            }
        }
        count = 0;
        for(int row = 0 ; row < 4 ; row++){  //Adds tri-state boxes for then tier 2 ghosts
            for(int column = 0 ; column < 2 ; column++){
                auto temp = std::make_shared<tristate>(true, coords_to_journal({700+(column*300), 50+(row*100)}, backdrop), tier2_ghosts[count]);
                tri_states[tier2_ghosts[count]] = temp;
                tier2->add(tri_states[tier2_ghosts[count]]->box);
                tier2->add(tri_states[tier2_ghosts[count]]->text);
                count++;
            }
        }
        count = 0; 
        for(int row = 0 ; row < 2 ; row++){  //Adds tri-state boxes for then tier 3 ghosts
            for(int column = 0 ; column < 2 ; column++){
                auto temp = std::make_shared<tristate>(true, coords_to_journal({700+(column*300), 50+(row*100)}, backdrop), tier3_ghosts[count]);
                tri_states[tier3_ghosts[count]] = temp;
                tier3->add(tri_states[tier3_ghosts[count]]->box);
                tier3->add(tri_states[tier3_ghosts[count]]->text);
                count++;
            }
        }
        
        //Button for showing tier 1 ghosts
        tgui::Button::Ptr change_tier1 = tgui::Button::create();
        change_tier1->setPosition(coords_to_journal({(backdrop->getSize().x/2*5), (backdrop->getSize().y*5)-80}, backdrop));
        change_tier1->setSize({48, 48});
        change_tier1->setRenderer(button_theme.getRenderer("Button_blank"));
        change_tier1->onClick([&]{
            change_tier(1);
        });
        ghost_selecting->add(change_tier1);

        //Button for showing tier 2 ghosts
        tgui::Button::Ptr change_tier2 = tgui::Button::create();
        change_tier2->setPosition(coords_to_journal({(backdrop->getSize().x/2*5) + 52, (backdrop->getSize().y*5)-80}, backdrop));
        change_tier2->setSize({48, 48});
        change_tier2->setRenderer(button_theme.getRenderer("Button_blank"));
        change_tier2->onClick([&]{
            change_tier(2);
        });
        ghost_selecting->add(change_tier2);
        
        //Button for showing tier 3 ghosts
        tgui::Button::Ptr change_tier3 = tgui::Button::create();
        change_tier3->setPosition(coords_to_journal({(backdrop->getSize().x/2*5) + 104, (backdrop->getSize().y*5)-80}, backdrop));
        change_tier3->setSize({48, 48});

        change_tier3->setRenderer(button_theme.getRenderer("Button_blank"));
        change_tier3->onClick([&]{
            change_tier(3);
        });
        ghost_selecting->add(change_tier3);

        tgui::Button::Ptr leave = tgui::Button::create();
        leave->setPosition(coords_to_journal({(backdrop->getSize().x/2*5) + 104, (backdrop->getSize().y*5)-60}, backdrop));
        leave->setSize({86,48});
        leave->onClick([&]{
            end = true;
        });
        ghost_selecting->add(leave);

        //Setup the slots for images taken by the camera
        images[0]->setPosition({coords_to_journal({96, 48}, backdrop)});  //Image 1
        images[0]->setSize({384,192});
        align_straps(images[0]);
        photos->add(images[0]);
       

        images[1]->setPosition({coords_to_journal({96, 336}, backdrop)});  //Image 2
        images[1]->setSize({384,192});
        align_straps(images[1]);
        photos->add(images[1]); 

        images[2]->setPosition({coords_to_journal({712, 48}, backdrop)});  //Image 3
        images[2]->setSize({384,192});
        align_straps(images[2]);
        photos->add(images[2]);

        images[3]->setPosition({coords_to_journal({712, 336}, backdrop)});  //Image 4
        images[3]->setSize({384,192});
        align_straps(images[3]);
        photos->add(images[3]);

        for(int i = 0 ; i < 16 ; i++){
            photos->add(image_staple[i]);
        }

        for(int i = 0 ; i < 4 ; i++){  //Create the label for each picture
            image_labels.push_back(tgui::Label::create());
            image_labels[i]->setPosition({images[i]->getPosition().x, images[i]->getPosition().y+images[i]->getSize().y+12});
            image_labels[i]->setSize(200,100);
            image_labels[i]->setTextSize(24);
            image_labels[i]->setInheritedFont(font);
            photos->add(image_labels[i]);
        }

        
        //Both of the below buttons are used for flipping through each menu in the journal
        tgui::Button::Ptr backwards = tgui::Button::create();
        backwards->setPosition(coords_to_journal({0,backdrop->getSize().y*5}, backdrop));
        backwards->setSize({96,96});
        backwards->setRenderer(button_theme.getRenderer("Back"));
        backwards->onClick([&]{
            change_tab(false);
        });
        navigators->add(backwards, "backwards");

        tgui::Button::Ptr forwards = tgui::Button::create();
        forwards->setPosition(coords_to_journal({backdrop->getSize().x*5 - 96, backdrop->getSize().y*5}, backdrop));
        forwards->setSize({96,96});
        forwards->setRenderer(button_theme.getRenderer("Forw"));
        forwards->onClick([&]{
            change_tab(true);
        });
        navigators->add(forwards, "forwards");
 

        chosen_tier = &tier1;
        ghost_selecting->add(*chosen_tier);
    }

    void toggle_journal(tgui::Gui& gui){//toggle the journal open and close
        if(journal){
            journal = false;
            gui.removeAllWidgets();
        }
        else{
            journal = true;
            gui.add(backdrop);
            gui.add(dividers);
            gui.add(navigators);
            if(pages[page_index] == "Help"){
                gui_ptr->add(help);
            }
            else if(pages[page_index] == "Photos"){
                gui_ptr->add(photos);
            }
            else if(pages[page_index] == "Ghosts"){
                gui_ptr->add(ghost_selecting);
            }
        }
    }

    void reset(bool full){//reset the journal when the game has ended
        journal = false;
        gui_ptr->removeAllWidgets();
        chosen_tier = &tier1;
        page_index = 2;
        image_empty = {true, true, true, true};
        for(auto& label : image_labels){
            label = tgui::Label::create();
        }
        
        if(full){
            image_txt = {sf::Texture(), sf::Texture(), sf::Texture(), sf::Texture()};
        
            for(int i = 0 ; i < 3 ; i++){
                images[i]->getRenderer()->setTexture(tgui::Texture());
            }

            for(auto& tri : tri_states){
                tri.second->change_state(0);
            }
        }
    }

    bool correct_ghost(std::string ghost){//is the selected ghost the right ghost
        for(auto& tri : tri_states){
            if(tri.second->state == 1 && tri.first == ghost){
                return true;
            }
        }
        return false;
    }
}
}