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

#ifndef CONFHUD_TIMETABLE_H
#define CONFHUD_TIMETABLE_H

#include "core/display.h"
#include "core/fxfont.h"
#include "core/regex.h"

#include <fstream>

#include <vector>
#include <algorithm>


class TimetableEntry {
public:
    std::string room;
    std::string description;
    std::string display_time;

    FXFont font, font2;

    TimetableEntry(std::string room, std::string description, std::string display_time);

    void draw(float dt, float table_alpha);
};

class Timetable {
protected:


    time_t last_modified_time;

    std::string title;

    std::string location_label, time_label;

    int entry_height;

    float alpha;
    bool fadeout;

    FXFont font;
    std::vector<TimetableEntry*> entries;

    std::string timetablefile;

    void deleteEntries();
    void addEntry(std::string room, std::string description, std::string display_time);
public:
    Timetable(std::string title, std::string timetablefile);
    ~Timetable();

    void setLocationLabel(std::string label);
    void setTimeLabel(std::string label);

    void fadeIn();
    void fadeOut();

    float getAlpha();

    bool isVisible();

    bool loadTimetable(std::string timetablefile);

    int getEntryHeight();
    int getEntryCount();
    int getHeight();

    void refresh();

    void logic(float dt);
    void draw(float dt);
};

extern vec3f gConfHUDColourDescription;
extern vec3f gConfHUDColourTitle;
extern vec3f gConfHUDColourTime;
extern vec3f gConfHUDColourMessage;
extern vec4f gConfHUDColourVisor;

#endif
