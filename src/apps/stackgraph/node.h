#ifndef STACKNODE_H
#define STACKNODE_H

#include "core/bounds.h"
#include "core/regex.h"
#include "core/sdlapp.h"
#include "core/stringhash.h"

#include <string>
#include <fstream>
#include <list>

class StackBet;
class StackGraph;

class StackNode {

    float value;
    float current_value;

    std::string title;

    float alpha;
    float x;

    int title_size;

    float modified;

    float tooltip_alpha;
    float title_alpha;

    vec3f colour;

    float recent_mouse_over;

    StackNode* parent;
    StackGraph* graph;

    std::list<StackBet*> bets;
    std::vector<StackNode*> children;

    bool show_tooltip;

    bool initNode();

    std::string getNodeFilename();
public:
    StackNode(StackGraph* graph, std::string title, float value = 0.0f, vec3f colour = vec3f(0.0, 0.0, 0.0));
    ~StackNode();

    void onBlur();
    void onFocus();

    void showToolTip();

    void setParent(StackNode* parent);

    void addChild(StackNode* child);

    int getChildCount();

    void setX(float x);
    void setTitleSize(int size);

    void setColour(vec3f colour);

    vec3f getColour();
    vec3f barColour();

    Bounds2D getBounds();
    std::string getTitle();

    bool mouseOver(vec2f pos);
    StackNode* mouseOverChildren(vec2f pos);

    bool isLoading();

    StackGraph* getGraph();

    StackNode* getParent();

    float getChildBarWidths();
    float getBarWidth();

    float getLeft();
    float getTop();

    float getValue();

    void setValue(float value);
    void addValue(float value);

    void addBet(StackNode* node, float amount);

    void updateBets(float dt);
    float updateChildren(float dt);
    void positionChildren();

    void logic(float dt);

    void drawChildToolTips();

    void drawBarToolTip(float tip_alpha);

    void drawTitle(float alpha);

    void drawToolTip();
    void drawBar(float dt);
    void drawContent(float dt);
};

extern float gMaxBarWidth;
extern float gMinBarHeight;

extern float gFontShadow;
extern vec3f gTextBaseColour;

#endif
