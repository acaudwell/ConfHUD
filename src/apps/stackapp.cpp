#include "stackapp.h"

StackApp::StackApp(std::string conffile) : ConfApp(conffile) {
    max_tick_rate = 1.0 / 60.0;
}

void StackApp::init() {

    std::string logfile;
    vec3f background;
    bool drawBackground = false;

    std::string value;

    std::string title = config.getString("stackgraph", "title");

    if((value = config.getString("stackgraph", "logfile")) != "") {
        logfile = value;
    }

    if(config.hasValue("stackgraph", "background")) {
        background = config.getVec3("stackgraph", "background");
        drawBackground = true;
    }

    gStackGraphDrawBackground  = drawBackground;

    //create StackGraph object
    StackGraph* g = new StackGraph(title, logfile);

    g->init();

    //set background colour
    g->setBackgroundColour(background);

    app = g;

    ready=true;
}
