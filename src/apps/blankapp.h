#ifndef BLANK_CONFAPP_H
#define BLANK_CONFAPP_H

#include "confapp.h"

class BlankApp : public ConfApp {
public:
    BlankApp(std::string conffile);
    void init();
};

#endif
