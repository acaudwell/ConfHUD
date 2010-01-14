#include "gapp.h"

GourceApp::GourceApp(std::string conffile) : ConfApp(conffile) {
    max_tick_rate = 1.0 / 60.0;
}

void GourceApp::init() {

    srand ( time(0) );

    float start_position = ((float) (rand() % 1000)) / 1000.0f;
    float rate = 0.1;
    float elasticity = 0.0;

    bool hidefilenames=false;
    bool hidedate=false;

    float bloom_intensity = 0.25;
    float bloom_multiplier = 1.0;

    bool drawBackground=false;
    bool zoomin=false;

    std::string userimagedir;
    std::string defaultuserimage;

    std::string logfile;

    vec3f background(0.1, 0.1, 0.1);

    std::string value;

    if((value = config.getString("gource", "logfile")) != "") {
        logfile = value;
    }

    if((value = config.getString("gource", "elasticity")) != "") {
        elasticity = atof(value.c_str());
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

    if(config.getBool("gource", "hidedate")) {
        hidedate = true;
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
        drawBackground = true;
    }

    if(config.hasValue("gource", "user-image-dir")) {
        userimagedir = config.getString("gource", "user-image-dir");

        //append slash
        if(userimagedir[userimagedir.size()-1] != '/') {
            userimagedir += std::string("/");
        }
    }

    defaultuserimage = config.getString("gource", "user-image");

    gGourceElasticity = elasticity;

    gGourceUserImageDir = userimagedir;

    gGourceDefaultUserImage = defaultuserimage;
    gGourceColourUserImages = defaultuserimage.size() > 0 ? true : false;

    gGourceDrawBackground  = drawBackground;
    gGourceDisableProgress = true;

    gGourceHideFilenames = hidefilenames;
    gGourceHideDate      = hidedate;

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
