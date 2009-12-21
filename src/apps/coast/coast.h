#ifndef LCA_COAST_H
#define LCA_COAST_H

#include "core/sdlapp.h"

class LCACoast : public SDLApp {

    TextureResource* footer;

    void drawBackground();
public:
    LCACoast();
    ~LCACoast();

    void draw(float t, float dt);
};

#endif
