#ifndef STACKGRAPH_H
#define STACKGRAPH_H

#ifdef _WIN32
#include "windows.h"
#endif

#include "core/display.h"
#include "core/sdlapp.h"
#include "core/fxfont.h"
#include "core/bounds.h"

#include "node.h"
#include "bet.h"

#include <list>

void stackgraph_help(std::string error);

class StackGraph : public SDLApp {
    bool debug;

    vec3f clearcolour;
    vec3f border_colour;

    bool vertical;

    vec2f mousepos;
    bool  mouseclicked;
    bool  mousemoved;

    StackNode* mouseOverNode;

    StackNode* selectedNode;

    StackNode* activeNode;

    bool paused;

    float last_update;
    float last_added;

    float in_transition;

    float margin;

    vec3f transition_colour;

    Bounds2D selected_bounds;
    Bounds2D section_bounds;

    float bargap;

    float alpha;

    float scale_factor, current_scale_factor;

    vec2f corner;
    float width, height;

    bool theme_black;

    FXFont titlefont;
    FXFont timefont;
    FXFont keyfont;

    std::string displaydate;

    void toggleVertical();

    void resetView();

    void setColourTheme();

    void setActiveNode(StackNode* node);

    void drawScaleKey();
    void drawTime();
    void drawTransition(float dt);
    void selectNode(StackNode* node);
    void updateTransition(float dt);
    void updateScale(float dt);
public:
    StackGraph();
    ~StackGraph();

    void updateGraph(StackNode* node);

    std::string dollarAmount(double num);

    void setBackgroundColour(vec3f background);

    bool isBlackTheme();

    float getMargin();

    FXFont getTitleFont();
    FXFont getTimeFont();
    FXFont getKeyFont();

    bool isVertical();

    float getLength();
    float getBreadth();

    float getWidth();
    float getHeight();

    float getAlpha();

    vec2f getCorner();

    float getScaleFactor();
    float getCurrentScaleFactor();

    void logic(float t, float dt);
    void draw(float t, float dt);

    void init();
    void update(float t, float dt);
    void keyPress(SDL_KeyboardEvent *e);
    void mouseMove(SDL_MouseMotionEvent *e);
    void mouseClick(SDL_MouseButtonEvent *e);
};

extern bool  gStackGraphDrawBackground;
extern float gStackGraphFetchDelay;

#endif
