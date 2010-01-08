#include "gapp.h"

GourceApp::GourceApp(std::string conffile) : ConfApp(conffile) {
    max_tick_rate = 1.0 / 60.0;
}

void GourceApp::init() {

    float start_position = ((float) (rand() % 1000)) / 1000.0f;
    float rate = 0.1;

    bool hidefilenames=false;

    float bloom_intensity = 0.25;
    float bloom_multiplier = 1.0;

    bool zoomin=false;

    std::string logfile;

    vec3f background(0.1, 0.1, 0.1);

//    srand ( 0 );

    std::string value;

    if((value = config.getString("gource", "logfile")) != "") {
        logfile = value;
    }

    if((value = config.getString("gource", "bloom-multiplier")) != "") {
        bloom_multiplier = atof(value.c_str());
    }

    if((value = config.getString("gource", "bloom-intensity")) != "") {
        bloom_intensity = atof(value.c_str());
    }

    if((value = config.getString("gource", "zoomin")) == "1") {
        zoomin = true;
    }

    if((value = config.getString("gource", "hidefilenames")) == "1") {
        hidefilenames = true;
    }

    if((value = config.getString("gource", "rate")) != "") {
        rate = atof(value.c_str());
    }

    if((value = config.getString("gource", "start")) != "") {
        start_position = atof(value.c_str());
    }

    if(config.hasValue("gource", "background")) {
        background = config.getVec3("gource", "background");
    }

    gGourceDrawBackground  = false;
    gGourceDisableProgress = true;

    gGourceHideFilenames = hidefilenames;

    gGourceBloomMultiplier = bloom_multiplier;
    gGourceBloomIntensity  = bloom_intensity;

    gGourceDaysPerSecond = rate;

    //create Gource object
    Gource* g = new Gource(logfile);

    g->init();

    //set start position
    if(start_position>0.0) {
        g->setStartPosition(start_position);
    }

    //show full view of project all the time
    if(zoomin) {
        g->setCameraMode(true);
    }

    //set background colour
    g->setBackground(background);

    app = g;

    ready=true;
}
