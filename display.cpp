#include "display.h"
#include <ncurses/ncurses.h>
#include <signal.h>
#include <stdexcept>

namespace lambda {

void cleanup_exit(int arg) {
    endwin();
    exit(arg);
}

int display::init() {
    static bool did_init = false;
    if(!did_init) {
        did_init = true;
        signal(SIGINT, cleanup_exit);
    }
}

display::display() {
    display::init();
    
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, true);
    if(nodelay(stdscr, true) == ERR)
        cleanup_exit();

    choices_index = 0;
    focus = nullptr;
    draw_window();
}

int display::show() {

}

int display::hide() {

}

int display::update() {
    int ch;
    while((ch = getch()) != ERR) {
        switch(ch) {
        case '\t':
        case KEY_RIGHT:
            ++chioces_index;
            if(chioces_index >= choices.size())
                choices_index = 0;
        case KEY_LEFT:
            --choices_index;
            if(index < 0)
                index = choices.size() - 1;
        case KEY_ENTER:
            if(choices_index < chioces.size()) {
                choices[choices_index].pos->evaluate_step();
                choices = focus.find_steps();
                choices_index = 0;
            }        
        }
    }
    draw();
}


int display::draw() {
    if(focus.is_init()) {
        
    } else {

    }    
}

int display::~display() { 

}

}












