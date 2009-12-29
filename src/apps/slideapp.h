#ifndef SLIDESHOW_CONFAPP_H
#define SLIDESHOW_CONFAPP_H

#include <dirent.h>

#include "confapp.h"
#include "lca/slideshow.h"

class SlideShowApp : public ConfApp {
public:
    SlideShowApp(std::string conffile);
    void init();
};

#endif
