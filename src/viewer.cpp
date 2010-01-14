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

#include "viewer.h"

// TimetableViewer

TimetableViewer::TimetableViewer() {
    this->duration = 15.0;

    font = fontmanager.grab("FreeSans.ttf", 32);
    font.shadowStrength(0.4);
    font.dropShadow(true);
    font.roundCoordinates(true);

    reset();
}

TimetableViewer::~TimetableViewer() {
    reset();
}

void TimetableViewer::reset() {
    next_timetable = false;
    current_timetable = -1;
    elapsed = -1.0;

    for(std::vector<Timetable*>::iterator it = timetables.begin(); it != timetables.end(); it++) {
        Timetable* timetable = *it;
        delete timetable;
    }

    timetables.clear();
}

void TimetableViewer::setDuration(float duration) {
    this->duration = duration;
}

Timetable* TimetableViewer::getCurrentTimetable() {
    if(current_timetable==-1 || current_timetable>=timetables.size()) return 0;

    return timetables[current_timetable];
}

void TimetableViewer::fadeOutCurrent() {
    Timetable* current = getCurrentTimetable();

    next_timetable = true;

    if(current!=0) current->fadeOut();
}

//get the next non blank timetable.. if they are all blank, sets current_timetable to -1
void TimetableViewer::switchToNextPopulatedTimetable() {
    debugLog("switchToNextPopulatedTimetable()\n");

    if(timetables.size()==0) return;

    int start = (current_timetable + 1) % timetables.size();
    int next = start;

    while(1) {
        debugLog("current_timetable = %d, next = %d\n", current_timetable, next);

        timetables[next]->refresh();

        //found one with entries, use that
        if(timetables[next]->getEntryCount() > 0) {

            Timetable* prev = getCurrentTimetable();
            if(prev!=0) prev->fadeOut();

            current_timetable = next;

            Timetable* current = getCurrentTimetable();

            current->fadeIn();

            return;
        }

        next = (next + 1) % timetables.size();

        //if we've gone around in a loop and even the current one is empty, bail out
        if(next == start) break;
    }

    //didnt find any
    current_timetable = -1;
}

void TimetableViewer::addTimetable(std::string title, std::string timetablefile) {
    Timetable* timetable = new Timetable(title, timetablefile);
    timetables.push_back(timetable);
}

void TimetableViewer::logic(float dt) {

    if(timetables.size() == 0) return;

    elapsed -= dt;
    if(elapsed <= 0.0) fadeOutCurrent();

    Timetable* current = getCurrentTimetable();

    if(next_timetable && (current==0|| !current->isVisible())) {
        switchToNextPopulatedTimetable();

        //regardless of success or not, we will set set next_timetable to false
        //so we dont try again for another duration

        next_timetable = false;
        elapsed = duration;

        current = getCurrentTimetable();
    }

    if(current != 0) current->logic(dt);

}

void TimetableViewer::draw(float dt) {
     if(timetables.size() == 0) return;

    Timetable* current = getCurrentTimetable();

    if(current==0 || current->getEntryCount()==0) return;

   //draw transparent background
    glDisable(GL_TEXTURE_2D);

    float entry_height = current->getEntryHeight();
    float timetable_height = current->getHeight();// + 10.0;
    vec2f timetable_corner = vec2f(0.0, 130.0);

    float table_alpha = current->getAlpha();

    glBegin(GL_QUADS);
        glColor4f(gConfHUDColourVisor.x, gConfHUDColourVisor.y, gConfHUDColourVisor.z, gConfHUDColourVisor.w * table_alpha);

        glVertex2f(timetable_corner.x, timetable_corner.y);
        glVertex2f(display.width,      timetable_corner.y);

        glVertex2f(display.width,      timetable_corner.y + timetable_height);
        glVertex2f(timetable_corner.x, timetable_corner.y + timetable_height);
    glEnd();

    glEnable(GL_TEXTURE_2D);

    glPushMatrix();

        glTranslatef(timetable_corner.x, timetable_corner.y - entry_height*0.75, 0.0);

        float pos1 = 20;
        float pos2 = display.width * 0.25;

        glColor3fv(gConfHUDColourTitle);

        font.draw(pos1, 0.0, "Room");
        font.draw(pos2, 0.0, "Time");

        glTranslatef(0.0, 10.0, 0.0);

        current->draw(dt);

    glPopMatrix();
}
