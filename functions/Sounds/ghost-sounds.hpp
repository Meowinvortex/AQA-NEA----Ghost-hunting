#ifndef GHOST_SOUNDS_H
#define GHOST_SOUNDS_H


#include <SFML/Audio.hpp>
#include <ctime>


namespace GH{
namespace SOUNDS{
namespace GHOST{

    extern int indexes[];

    extern sf::Music music[6];

    void set_up();

    void play_sound();
}
}
}

#endif