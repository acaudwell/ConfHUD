/*
    Copyright (C) 2009 Andrew Caudwell (acaudwell@gmail.com)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef CONFHUD_H
#define CONFHUD_H

#define CONFHUD_VERSION "0.10"

#ifdef _WIN32
#include "windows.h"
#endif

#include <fstream>

#include <deque>
#include <dirent.h>

#include "core/display.h"
#include "core/sdlapp.h"
#include "core/fxfont.h"

#include "timetable.h"

void createWindowsConsole();

void confhud_info(std::string msg);
void confhud_help(std::string error);

class ConfHUD : public SDLApp {

protected:
    bool debug;

    float t, dt;

    TimetableViewer* timetable_viewer;

    TextureResource* footer;

    FXFont font, scrollfont;

    std::string scroll_message;
    float scroll_message_width;
    float scroll_message_x;

    void updateScrollMessage();

    void reset();
    void drawBackground();
public:
    ConfHUD();
    ~ConfHUD();

    void update(float t, float dt);
    void logic();
    void draw();

    void init();
    void keyPress(SDL_KeyboardEvent *e);
    void mouseMove(SDL_MouseMotionEvent *e);
    void mouseClick(SDL_MouseButtonEvent *e);
};

#endif
