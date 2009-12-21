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

float gConfHUDTimetableDuration = 15.0;

// TimetableViewer

TimetableViewer::TimetableViewer() {
    next_timetable = false;
    current_timetable = 0;

    duration_length = gConfHUDTimetableDuration;
    duration        = duration_length;

    font = fontmanager.grab("FreeSans.ttf", 32);
    font.shadowStrength(0.4);
    font.dropShadow(false);
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

    Timetable* current = timetables[current_timetable];

    current->fadeOut();
}

void TimetableViewer::addTimetable(std::string title, std::string timetablefile) {
    Timetable* timetable = new Timetable(title, timetablefile);

    if(timetable->getEntryCount() > 0) timetables.push_back(timetable);
    else delete timetable;
}

void TimetableViewer::logic(float dt) {

    if(timetables.size() == 0) return;

    if(duration > 0.0) {
        duration -= dt;
        if(duration <= 0.0) nextTimetable();
    }

    Timetable* current = timetables[current_timetable];

    if(next_timetable && !current->isVisible()) {
        current_timetable = (current_timetable + 1) % timetables.size();

        current = timetables[current_timetable];

        current->refresh();
        current->fadeIn();

        next_timetable = false;
        duration = duration_length;
    }

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

        glTranslatef(timetable_corner.x, timetable_corner.y - entry_height, 0.0);

        float pos1 = 20;
        float pos2 = display.width * 0.1;

        glColor3fv(gConfHUDColourTitle);

        font.draw(pos1, 0.0, "Room");
        font.draw(pos2, 0.0, "Time");

        glTranslatef(0.0, 10.0, 0.0);

        current->draw(dt);

    glPopMatrix();
}
