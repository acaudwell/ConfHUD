#ifndef CONFAPP_H
#define CONFAPP_H

#include "core/sdlapp.h"
#include "core/fxfont.h"
#include "core/conffile.h"

class ConfApp {
protected:
    SDLApp* app;

    float duration;
    float elapsed;

    float fixed_tick_rate;

    float scaled_t;
    float scaled_dt;

    bool ready;
    bool prepared;
    bool prepare_failed;

    std::string title;
    std::string subtitle;

    std::string conffile;
    ConfFile config;

    vec3f colour_desc;
    vec3f colour_title;
    vec3f colour_time;
    vec3f colour_message;
    vec4f colour_visor;
public:
    ConfApp(std::string conffile);

    void logic(float dt);
    void draw();

    void setTitle(std::string title, std::string subtitle);

    bool isReady();
    bool isPrepared();
    bool isFinished();
    bool prepareFailed();

    vec3f getColourDescription();
    vec3f getColourTime();
    vec3f getColourTitle();
    vec3f getColourMessage();
    vec4f getColourVisor();

    void prepare();

    virtual void init() { ready=true; };
};

#endif

