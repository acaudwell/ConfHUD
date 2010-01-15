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

bool gConfHUDReload = false;

#ifdef _WIN32
HWND confHUDconsoleWindow = 0;

void ConfHUDWindowsConsole() {
    if(confHUDconsoleWindow !=0) return;

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

    confHUDconsoleWindow = 0;

    //wait for our console window
    while(confHUDconsoleWindow==0) {
        confHUDconsoleWindow = FindWindow(0, consoleTitle);
        SDL_Delay(100);
    }

    //disable the close button so the user cant crash it
    HMENU hm = GetSystemMenu(confHUDconsoleWindow, false);
    DeleteMenu(hm, SC_CLOSE, MF_BYCOMMAND);
}
#endif

//info message
void confhud_info(std::string msg) {
#ifdef _WIN32
    ConfHUDWindowsConsole();
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
    ConfHUDWindowsConsole();
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
    ConfHUDWindowsConsole();

    //resize window to fit help message
    if(confHUDconsoleWindow !=0) {
        RECT windowRect;
        if(GetWindowRect(confHUDconsoleWindow, &windowRect)) {
            float width = windowRect.right - windowRect.left;
            MoveWindow(confHUDconsoleWindow,windowRect.left,windowRect.top,width,400,true);
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

void confhud_trigger_reload(int param) {
    gConfHUDReload=true;
}


ConfHUD::ConfHUD(std::string conffile) {

#ifdef SIGHUP
    //register to handle HUP signal
    signal (SIGHUP, confhud_trigger_reload);
#endif

    if(!conf.load(gSDLAppConfDir + conffile)) {
        confhud_help("failed to load confhud.conf");
    }

    debug = false;

    timetable_viewer = new TimetableViewer();
    confapp = 0;

    logo = 0;

    scroll_message_y = display.height - 120;

    font = fontmanager.grab("FreeSans.ttf", 16);
    font.dropShadow(true);
    font.roundCoordinates(true);

    scrollfont = fontmanager.grab("FreeSans.ttf", 32);
    scrollfont.shadowStrength(0.4);
    scrollfont.dropShadow(true);
    scrollfont.roundCoordinates(true);

    //hide mouse cursor
    SDL_ShowCursor(false);

    //make footer repeat horizontally
    footer = texturemanager.grab("footer.jpg");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    lastFrame = display.emptyTexture(display.width, display.height);

    reset();
    readConfig();

    updateScrollMessage();
}


void ConfHUD::reset() {
    if(confapp != 0) {
        delete confapp;
        confapp = 0;
    }

    playlist_index = -1;
    transition = 0.0;
    timetable_viewer->reset();
}

void ConfHUD::reloadConfig() {
    gConfHUDReload = false;

    reset();
    readConfig();
    updateScrollMessage();
}

void ConfHUD::readConfig() {
    if(!conf.load()) return;

    if(conf.hasSection("settings")) {
        message_file   = conf.getString("settings", "message_file");
        playlist_conf  = conf.getString("settings", "playlist_conf");
        timetable_conf = conf.getString("settings", "timetable_conf");
    }

    if(conf.hasValue("settings", "message_y")) {
        scroll_message_y = conf.getFloat("settings", "message_y");
    }

    if(conf.hasValue("settings", "logo_file")) {
        std::string logo_file = conf.getString("settings", "logo_file");
        logo_pos = conf.getVec2("settings", "logo_position");

        if(logo_file.size()) {
            logo = texturemanager.grab(logo_file, 1, 1, 0, true);
        }
    }

    readTimetables();
}

void ConfHUD::readTimetables() {
    if(!timetable_conf.size()) return;

    ConfFile ttconf;
    if(!ttconf.load(timetable_conf)) {
        debugLog("failed to read  timetable conf '%s'\n", timetable_conf.c_str());
    }

    float viewer_duration = ttconf.getFloat("settings", "duration");

    //check the value is sensible
    if(viewer_duration<1.0) viewer_duration = 15.0;
    timetable_viewer->setDuration(viewer_duration);

    timetable_viewer->reset();

    ConfSectionList* timetables = ttconf.getSections("timetable");

    if(timetables == 0) return;

    for(ConfSectionList::iterator it = timetables->begin(); it != timetables->end(); it++) {

        ConfSection* section = (*it);

        //check it has required values
        if(!section->hasValue("title")) continue;
        if(!section->hasValue("file")) continue;

        std::string title = section->getString("title");
        std::string file  = section->getString("file");

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

    nextApp = true;
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

        if (e->keysym.sym == SDLK_r) {
            gConfHUDReload = true;
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
    if(playlist_conf.size()==0) return;

    if(!playlist.load(playlist_conf)) {
        debugLog("failed to open playlist conf '%s'\n", playlist_conf.c_str());
        return;
    }
}

ConfApp* ConfHUD::getNextApp() {

    if(confapp != 0) {
        delete confapp;
        confapp = 0;
    }

    readPlaylist();

    ConfSectionList* confapps = playlist.getSections("confapp");

    if(confapps == 0) return 0;

    playlist_index = (playlist_index + 1) % confapps->size();

    ConfSectionList::iterator it = confapps->begin();
    for(int i=0;i<playlist_index;i++) it++;

    ConfSection* appcfg = (*it);

    debugLog("type=%s\n", appcfg->getString("type").c_str());

    if(appcfg->hasValue("type") && appcfg->hasValue("config")) {
        std::string appname = appcfg->getString("type");
        std::string appconf = appcfg->getString("config");

        ConfApp* app = 0;

        if(appname == "blank") {
            app = new BlankApp(appconf);
        } else if(appname == "slideshow") {
            app = new SlideShowApp(appconf);
        } else if(appname == "stackgraph") {
            app = new StackApp(appconf);
        } else if(appname == "gource") {
            app = new GourceApp(appconf);
        }

        if(app != 0) {
            app->prepare();

            if(app->prepareFailed()) {
                delete app;
                return 0;
            }

            app->init();

            if(!app->isReady()) {
                delete app;
                return 0;
            }
        }

        transition = 1.0;

        return app;
    }

    return 0;
}

void ConfHUD::updateConfApp(float dt) {

    // TODO: check prepare succeeded, etc

    if(confapp == 0 || nextApp || confapp->isFinished()) {
        nextApp = false;

        confapp = getNextApp();

        //HACK: make 2 attempts in case first app fails to initialze
        if(confapp==0) confapp = getNextApp();

        if(confapp==0) return;
    }

    try {
        confapp->logic(dt);
    } catch(...) {
        confapp = 0;
    }
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
    if(gConfHUDReload) reloadConfig();

    //update scroll bar
    scroll_message_x -= dt * 120.0f;
    if(scroll_message_x < (-scroll_message_width - 100.0)) {
        updateScrollMessage();
    }

    timetable_viewer->logic(dt);

    updateConfApp(dt);

    updateColours(dt);
}

void ConfHUD::drawBackground(float dt) {

    float footer_start_y = display.height - footer->h;
    float footer_w_ratio = ((float) display.width / footer->w) * 0.75;

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


        glColor3f(0.97, 0.98, 0.99);

        //quad5

        glVertex2f(0.0f,          bar4);
        glVertex2f(display.width, bar4);

        glColor3f(1.0, 1.0, 1.0);

        glVertex2f(display.width, footer_start_y);
        glVertex2f(0.0f,          footer_start_y);
    glEnd();

    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

void ConfHUD::drawLogo(float dt) {
    if(logo==0) return;

    glBindTexture(GL_TEXTURE_2D, logo->textureid);

    glPushMatrix();

    glTranslatef(logo_pos.x, logo_pos.y, 0.0f);

    glColor4f(1.0, 1.0, 1.0, 1.0);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0f);

        glTexCoord2f(1.0, 0.0f);
        glVertex2f(logo->w, 0.0f);

        glTexCoord2f(1.0, 1.0f);
        glVertex2f(logo->w, logo->h);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, logo->h);
    glEnd();

    glPopMatrix();
}

void ConfHUD::draw() {

    display.clear();

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    display.mode2D();

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    drawBackground(dt);

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

    scrollfont.draw(scroll_message_x, scroll_message_y, scroll_message);

    //draw logo
    drawLogo(dt);

    if(debug) {
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
        font.print(10, 10, "FPS %.2f", fps);
    }
}

