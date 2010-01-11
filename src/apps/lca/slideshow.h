#ifndef LCA_SLIDESHOW_H
#define LCA_SLIDESHOW_H

#include "core/sdlapp.h"

#include "lca.h"

class SlideShow : public SDLApp {

    TextureResource* image;
    std::vector<std::string> images;
    int image_index;

    float duration;
    float elapsed;
    float alpha;

    bool loadNextImage();

    vec2f getAspectRatio();
public:
    SlideShow(float image_duration);
    ~SlideShow();

    void addImageFile(std::string imagefile);

    void logic(float t, float dt);
    void draw(float t, float dt);
};

#endif
