#include "lcaapp.h"

LCAApp::LCAApp(std::string conffile) : ConfApp(conffile) {
}

void LCAApp::init() {
    app = new LCABackground();
    app->init();

    ready=true;
}
