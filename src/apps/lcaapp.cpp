#include "lcaapp.h"

LCAApp::LCAApp(std::string conffile) : ConfApp(conffile) {
}

void LCAApp::init() {
    app = new LCACoast();
    app->init();

    ready=true;
}
