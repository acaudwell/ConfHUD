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

Regex playlist_line("^([^\\|]+)\\|(.+)$");

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

ConfHUD::ConfHUD(ConfFile* conf) {

    this->conf = conf;

    debug = false;

    timetable_viewer = 0;
    confapp = 0;

    font = fontmanager.grab("FreeSans.ttf", 16);
    font.dropShadow(true);
    font.roundCoordinates(true);

    scrollfont = fontmanager.grab("FreeSans.ttf", 32);
    scrollfont.shadowStrength(0.4);
    scrollfont.dropShadow(true);
    scrollfont.roundCoordinates(true);

    lastFrame = display.emptyTexture(display.width, display.height);

    playlist_index = -1;

    transition = 0.0;

    reset();
    readConfig();

    updateScrollMessage();
}


void ConfHUD::reset() {
    if(timetable_viewer !=0) delete timetable_viewer;
    timetable_viewer = new TimetableViewer();
}

void ConfHUD::readConfig() {
    if(!conf->load()) return;

    if(conf->hasSection("settings")) {
        message_file = conf->getString("settings", "message_file");
        playlist_file = conf->getString("settings", "playlist_file");
    }

    int timetable_no = 1;
    while(1) {
        char title_buff[1024];
        sprintf(title_buff, "timetable_%d", timetable_no++);

        std::string section = std::string(title_buff);

        if(!conf->hasSection(section)) break;

        //check it has required values
        if(!conf->hasValue(section, "title")) continue;
        if(!conf->hasValue(section, "file")) continue;

        std::string title = conf->getString(section, "title");
        std::string file  = conf->getString(section, "file");

        timetable_viewer->addTimetable(title, file);
    }
}

ConfHUD::~ConfHUD() {
    reset();

    if(timetable_viewer != 0) delete timetable_viewer;

    if(lastFrame != 0) glDeleteTextures(1, &lastFrame);
}

void ConfHUD::updateScrollMessage() {

    if(message_file.size()==0) return;

    debugLog("Updating scroll message\n");

    //get message
    std::ifstream in(message_file.c_str());

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

        if (e->keysym.sym == SDLK_n) {
            nextApp = true;
        }

    }
}

void ConfHUD::updateColours(float dt) {

    if(confapp == 0) return;

    float amount = std::min(1.0f, dt*2.0f);

    gConfHUDColourDescription += (confapp->getColourDescription() - gConfHUDColourDescription) * amount;
    gConfHUDColourTitle       += (confapp->getColourTitle() - gConfHUDColourTitle) * amount;
    gConfHUDColourTime        += (confapp->getColourTime() - gConfHUDColourTime) * amount;
    gConfHUDColourMessage     += (confapp->getColourMessage() - gConfHUDColourMessage) * amount;
    gConfHUDColourVisor       += (confapp->getColourVisor() - gConfHUDColourVisor) * amount;
}

void ConfHUD::readPlaylist() {
    if(playlist_file.size()==0) return;

    std::ifstream in(playlist_file.c_str());

    if(!in.is_open()) {
        debugLog("failed to open playlist file '%s'\n", playlist_file.c_str());
        return;
    }

    playlist.clear();

    char buff[1024];

    while(in.getline(buff, 1024)) {

        std::string line = std::string(buff);

        if(line.size()==0 || line[0] == '#') continue;
        if(!playlist_line.match(line)) continue;

        playlist.push_back(line);
    }
}

ConfApp* ConfHUD::getNextApp() {
    if(confapp != 0) {
        delete confapp;
        confapp = 0;
    }

    readPlaylist();

    playlist_index = (playlist_index + 1) % playlist.size();

    std::vector<std::string> listentry;

    if(playlist_line.match(playlist[playlist_index], &listentry)) {
        std::string appname = listentry[0];
        std::string appconf = listentry[1];

        ConfApp* app = 0;

        if(appname == "lca") {
            app = new LCAApp(appconf);
        } else if(appname == "gource") {
            app = new GourceApp(appconf);
        }

        if(app != 0) {
            app->prepare();
            app->init();
        }

        transition = 1.0;

        return app;
    }

    return 0;
}

void ConfHUD::updateConfApp(float dt) {

    // TODO: check prepare succeeded, etc

    if(confapp == 0 || nextApp || confapp->isFinished() || confapp->prepareFailed()) {
        nextApp = false;

        confapp = getNextApp();

        if(confapp==0) return;
    }

    confapp->logic(dt);
}

void ConfHUD::drawConfApp(float dt) {
    if(confapp==0) return;

    confapp->draw();

    if(lastFrame && transition > 0.0) {
        display.mode2D();

        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glBindTexture(GL_TEXTURE_2D, lastFrame);

        glColor4f(1.0, 1.0, 1.0, std::max(0.0f, transition));

        glBegin(GL_QUADS);
            glTexCoord2f(0.0f, 1.0f);
            glVertex2f(0.0f, 0.0);

            glTexCoord2f(1.0, 1.0f);
            glVertex2f(display.width, 0.0);

            glTexCoord2f(1.0, 0.0f);
            glVertex2f(display.width, display.height);

            glTexCoord2f(0.0f, 0.0f);
            glVertex2f(0.0f, display.height);
        glEnd();

        transition -= dt;
    } else {
        display.renderToTexture(lastFrame, display.width, display.height, GL_RGBA);
    }
}

void ConfHUD::logic() {

    //update scroll bar
    scroll_message_x -= dt * 120.0f;
    if(scroll_message_x < (-scroll_message_width - 100.0)) scroll_message_x = display.width;

    timetable_viewer->logic(dt);

    updateConfApp(dt);

    updateColours(dt);
}

void ConfHUD::draw() {

    display.clear();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    display.mode2D();

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    drawConfApp(dt);

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    timetable_viewer->draw(dt);

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glColor4f(gConfHUDColourMessage.x, gConfHUDColourMessage.y, gConfHUDColourMessage.z, 1.0);

    scrollfont.draw(scroll_message_x, display.height - 120.0, scroll_message);

    if(debug) {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        font.print(10, 10, "FPS %.2f", fps);
    }
}

