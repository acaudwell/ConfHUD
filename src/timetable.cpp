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

#include "timetable.h"

// room|timestamp|description
Regex timetable_entry_regex("^([0-9]{0,10})\\|([^|]*)\\|(.*)$");

//TimetableEntry

TimetableEntry::TimetableEntry(std::string room, std::string description, time_t start_time) {
    this->room        = room;
    this->description = description;
    this->start_time  = start_time;

    //make display time
    if(start_time != 0) {
        char datestr[256];
        struct tm* timeinfo = localtime ( &start_time );
        strftime(datestr, 256, "%H%M", timeinfo);
        display_time = std::string(datestr);
    }

    font = fontmanager.grab("FreeSans.ttf", 28);
    font.shadowStrength(0.4);
    font.dropShadow(true);
    font.roundCoordinates(true);
}

void TimetableEntry::draw(float dt, float table_alpha) {

    float pos1 = 20;
    float pos2 = display.width * 0.1;
    float pos3 = display.width * 0.25;

    glColor4f(0.17, 0.47, 0.76, table_alpha);

    font.print(20,   0, "%s", room.c_str());

    glColor4f(0.74, 0.57, 0.12, table_alpha);

    font.print(pos2, 0, "%s", display_time.c_str());

    glColor4f(0.17, 0.47, 0.76, table_alpha);

    font.print(pos3, 0, "%s", description.c_str());
}

//Timetable

Timetable::Timetable(std::string timetablefile) {

    font = fontmanager.grab("FreeSans.ttf", 32);
    font.shadowStrength(0.4);
    font.dropShadow(false);
    font.roundCoordinates(true);

    alpha = 0.0;
    fadeout = false;

    entry_height = 35;

    last_modified_time = 0;

    loadTimetable(timetablefile);
}

bool Timetable::loadTimetable(std::string timetablefile) {
    this->timetablefile = timetablefile;

    deleteEntries();

    char buff[1024];

    //store file modified time
    struct stat fileinfo;
    int rc = stat(timetablefile.c_str(), &fileinfo);

    if(rc !=0) return false;

    last_modified_time = fileinfo.st_mtime;

    std::ifstream in(timetablefile.c_str());
    if(!in.is_open()) return false;

    in.getline(buff, 1024);

    title = std::string(buff);

    while(in.getline(buff, 1024)) {
        std::string line = std::string(buff);
        std::vector<std::string> matches;

        if(timetable_entry_regex.match(line, &matches)) {
            debugLog("line %s\n", line.c_str());
            time_t timestamp = atoi(matches[0].c_str());
            addEntry(matches[1], matches[2], timestamp);
        }

    }

    in.close();

    return true;
}

Timetable::~Timetable() {
    deleteEntries();
}

void Timetable::deleteEntries() {
    for(std::vector<TimetableEntry*>::iterator it = entries.begin(); it != entries.end(); it++) {
        TimetableEntry* entry = *it;
        delete entry;
    }
    entries.clear();
}


bool conf_timetable_sort(const TimetableEntry* a, const TimetableEntry* b) {
  return (a->start_time < b->start_time || a->room.compare(b->room) > 1);
}

void Timetable::addEntry(std::string room, std::string description, time_t start_time) {
    TimetableEntry* entry = new TimetableEntry(room, description, start_time);

    //add booking
    entries.push_back(entry);
}

int Timetable::getHeight() {
    return entry_height * entries.size();
}

int Timetable::getEntryHeight() {
    return entry_height;
}

int Timetable::getEntryCount() {
    return entries.size();
}

void Timetable::refresh() {

    //if file modified, reload file

    struct stat fileinfo;
    int rc = stat(timetablefile.c_str(), &fileinfo);

    if(rc !=0) return;

    if(last_modified_time < fileinfo.st_mtime) {
        debugLog("Timetable %s appears to have been modified. Reloading.\n", timetablefile.c_str());
        loadTimetable(timetablefile);
    }
}

void Timetable::fadeOut() {
    fadeout=true;
}
void Timetable::fadeIn() {
    fadeout=false;
}

float Timetable::getAlpha() {
    return alpha;
}

bool Timetable::isVisible() {
    return alpha > 0.0;
}

void Timetable::logic(float dt) {

    if(fadeout) {
        if(alpha>0.0) alpha = std::max(0.0f, alpha - dt);
    } else {
        if(alpha<1.0) alpha = std::min(1.0f, alpha + dt);
    }
}

void Timetable::draw(float dt) {

    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0, 1.0, 1.0, std::min(1.0f, alpha * 2.0f));

    float title_pos = display.width * 0.25;

    //draw title
    font.draw(title_pos, -10.0, title);

    glPushMatrix();

        glTranslatef(0.0, entry_height, 0.0);

        //draw bookings
        for(std::vector<TimetableEntry*>::iterator it = entries.begin(); it != entries.end(); it++) {
            TimetableEntry* entry = *it;
            entry->draw(dt, alpha);
            glTranslatef(0.0, entry_height, 0.0);
        }

    glPopMatrix();
}

