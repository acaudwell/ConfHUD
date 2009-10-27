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
Regex timetable_entry_regex("^([0-9]+)\\|([^|]*)\\|(.*)$");

//TimetableEntry

TimetableEntry::TimetableEntry(std::string room, std::string description, time_t start_time) {
    this->room        = room;
    this->description = description;
    this->start_time  = start_time;

    //make display time
    char datestr[256];
    struct tm* timeinfo = localtime ( &start_time );
    strftime(datestr, 256, "%H%M", timeinfo);
    display_time = std::string(datestr);

    font = fontmanager.grab("FreeSans.ttf", 28);
    font.shadowStrength(0.4);
    font.dropShadow(true);
    font.roundCoordinates(true);
}

void TimetableEntry::draw(float dt) {

    float pos1 = 20;
    float pos2 = display.width * 0.1;
    float pos3 = display.width * 0.25;

    glColor3f(0.17, 0.47, 0.76);

    font.print(20,   0, "%s", room.c_str());

    glColor3f(0.74, 0.57, 0.12);

    font.print(pos2, 0, "%s", display_time.c_str());

    glColor3f(0.17, 0.47, 0.76);

    font.print(pos3, 0, "%s", description.c_str());
}

//Timetable

Timetable::Timetable(std::string timetablefile) {

    font = fontmanager.grab("FreeSans.ttf", 28);
    font.shadowStrength(0.4);
    font.dropShadow(true);
    font.roundCoordinates(true);

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
    std::string title = std::string(buff);

    while(in.getline(buff, 1024)) {
        std::string line = std::string(buff);
        std::vector<std::string> matches;
        if(timetable_entry_regex.match(line, &matches)) {
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

    //resort
    std::sort(entries.begin(), entries.end(), conf_timetable_sort);
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
        loadTimetable(timetablefile);
    }
}

void Timetable::logic(float dt) {

}

void Timetable::draw(float dt) {


    glEnable(GL_TEXTURE_2D);

    glPushMatrix();

        glTranslatef(0.0, entry_height, 0.0);

        //draw bookings
        for(std::vector<TimetableEntry*>::iterator it = entries.begin(); it != entries.end(); it++) {
            TimetableEntry* entry = *it;
            entry->draw(dt);
            glTranslatef(0.0, entry_height, 0.0);
        }

    glPopMatrix();
}

// TimetableViewer

TimetableViewer::TimetableViewer() {
    next_timetable = false;
    current_timetable = 0;

    font = fontmanager.grab("FreeSans.ttf", 28);
    font.shadowStrength(0.4);
    font.dropShadow(true);
    font.roundCoordinates(true);
}

TimetableViewer::~TimetableViewer() {
    for(std::vector<Timetable*>::iterator it = timetables.begin(); it != timetables.end(); it++) {
        Timetable* timetable = *it;
        delete timetable;
    }
    timetables.clear();
}

void TimetableViewer::nextTimetable() {
    if(timetables.size()==0) return;
    next_timetable = true;
}

void TimetableViewer::addTimetable(std::string timetablefile) {
    Timetable* timetable = new Timetable(timetablefile);
    timetables.push_back(timetable);
}

void TimetableViewer::logic(float dt) {

    if(timetables.size() == 0) return;

    Timetable* current = timetables[current_timetable];

    current->logic(dt);
}

void TimetableViewer::draw(float dt) {
     if(timetables.size() == 0) return;

    Timetable* current = timetables[current_timetable];

    if(current->getEntryCount()==0) return;

   //draw transparent background
    glDisable(GL_TEXTURE_2D);

    float entry_height = current->getEntryHeight();
    float timetable_height = current->getHeight() + 10.0;
    vec2f timetable_corner = vec2f(0.0, 130.0);

    glBegin(GL_QUADS);
        glColor4f(1.0, 1.0, 1.0, 0.4);

        glVertex2f(timetable_corner.x, timetable_corner.y);
        glVertex2f(display.width,      timetable_corner.y);

        glVertex2f(display.width,      timetable_corner.y + timetable_height);
        glVertex2f(timetable_corner.x, timetable_corner.y + timetable_height);
    glEnd();

    glEnable(GL_TEXTURE_2D);

    glPushMatrix();

        glTranslatef(timetable_corner.x, timetable_corner.y - entry_height, 0.0);

        float pos1 = 20;
        float pos2 = display.width * 0.1;
        float pos3 = display.width * 0.25;

        glColor3f(1.0, 1.0, 1.0);

        font.draw(pos1, 0.0, "Room");
        font.draw(pos2, 0.0, "Time");
        font.draw(pos3, 0.0, "Description");

        glTranslatef(0.0, 10.0, 0.0);

        current->draw(dt);

    glPopMatrix();
}
