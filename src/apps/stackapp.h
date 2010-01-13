#ifndef STACK_CONFAPP_H
#define STACK_CONFAPP_H

#include "confapp.h"
#include "stackgraph/graph.h"

class StackApp : public ConfApp {

    std::string logfile;

    vec3f bar_colour;

    float graph_elapsed;

    void addBar(StackNode* chart, std::string title, float value);
    void updateGraph();
public:
    StackApp(std::string conffile);
    void init();
    void logic(float dt);
};

#endif
