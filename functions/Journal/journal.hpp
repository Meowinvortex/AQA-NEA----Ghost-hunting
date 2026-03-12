#ifndef JOURNAL_H
#define JOURNAL_H

#include <SFML/Graphics.hpp>
#include<TGUI/TGUI.hpp>
#include<TGUI/Backend/SFML-Graphics.hpp>


namespace GH{
namespace JOURNAL{
    extern std::vector<bool> image_empty;
    extern bool end;

    extern bool journal;

    void store_image(int index, std::string action);

    void setup(int W_W, int W_H, tgui::Gui& gui);

    void toggle_journal(tgui::Gui& gui);

    void reset(bool full);

    bool correct_ghost(std::string ghost);
 
}
}

#endif