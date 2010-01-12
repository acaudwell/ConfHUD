#include "bet.h"
#include "graph.h"

//StackBet

float gMinBetHeight = 0.0;

StackBet::StackBet(StackNode* node, float amount) {
    this->node    = node;
    this->amount = amount;

    this->max_speed = 500.0;

    speed = ((amount/(node->getGraph()->getCurrentScaleFactor()+1.0)) * max_speed) + 10;

    debugLog("amount = %.2f, speed = %.2f\n", amount, speed);

    speed = std::min(speed, max_speed);

    this->dollar_amount = node->getGraph()->dollarAmount(amount);

    y = 0.0;
    colour = vec3f(1.0, 1.0, 1.0);
    alpha = 1.0;
    amount_elapsed = 0.0;
    applied = false;
    reached = false;
    finished = false;
}

void StackBet::apply() {
    node->addValue(amount);
    applied = true;
}

bool StackBet::reachedBar() {
    return reached;
}

bool StackBet::hasApplied() {
    return applied;
}

bool StackBet::isFinished() {
    return finished;
}

void StackBet::logic(float dt) {

    StackGraph* graph = node->getGraph();

    float desty = node->getTop();

    if(!reached) {
        speed += speed*2.0*dt;
        speed = std::min(speed, max_speed);

        y = std::min( y + speed*dt, desty);
        alpha = 1.0;

        if(y>= desty) {
            y = desty;
            reached = true;
        }
    }

    if(reached) {
        alpha          = std::max(0.0f, alpha-dt);
        amount_elapsed = std::min(1.0f, amount_elapsed+dt*0.2f);
    }

    if(amount_elapsed >= 1.0f) {
        finished = true;
    }
}

float StackBet::getHeight() {
    StackGraph* graph = node->getGraph();
    float scale_factor  = graph->getCurrentScaleFactor();
    float graph_length  = graph->getLength();

    float scaled_amount = (amount/scale_factor) * graph_length;

    return std::max(gMinBetHeight, scaled_amount);
}

void StackBet::drawDollarAmount(float dt) {
    if(!reachedBar()) return;

    glEnable(GL_TEXTURE_2D);

    float draw_pos_y  = std::min(1.0, amount_elapsed * 2.0);
    float draw_alpha  = 1.0 - fabs((draw_pos_y - 0.5) * 2.0);

    StackGraph* graph = node->getGraph();
    float scale_factor  = graph->getCurrentScaleFactor();
    float graph_height  = graph->getHeight();
    float graph_width   = graph->getWidth();
    float graph_alpha   = graph->getAlpha();

    float bet_height = getHeight();

    FXFont font = graph->getTitleFont();

    vec3f text_colour = graph->isBlackTheme() ? gTextBaseColour : node->barColour();

    glColor4f(text_colour.x, text_colour.y, text_colour.z, draw_alpha*graph_alpha);

    int textwidth  = font.getWidth(dollar_amount);
    int textheight = font.getHeight();

    if(graph->isVertical()) {
        font.draw(graph_width - y - textwidth*0.5, node->getLeft() + node->getBarWidth() * 0.5 - 10.0f - 30.0f * draw_pos_y, dollar_amount.c_str());
    } else {
        font.draw(node->getLeft() + node->getBarWidth()*0.5 - textwidth*0.5 , node->getTop() - textheight -  30.0f * draw_pos_y, dollar_amount.c_str());
    }
}

void StackBet::draw(float dt) {
    StackGraph* graph = node->getGraph();

    float graph_alpha = graph->getAlpha();

    float bar_width    = node->getBarWidth();
    float graph_length = graph->getLength();

    float bet_height = getHeight();

    if(y-bet_height < 0.0) {
        bet_height = y;
    }

    vec3f node_colour = node->barColour();

    vec3f node_colour2 = node_colour;

    if(graph->isVertical()) {
        glPushMatrix();
            glTranslatef(graph_length-y, node->getLeft(), 0.0f);
            glBegin(GL_QUADS);
                glColor4f(node_colour.x, node_colour.y, node_colour.z, alpha*graph_alpha);

                glVertex2f(0.0f, 0.0f);
                glVertex2f(0.0, bar_width);

                glColor4f(node_colour2.x, node_colour2.y, node_colour2.z, 0.3*alpha*graph_alpha);

                glVertex2f(bet_height, bar_width);
                glVertex2f(bet_height, 0.0);
            glEnd();
        glPopMatrix();
    } else {
        glPushMatrix();
            glTranslatef(node->getLeft(), y, 0.0f);
            glBegin(GL_QUADS);
                glColor4f(node_colour.x, node_colour.y, node_colour.z, alpha*graph_alpha);

                glVertex2f(0.0f, 0.0f);
                glVertex2f(bar_width, 0.0f);

                glColor4f(node_colour2.x, node_colour2.y, node_colour2.z, 0.3*alpha*graph_alpha);

                glVertex2f(bar_width, -bet_height);
                glVertex2f(0.0, -bet_height);
            glEnd();
        glPopMatrix();
    }
}
