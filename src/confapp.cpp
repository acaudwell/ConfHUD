#include "confapp.h"

ConfApp::ConfApp(std::string conffile) {

    this->conffile = conffile;

    app             = 0;
    prepare_failed  = false;
    ready           = false;
    prepared        = false;
    duration        = 0.0;
    elapsed         = 0.0;
    max_tick_rate   = 0.0;
    scaled_t        = 0.0;
    scaled_dt       = 0.0;

    colour_desc    = vec3f(1.0, 1.0, 1.0);
    colour_title   = vec3f(1.0, 1.0, 1.0);
    colour_time    = vec3f(1.0, 1.0, 1.0);
    colour_message = vec3f(1.0, 1.0, 1.0);
    colour_visor   = vec4f(1.0, 1.0, 1.0, 0.4);
}

vec3f ConfApp::getColourDescription() {
    return colour_desc;
}

vec3f ConfApp::getColourTime() {
    return colour_time;
}

vec3f ConfApp::getColourTitle() {
    return colour_title;
}

vec3f ConfApp::getColourMessage() {
    return colour_message;
}

vec4f ConfApp::getColourVisor() {
    return colour_visor;
}

bool ConfApp::isPrepared() {
    return prepared;
}

bool ConfApp::isReady() {
    return ready;
}

bool ConfApp::prepareFailed() {
    return prepare_failed;
}

bool ConfApp::isFinished() {
    if(app!=0 && app->isFinished()) return true;
    if(duration>0.0 && elapsed > duration) return true;

    return false;
}

void ConfApp::prepare() {

    if(conffile.size() == 0) {
        debugLog("no config file supplied\n");
        prepare_failed = true;
        return;
    }

    if(!config.load(conffile)) {
            std::string error = config.getError();
            debugLog("error reading conffile: %s\n", error.c_str());
            prepare_failed = true;
            return;
    }

    // title / subtitle
    std::string title    = config.getString("confapp", "title");
    std::string subtitle = config.getString("confapp", "subtitle");

    setTitle(title, subtitle);

    // colours

    if(config.hasValue("confapp", "colour_title")) {
        this->colour_title = config.getVec3("confapp", "colour_title");
    }
    if(config.hasValue("confapp", "colour_time")) {
        this->colour_time = config.getVec3("confapp", "colour_time");
         debugLog("vec3f = %.2f, %.2f, %.2f, %.2f\n", colour_time.x, colour_time.y, colour_time.z);
    }
    if(config.hasValue("confapp", "colour_desc")) {
        this->colour_desc = config.getVec3("confapp", "colour_desc");
    }
    if(config.hasValue("confapp", "colour_message")) {
        this->colour_message = config.getVec3("confapp", "colour_message");
    }
    if(config.hasValue("confapp", "colour_visor")) {
        this->colour_visor = config.getVec4("confapp", "colour_visor");
         debugLog("vec4f = %.2f, %.2f, %.2f, %.2f\n", colour_visor.x, colour_visor.y, colour_visor.z, colour_visor.w);
    }

    // duration

    float duration = config.getFloat("confapp", "duration");

    if(duration > 0.0) {
        this->duration = duration;
    }

    // run preprocess script

    std::string script = config.getString("confapp", "script");

    if(script.size() > 0) {
        int rc = system(script.c_str());

        //prepare failed
        if(rc !=0) {
            debugLog("command '%s' failed with return code %d\n", script.c_str(), rc);
            prepare_failed = true;
            return;
        }
    }

    prepared = true;
}

void ConfApp::setTitle(std::string title, std::string subtitle) {
    this->title = title;
    this->subtitle = subtitle;
}

void ConfApp::logic(float dt) {
    if(!ready || app->isFinished()) return;

    if(duration>0.0) this->elapsed += dt;

    dt = std::min(max_tick_rate, dt);

    scaled_t += dt;
    scaled_dt = dt;

    app->logic(scaled_t, scaled_dt);
}

void ConfApp::draw() {
    if(!ready || app->isFinished()) return;

    app->draw(scaled_t, scaled_dt);

    float elapse_time = 15.0;
    float elapsed_fade = 5.0;

//    if(elapsed < elapse_time + elapsed_fade) {
        glActiveTextureARB(GL_TEXTURE0);
        glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

//        float fade = elapsed < elapse_time ? 1.0 : 1.0 - (elapsed-elapse_time)/elapsed_fade;
//        glColor4f(colour_desc.x, colour_desc.y, colour_desc.z, fade);
        glColor4f(colour_title.x, colour_title.y, colour_title.z, 1.0);

        FXFont font = fontmanager.grab("FreeSans.ttf", 22);

        font.dropShadow(true);
        font.alignTop(false);

        font.draw(10.0, display.height - 10.0, title.c_str());

        font.alignRight(true);
        font.alignTop(false);

        font.draw(display.width - 10.0, display.height - 10.0, subtitle.c_str());
//    }
}

