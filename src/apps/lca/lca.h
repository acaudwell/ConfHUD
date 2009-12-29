#ifndef LCA_BACKGROUND_H
#define LCA_BACKGROUND_H

#include "core/sdlapp.h"

class LCABackground : public SDLApp {

    TextureResource* footer;

protected:
    void drawBackground();
public:
    LCABackground();
    ~LCABackground();

    void draw(float t, float dt);
};

#endif
