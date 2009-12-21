#ifndef GOURCE_CONFAPP_H
#define GOURCE_CONFAPP_H

#include "confapp.h"
#include "gource/gource.h"

class GourceApp : public ConfApp {
public:
    GourceApp(std::string conffile);
    void init();
};

#endif
