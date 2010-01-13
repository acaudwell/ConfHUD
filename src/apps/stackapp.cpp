#include "stackapp.h"

StackApp::StackApp(std::string conffile) : ConfApp(conffile) {
    max_tick_rate = 1.0 / 60.0;
    graph_elapsed = 0.0;
}

void StackApp::addBar(StackNode* chart, std::string title, float value) {
    chart->addChild(new StackNode((StackGraph*) app, title, value, bar_colour));
}

void StackApp::updateGraph() {

    StackGraph* g = (StackGraph*) app;

    //TODO: read log, aggregate top 20, construct graph

    StackNode* chart = new StackNode(g, "Top 20 Life Flight Trust Donors");

    addBar(chart, "Andrew Caudwell", 5.0);
    addBar(chart, "John Smith", 5.0);

    g->updateGraph(chart);
}

void StackApp::logic(float dt) {
    graph_elapsed += dt;

    if(graph_elapsed > 30.0) {
        updateGraph();
        graph_elapsed = 0.0;
    }

    ConfApp::logic(dt);
}

void StackApp::init() {

    vec3f background;
    bool drawBackground = false;

    std::string value;

    logfile    = config.getString("stackgraph", "logfile");
    bar_colour = config.getVec3("stackgraph", "bar_colour");

    if(config.hasValue("stackgraph", "background")) {
        background = config.getVec3("stackgraph", "background");
        drawBackground = true;
    }

    gStackGraphDrawBackground  = drawBackground;

    //create StackGraph object
    StackGraph* g = new StackGraph();

    //set background colour
//    g->setBackgroundColour(background);

    g->init();

    app = g;

    updateGraph();

    ready=true;
}
