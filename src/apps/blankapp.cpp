#include "blankapp.h"

BlankApp::BlankApp(std::string conffile) : ConfApp(conffile) {
}

void BlankApp::init() {
    app = new SDLApp();
    app->init();

    ready=true;
}
