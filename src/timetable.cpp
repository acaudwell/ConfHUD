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
Regex timetable_entry_regex("^([^|]*)\\|([^|]*)\\|(.*)$");
Regex timetable_timestamp_regex("^[0-9]{0,10}$");

std::string timetable_timeformat("%H:%M");

vec3f gConfHUDColourDescription(1.0, 1.0, 1.0);
vec3f gConfHUDColourTitle(1.0, 1.0, 1.0);
vec3f gConfHUDColourTime(1.0, 1.0, 1.0);
vec3f gConfHUDColourMessage(1.0, 1.0, 1.0);
vec4f gConfHUDColourVisor(1.0, 1.0, 1.0, 0.0);

//TimetableEntry

TimetableEntry::TimetableEntry(std::string room, std::string description, std::string display_time) {
    this->room        = room;
    this->description = description;
    this->display_time  = display_time;

    //if display time appears to be a timestamp, process it
    if(timetable_timestamp_regex.match(display_time)) {
        char datestr[256];
        time_t display_timestamp = atof(display_time.c_str());

        struct tm* timeinfo = localtime ( &display_timestamp );

        strftime(datestr, 256, timetable_timeformat.c_str(), timeinfo);

        this->display_time = std::string(datestr);
    }

    font = fontmanager.grab("FreeSans.ttf", 28);
    font.shadowStrength(0.4);
    font.dropShadow(true);
    font.roundCoordinates(true);

    font2 = fontmanager.grab("FreeSans.ttf", 20);
    font2.shadowStrength(0.4);
    font2.dropShadow(true);
    font2.roundCoordinates(true);
}

void TimetableEntry::draw(float dt, float table_alpha) {

    float pos1 = 20;
    float pos2 = display.width * 0.25;
    float pos3 = display.width * 0.33;

    glColor4f(gConfHUDColourDescription.x, gConfHUDColourDescription.y, gConfHUDColourDescription.z, table_alpha);

    font.print(20,   0, "%s", room.c_str());

    glColor4f(gConfHUDColourTime.x, gConfHUDColourTime.y, gConfHUDColourTime.z, table_alpha);

    font.print(pos2, 0, "%s", display_time.c_str());

    glColor4f(gConfHUDColourDescription.x, gConfHUDColourDescription.y, gConfHUDColourDescription.z, table_alpha);

    int description_space = display.width - pos3;

    if(font.getWidth(description) < description_space) {
        font.print(pos3, 0, "%s", description.c_str());
    } else {
        font2.print(pos3, 0, "%s", description.c_str());
    }
}

//Timetable

Timetable::Timetable(std::string title, std::string timetablefile) {

    font = fontmanager.grab("FreeSans.ttf", 32);
    font.shadowStrength(0.4);
    font.dropShadow(true);
    font.roundCoordinates(true);

    this->title = title;

    alpha = 0.0;
    fadeout = false;

    entry_height = 48;

    last_modified_time = 0;

    location_label = std::string("Room");
    time_label     = std::string("Time");

    loadTimetable(timetablefile);
}

void Timetable::setLocationLabel(std::string label) {
    location_label = label;
}

void Timetable::setTimeLabel(std::string label) {
    time_label = label;
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

    while(in.getline(buff, 1024)) {
        std::string line = std::string(buff);
        std::vector<std::string> matches;

        if(timetable_entry_regex.match(line, &matches)) {
            addEntry(matches[1], matches[2], matches[0]);
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

void Timetable::addEntry(std::string room, std::string description, std::string display_time) {
    TimetableEntry* entry = new TimetableEntry(room, description, display_time);

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

bool Timetable::isFinished(float interval) {
    return (alpha <= -interval && fadeout);
}

void Timetable::logic(float dt) {

    if(fadeout) {
        alpha -= dt;
    } else {
        if(alpha<1.0) alpha = std::min(1.0f, alpha + dt);
    }
}

void Timetable::draw(float dt) {

    glEnable(GL_TEXTURE_2D);

    float current_alpha = std::max(0.0f, alpha);

    glColor4f(gConfHUDColourTitle.x, gConfHUDColourTitle.y, gConfHUDColourTitle.z, std::min(1.0f, current_alpha * 2.0f));

    float pos1 = 20;
    float pos2 = display.width * 0.25;
    float title_pos = display.width * 0.33;

    //draw entry
    font.draw(pos1,      0.0, location_label);
    font.draw(pos2,      0.0, time_label);
    font.draw(title_pos, 0.0, title);

    glPushMatrix();
        //glTranslatef(0.0, 10.0, 0.0);
        glTranslatef(0.0, entry_height, 0.0);

        //draw bookings
        for(std::vector<TimetableEntry*>::iterator it = entries.begin(); it != entries.end(); it++) {
            TimetableEntry* entry = *it;
            entry->draw(dt, current_alpha);
            glTranslatef(0.0, entry_height, 0.0);
        }

    glPopMatrix();
}

