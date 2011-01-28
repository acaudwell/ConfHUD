#include "stackapp.h"

Regex bar_regex("^([^|]*)\\|([^|]*)\\|(.*)$");

StackApp::StackApp(std::string conffile) : ConfApp(conffile) {
    max_tick_rate = 1.0 / 60.0;
    graph_elapsed = 0.0;
    chart = 0;
    unit = 5.0;
    graph_title;
    totalamount = 0.0;
    total_label = std::string("Total");

    bar_amounts_done = false;
}

void StackApp::addBar(std::string title, float init_value, float delta_value) {

    StackNode* newnode = new StackNode((StackGraph*) app, title, init_value, bar_colour);

    bar_amount[newnode] = delta_value;
    bars.push_back(newnode);

    chart->addChild(newnode);
}

bool StackApp::updateGraph() {
    if(!logfile.size()) return false;

    //read amounts
    std::ifstream in(logfile.c_str());
    if(!in.is_open()) return false;

    this->chart = new StackNode((StackGraph*) app, graph_title);

    std::string line;

    bool firstline = true;

    while(std::getline(in, line)) {

        if(firstline) {
            totalamount = atof(line.c_str());
            firstline=false;
            continue;
        }

        std::vector<std::string> entry;

        if(bar_regex.match(line, &entry)) {
            std::string bar_name = entry[0];
            float init_value       = atof(entry[1].c_str());
            float delta_value      = atof(entry[2].c_str());

            addBar(bar_name, init_value, delta_value);
        }
    }

    in.close();

    ((StackGraph*)app)->updateGraph(chart);

    if(bars.size()==0) return false;

    return true;
}

void StackApp::logic(float dt) {
    graph_elapsed += dt;

    if(!bar_amounts_done && graph_elapsed > 1.0) {

        float total_remaining = 0.0;

        //add delta amounts for each bar
        for(std::list<StackNode*>::iterator it = bars.begin(); it != bars.end(); it++) {

            StackNode* node = (*it);

            float remaining = bar_amount[node];

            total_remaining += remaining;

            if(node->isLoading()) continue;

//            printf("%s, remaining = %.2f\n", node->getTitle().c_str(), remaining);

            if(remaining>0.0) {

                float d = floor(remaining*0.667);

                if(d<=1.0) {

                    //avoid $0 appearing
                    if(remaining>=1.0) chart->addBet(node, remaining);

                    bar_amount[node] = 0.0;

                } else {
                    chart->addBet(node, d);

                    remaining -= d;
                    bar_amount[node] = remaining;
                }
            }

        }

        if(total_remaining<=0.0) {
            chart->onFocus();
            bar_amounts_done = true;
        }

        graph_elapsed = 0.0;
    }

    ConfApp::logic(dt);
}

void StackApp::draw() {

    ConfApp::draw();

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    FXFont font = fontmanager.grab("FreeSans.ttf", 28);

    font.dropShadow(true);
    font.alignTop(false);
    font.alignRight(true);

    glColor4f(colour_title.x, colour_title.y, colour_title.z, 1.0);
//    glColor4f(0.0, 0.0, 0.0, 1.0);

    font.print(display.width-10, display.height-45, "%s: $%d", total_label.c_str(), (int) totalamount);
}

void StackApp::init() {

    std::string value;

    graph_title = config.getString("stackgraph", "title");

    total_label = config.getString("stackgraph", "total_label");

    logfile    = config.getString("stackgraph", "logfile");
    bar_colour = config.getVec3("stackgraph", "bar_colour");

    float unit = config.getFloat("stackgraph", "unit");

    if(unit > 0.0) {
        this->unit = unit;
    }

    gStackGraphDrawBackground = false;

    //create StackGraph object
    StackGraph* g = new StackGraph();

    g->init();

    app = g;

    if(updateGraph()) {
        ready=true;
    }
}
