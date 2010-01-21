#ifndef SLIDESHOW_CONFAPP_H
#define SLIDESHOW_CONFAPP_H

#include <dirent.h>
#include <algorithm>

#include "confapp.h"


class SlideShow : public SDLApp {

    TextureResource* image;
    std::vector<std::string> images;
    int image_index;

    float duration;
    float elapsed;
    float alpha;

    vec2f screen_ratio;

    bool loadNextImage();

    vec2f getAspectRatio();
public:
    SlideShow(float image_duration, vec2f screen_ratio);
    ~SlideShow();

    void addImageFile(std::string imagefile);
    void randomize();

    void logic(float t, float dt);
    void draw(float t, float dt);
};

class SlideShowApp : public ConfApp {
public:
    SlideShowApp(std::string conffile);
    void init();
};

#endif
