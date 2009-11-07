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

#include "confhud.h"


#ifdef _WIN32
HWND consoleWindow = 0;

void createWindowsConsole() {
    if(consoleWindow !=0) return;

    //create a console on Windows so users can see messages

    //find an available name for our window
    int console_suffix = 0;
    char consoleTitle[512];
    sprintf(consoleTitle, "%s", "ConfHUD Console");

    while(FindWindow(0, consoleTitle)) {
        sprintf(consoleTitle, "ConfHUD Console %d", ++console_suffix);
    }

    AllocConsole();
    SetConsoleTitle(consoleTitle);

    //redirect streams to console
    freopen("conin$", "r", stdin);
    freopen("conout$","w", stdout);
    freopen("conout$","w", stderr);

    consoleWindow = 0;

    //wait for our console window
    while(consoleWindow==0) {
        consoleWindow = FindWindow(0, consoleTitle);
        SDL_Delay(100);
    }

    //disable the close button so the user cant crash it
    HMENU hm = GetSystemMenu(consoleWindow, false);
    DeleteMenu(hm, SC_CLOSE, MF_BYCOMMAND);
}
#endif

//info message
void confhud_info(std::string msg) {
#ifdef _WIN32
    createWindowsConsole();
#endif

    printf("%s\n", msg.c_str());

#ifdef _WIN32
    printf("\nPress Enter\n");
    getchar();
#endif

    exit(0);
}

//display error only
void confhud_quit(std::string error) {
    SDL_Quit();

#ifdef _WIN32
    createWindowsConsole();
#endif

    printf("Error: %s\n\n", error.c_str());

#ifdef _WIN32
    printf("Press Enter\n");
    getchar();
#endif

    exit(1);
}

//display help message + error (optional)
void confhud_help(std::string error) {

#ifdef _WIN32
    createWindowsConsole();

    //resize window to fit help message
    if(consoleWindow !=0) {
        RECT windowRect;
        if(GetWindowRect(consoleWindow, &windowRect)) {
            float width = windowRect.right - windowRect.left;
            MoveWindow(consoleWindow,windowRect.left,windowRect.top,width,850,true);
        }
    }
#endif

    printf("ConfHUD v%s\n", CONFHUD_VERSION);

    if(error.size()) {
        printf("Error: %s\n\n", error.c_str());
    }

    printf("Usage: confhud [OPTIONS]\n");
    printf("\nOptions:\n");
    printf("  -h, --help                       Help\n\n");
    printf("  -WIDTHxHEIGHT                    Set window size\n");
    printf("  -f                               Fullscreen\n\n");
    printf("  --duration SECONDS               Time duration each timetable is shown\n\n");


#ifdef _WIN32
    printf("Press Enter\n");
    getchar();
#endif

    //check if we should use an error code
    if(error.size()) {
        exit(1);
    } else {
        exit(0);
    }
}

ConfHUD::ConfHUD() {

    debug = false;

    timetable_viewer = 0;

    font = fontmanager.grab("FreeSans.ttf", 16);
    font.dropShadow(true);
    font.roundCoordinates(true);

    scrollfont = fontmanager.grab("FreeSans.ttf", 32);
    scrollfont.shadowStrength(0.4);
    scrollfont.dropShadow(true);
    scrollfont.roundCoordinates(true);

    footer = texturemanager.grab("footer.jpg");

    //make footer repeat horizontally
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    updateScrollMessage();

    reset();
}


void ConfHUD::reset() {
    if(timetable_viewer !=0) delete timetable_viewer;
    timetable_viewer = new TimetableViewer();

    timetable_viewer->addTimetable("data/test/talks.txt");
    timetable_viewer->addTimetable("data/test/miniconfs.txt");
}

ConfHUD::~ConfHUD() {
    reset();

    if(timetable_viewer != 0) delete timetable_viewer;
}

void ConfHUD::updateScrollMessage() {
    debugLog("Updating scroll message\n");
    
    //get message
    std::ifstream in("data/test/message.txt");

    if(in.is_open()) {
        char buff[4096];
        in.getline(buff, 4096);
        in.close();

        scroll_message = std::string(buff);
    }

    scroll_message_width = scrollfont.getWidth(scroll_message);
    scroll_message_x = display.width;
}

void ConfHUD::init() {
}

void ConfHUD::update(float t, float dt) {
    this->t  = t;
    this->dt = dt;

    logic();
    draw();
}

void ConfHUD::mouseMove(SDL_MouseMotionEvent *e) {
    vec2f mousepos = vec2f(e->x, e->y);
}

void ConfHUD::mouseClick(SDL_MouseButtonEvent *e) {
    if(e->type != SDL_MOUSEBUTTONDOWN) return;

}

void ConfHUD::keyPress(SDL_KeyboardEvent *e) {
    if (e->type == SDL_KEYUP) return;

    if (e->type == SDL_KEYDOWN) {
        if (e->keysym.sym == SDLK_ESCAPE) {
            appFinished=true;
        }

        if (e->keysym.sym == SDLK_q) {
            debug = !debug;
        }
    }
}

void ConfHUD::logic() {

    //update scroll bar
    scroll_message_x -= dt * 120.0f;
    if(scroll_message_x < (-scroll_message_width - 100.0)) scroll_message_x = display.width;

    timetable_viewer->logic(dt);
}

void ConfHUD::drawBackground() {

    glDisable(GL_TEXTURE_2D);

    float bar1 = display.height * 0.15;
    float bar2 = display.height * 0.30;
    float bar3 = display.height * 0.50;
    float bar4 = display.height * 0.75;

    glBegin(GL_QUADS);
        glColor3f(0.17, 0.47, 0.76);

        //quad1

        glVertex2f(0.0f, 0.0f);
        glVertex2f(display.width, 0.0f);

        glColor3f(0.66, 0.80, 0.91);

        glVertex2f(display.width, bar1);
        glVertex2f(0.0f,          bar1);

        //quad2

        glVertex2f(0.0f,          bar1);
        glVertex2f(display.width, bar1);

        glColor3f(0.93, 0.96, 0.98);

        glVertex2f(display.width, bar2);
        glVertex2f(0.0f,          bar2);

        //quad3

        glVertex2f(0.0f,          bar2);
        glVertex2f(display.width, bar2);

        glColor3f(0.85, 0.92, 0.95);

        glVertex2f(display.width, bar3);
        glVertex2f(0.0f,          bar3);

        //quad4

        glVertex2f(0.0f,          bar3);
        glVertex2f(display.width, bar3);

        glColor3f(0.97, 0.98, 0.99);

        glVertex2f(display.width, bar4);
        glVertex2f(0.0f,          bar4);

        //quad5

        glVertex2f(0.0f,          bar4);
        glVertex2f(display.width, bar4);

        glColor3f(1.0, 1.0, 1.0);

        glVertex2f(display.width, display.height);
        glVertex2f(0.0f,          display.height);
    glEnd();

    glEnable(GL_TEXTURE_2D);


    float footer_start_y = display.height - footer->h;

    float footer_w_ratio = ((float) display.width / footer->w) * 0.75;

    glBindTexture(GL_TEXTURE_2D, footer->textureid);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, footer_start_y);

        glTexCoord2f(footer_w_ratio, 0.0f);
        glVertex2f(display.width, footer_start_y);

        glTexCoord2f(footer_w_ratio, 1.0f);
        glVertex2f(display.width, display.height);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, display.height);
    glEnd();
}

void ConfHUD::draw() {

    display.clear();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    display.mode2D();

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    drawBackground();

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    timetable_viewer->draw(dt);

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glColor4f(0.17, 0.47, 0.76, 1.0);
//    glColor4f(0.66, 0.80, 0.91, 1.0);

    scrollfont.draw(scroll_message_x, display.height - 120.0, scroll_message);

    if(debug) {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        font.print(10, 10, "FPS %.2f", fps);
    }
}

