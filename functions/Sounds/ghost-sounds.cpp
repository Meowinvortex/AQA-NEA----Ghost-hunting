#include <SFML/Audio.hpp>
#include <ctime>

#include "ghost-sounds.hpp"

/*very small file just used for the ghost to play a random sound*/

namespace GH{
namespace SOUNDS{
namespace GHOST{
    
    int indexes[] = {1,2,3,12,15,18};
    sf::Music music[6];

    void set_up(){//initial set up
        for(int i = 0 ; i < 6 ; i++){
            music[i].openFromFile(ASSETS_DIR"/sounds/Magic and Spells 5 - Universal Sound Effects/MP3/Magic Spell " + std::to_string(i) + ".mp3");
        }
    }

    void play_sound(){//play random sound
        srand(time(0));

        int random_index = rand() % 6;
        
        music[random_index].play();
    }
}
}
}