#ifndef STACK_CONFAPP_H
#define STACK_CONFAPP_H

#include "confapp.h"
#include "stackgraph/graph.h"

class StackApp : public ConfApp {

    std::string logfile;
    std::string graph_title;
    std::string total_label;

    std::list<StackNode*> bars;
    std::map<StackNode*, float> bar_amount;

    bool bar_amounts_done;

    float totalamount;
    float unit;

    StackNode* chart;

    vec3f bar_colour;
    float graph_elapsed;

    void addBar(std::string title, float init_value, float delta_value);

    bool updateGraph();
public:
    StackApp(std::string conffile);
    void init();
    void logic(float dt);
    void draw();
};

#endif
