#include "node.h"
#include "bet.h"
#include "graph.h"

float gMaxBarWidth  = 0.0;
float gMinBarHeight = 0.0;

float gFontShadow = 0.5;

vec3f gTextBaseColour = vec3f(1.0, 1.0, 1.0);

//StackNode

Regex readNodeDetail("^([0-9]+)\\|([^\\|]+)\\|([0-9\\.]+)");

StackNode::StackNode(StackGraph* graph, std::string title, float value, vec3f colour) {
    this->parent = 0;
    this->graph = graph;

    debugLog("title = %s, value = %.2f\n", title.c_str(), value);

    this->title = title;
    this->x = 0.0;
    this->modified = 0.0;

    this->tooltip_alpha = 0.0;
    this->title_alpha = 0.0;
    this->title_size = 16;

    recent_mouse_over = 0.0;

    this->value = value;
    current_value = 0.0;

    alpha = 0.0;

    setColour(colour);
}

StackNode::~StackNode() {

    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;
        delete node;
    }

    for(std::list<StackBet*>::iterator it = bets.begin(); it != bets.end(); it++) {
        StackBet* bet = *it;
        delete bet;
    }
}

void StackNode::setParent(StackNode* parent) {
    this->parent = parent;
}

void StackNode::addChild(StackNode* child) {
    value += child->getValue();

    child->setParent(this);

    children.push_back(child);

    positionChildren();
}

//position children
void StackNode::positionChildren() {
    if(children.size()==0) return;

    float barwidth  = getChildBarWidths();
//    float start_x = std::max(0.0, (graph->getBreadth() - barwidth * children.size()) * 0.5);

    float graph_margin = graph->getMargin();
    float bar_gap = std::max(0.0f, (graph->getBreadth() - graph_margin - barwidth * children.size())/(children.size()+1));

    float current_x = bar_gap + graph_margin;

    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;

        node->setX(current_x);

        current_x += barwidth;
        current_x += bar_gap;
    }


//    debugLog("start_x = %.2f\n", start_x);

    int   title_size = 16;
    float text_offset = 20.0;

    if(1 || current_x <= 1.0) {
        title_size = 16;
        text_offset = 15.0;
    }

    //title sizes
    int i = 0;

    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;

        node->setTitleSize(title_size);
    }


}

int StackNode::getChildCount() {
    return children.size();
}

StackGraph* StackNode::getGraph() {
    return graph;
}

StackNode* StackNode::getParent() {
    return parent;
}

float StackNode::getTop() {
    float graph_length = graph->getLength();
    float scale_factor = graph->getCurrentScaleFactor();

    if(scale_factor<=0.0) return 0.0;

    return std::min(graph_length - gMinBarHeight, std::max(0.0f, graph_length - (value/scale_factor) * graph_length));
}

float StackNode::getChildBarWidths() {
    if(children.size()==0) return 0;

    return std::min(gMaxBarWidth, graph->getBreadth() / (float) children.size());
}

float StackNode::getBarWidth() {
    if(parent==0 || parent->getChildCount()==0) return 0;

    return parent->getChildBarWidths();
}

float StackNode::getLeft() {
	return x;
}

float StackNode::getValue() {
    return value;
}

void StackNode::setX(float x) {
    this->x = x;
}

void StackNode::setValue(float value) {
    this->value = value;
}

void StackNode::addValue(float delta) {
    this->modified = std::min(1.0f, delta / this->value);
    this->value += delta;
}

bool StackNode::isLoading() {
    return current_value < value;
}

void StackNode::onBlur() {
}

void StackNode::onFocus() {

    title_alpha = 0.0;

    //highlight the min and max nodes
    if(children.size() == 0) return;

    StackNode* min_node = children[0];
    StackNode* max_node = children[0];

    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;

        if(node->getValue() > max_node->getValue()) {
            max_node = node;
        }

        if(node->getValue() < min_node->getValue()) {
            min_node = node;
        }
    }

    // show tips
//    min_node->showToolTip();
    if(min_node!=max_node) max_node->showToolTip();

}

StackNode* StackNode::mouseOverChildren(vec2f pos) {

    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;

        if(node->mouseOver(pos)) {
            return node;
        }
    }

    return 0;
}
bool StackNode::mouseOver(vec2f pos) {

    pos -= graph->getCorner();

    float bar_width = getBarWidth();

    float scale_factor = graph->getCurrentScaleFactor();
    float graph_width  = graph->getWidth();
    float graph_height = graph->getHeight();

    float top = getTop();

    float title_width = graph->getTitleFont().getWidth(title);

    if(graph->isVertical()) {
        if(   pos.y >= x && pos.y <= x + bar_width
        && pos.x >= (-5.0 - title_width) && pos.x <= graph_width-top) {
            recent_mouse_over = 1.0;
            return true;
        }
    } else {
        if(   pos.x >= x && pos.x <= x + bar_width
        && pos.y >= top && pos.y <= graph_height + 10.0 + title_width) {
            recent_mouse_over = 1.0;
            return true;
        }
    }

    return false;
}

//simulate betting activity
void StackNode::fetchBets() {

    // TODO: check if our value > total of the children?

    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {

        StackNode* node = *it;

        if(rand() % 100 < 90) continue;
        if(node->isLoading()) continue;

        int max_value = node->getValue() * 0.1f + 10.0f;

        StackBet* bet = new StackBet(node, (float) (rand() % max_value) + 1);
        bets.push_back(bet);
    }
}

void StackNode::setTitleSize(int size) {
    this->title_size = size;
}

float StackNode::updateChildren(float dt) {

    if(title_alpha<1.0) title_alpha = std::min(1.0f, title_alpha+dt);

    float max_value = 0.0;
    float total_value = 0.0;

    StackNode* max_node = 0;

    //update bars
    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;
        node->logic(dt);

        float childval = node->getValue();

        total_value += childval;

        if(childval > max_value) {
            max_value = childval;
            max_node  = node;
        }
    }

    //set node value to sum of children
    this->setValue(total_value);

    return max_value;
}

//update bets
void StackNode::updateBets(float dt) {

    for(std::list<StackBet*>::iterator it = bets.begin(); it != bets.end();) {
        StackBet* bet = *it;
        bet->logic(dt);

        if(bet->reachedBar() && !bet->hasApplied()) {
            bet->apply();
        }

        //delete if finished
        if(bet->isFinished()) {
            it = bets.erase(it);
            delete bet;
        } else {
            it++;
        }
    }
}

void StackNode::showToolTip() {
    tooltip_alpha = 10.0;
}

void StackNode::logic(float dt) {

    if(value > current_value) {
        current_value = std::min(current_value + std::max( 50.0f, value )*dt, value);
    }

    if(alpha < 1.0) {
        alpha = std::min(1.0f, alpha + dt*0.25f);
    }

    if(recent_mouse_over>0.0) recent_mouse_over -= dt*3.0;
    if(modified>0.0) modified -= dt;

    if(tooltip_alpha>0.0) tooltip_alpha -= dt;
}

void StackNode::setColour(vec3f colour) {
    this->colour = colour;
}

vec3f StackNode::getColour() {
    return colour;
}

vec3f StackNode::barColour() {

    float scale_factor = graph->getCurrentScaleFactor();

    float l = std::min(1.0f, current_value / scale_factor);

    vec3f col = getColour();// * l + vec3f(0.3f, 0.3, 0.3f) * (1.0-l) ;//getColour();

    if(recent_mouse_over > 0.0) {
        col = vec3f(0.5, 0.5, 0.5) * recent_mouse_over + col * (1.0 - recent_mouse_over);
    }

    if(modified>0.0) {
        col = col * (1.0-modified) + gTextBaseColour * modified;
    }

    return col;
}

std::string StackNode::getTitle() {
    return title;
}

Bounds2D StackNode::getBounds() {

    Bounds2D bounds;

    float scale_factor = graph->getCurrentScaleFactor();
    float graph_length = graph->getLength();
    float bar_width = getBarWidth();

    float top =  getTop();

    if(graph->isVertical()) {
        bounds.min = vec2f(0.0, getLeft());
        bounds.max = vec2f(graph_length-top,getLeft()+bar_width);
    } else {
        bounds.min = vec2f(getLeft(), top);
        bounds.max = vec2f(getLeft()+bar_width, graph_length);
    }

    return bounds;
}

void StackNode::drawContent(float dt) {

    //draw bars
    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;
        node->drawBar(dt);
    }

    //draw bets
    for(std::list<StackBet*>::iterator it = bets.begin(); it != bets.end(); it++) {
        StackBet* bet = *it;
        bet->draw(dt);
    }

    //draw amounts of bets that have finished falling
    for(std::list<StackBet*>::iterator it = bets.begin(); it != bets.end(); it++) {
        StackBet* bet = *it;

        if(bet->reachedBar()) {
            bet->drawDollarAmount(dt);
        }
    }

    glEnable(GL_TEXTURE_2D);
}

void StackNode::drawBarToolTip(float tip_alpha) {

    tip_alpha = std::min(1.0f, tip_alpha) * graph->getAlpha();

    glDisable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);

    glLineWidth(2.0);

    float left = getLeft();
    float top  = getTop();

    if(top<1.0) return;

    float bar_width = getBarWidth();

    //draw amount
    FXFont keyfont = graph->getKeyFont();

    std::string bar_amount = graph->dollarAmount(getValue());
    std::string bar_title  = getTitle();

    int font_offset = keyfont.getWidth(bar_amount);

    vec2f line_start;
    vec2f line_end;

    if(graph->isVertical()) {
        line_start = vec2f(graph->getWidth()-top-0.5, left+0.5);
        line_end   = vec2f(graph->getWidth()-top-0.5, graph->getHeight());

        line_end.y   += 10.0;
    } else {
        line_start = vec2f(left+0.5, top+0.5);
        line_end   = vec2f(0.0, top+0.5);

        line_start.x += bar_width - 1.0;
        //line_end.x   -= 10.0;
    }

    //draw line
    vec3f col = gTextBaseColour;

    glColor4f(col.x, col.y, col.z, tip_alpha * 0.5);

    glBegin(GL_LINES);
        glVertex2fv(line_start);
        glVertex2fv(line_end);
    glEnd();

    glEnable(GL_TEXTURE_2D);

    vec3f title_col = gTextBaseColour;

    FXFont titlefont = graph->getTitleFont();

    int title_width = titlefont.getWidth(bar_title);

    glColor4f(title_col.x,title_col.y,title_col.z, tip_alpha);

    //draw text
    if(graph->isVertical()) {
        int text_width = keyfont.getWidth(bar_amount);

        keyfont.draw(line_end.x - text_width/2, line_end.y, bar_amount);

        titlefont.draw(line_end.x + 5.0f, line_start.y + (line_end.y-line_start.y) * 0.5 - 10.0f, bar_title);
    } else {
        keyfont.draw(line_end.x - font_offset - 10.0, line_end.y - 10.0, bar_amount);
        titlefont.alignTop(false);
        titlefont.draw(line_end.x + (line_start.x-line_end.x) * 0.5 - title_width * 0.5, line_end.y - 5.0, bar_title);
    }

    glDisable(GL_TEXTURE_2D);
}

void StackNode::drawTitle(float alpha) {

    alpha *= graph->getAlpha();

    glEnable(GL_TEXTURE_2D);

    float left = getLeft();
    float top  = getTop();
    float bar_width = getBarWidth();

    vec3f unselected_colour = gTextBaseColour;

    vec3f title_col = unselected_colour * (1.0-recent_mouse_over) + colour * recent_mouse_over;

    glColor4f(title_col.x,title_col.y,title_col.z, alpha);

    FXFont titlefont = graph->getTitleFont();

    std::string bar_title = getTitle();
    int title_width = titlefont.getWidth(bar_title);

    float font_height = titlefont.getHeight();

    if(graph->isVertical()) {
            titlefont.draw(-10.0 - title_width, x + bar_width/2 - font_height/2, bar_title.c_str());
    } else {
        glPushMatrix();
            glTranslatef(left + bar_width/2 + font_height/2, graph->getHeight() + 10.0, 0.0);
            glRotatef(90.0f, 0.0, 0.0, 1.0);
            titlefont.draw(0.0, 0.0, bar_title.c_str());
        glPopMatrix();
    }

    glDisable(GL_TEXTURE_2D);
}

void StackNode::drawToolTip() {
    if(tooltip_alpha>0.0) {
        drawBarToolTip(tooltip_alpha);
    }
}

void StackNode::drawChildToolTips() {

    for(std::vector<StackNode*>::iterator it = children.begin(); it != children.end(); it++) {
        StackNode* node = *it;
        node->drawToolTip();
        if(title_alpha>0.0) node->drawTitle(title_alpha);
    }

}

void StackNode::drawBar(float dt) {

    vec3f barcol = barColour();

    float graph_alpha = graph->getAlpha();

    debugLog("title = %s, alpha = %.2f, graph_alpha = %.2f\n", title.c_str(), alpha, graph_alpha);

    float bar_width    = getBarWidth();
    float graph_length = graph->getLength();

    float bar_height = current_value;

    float scale_factor = graph->getCurrentScaleFactor();

    float scaled_bar_length = graph_length - getTop();

    debugLog("bar_width = %.2f,  graph_length = %.2f, bar_height = %.2f\n", bar_width, graph_length, bar_height);

//  debugLog("x = %.2f, height = %.2f, bar_width = %.2f, current value = %.2f, alpha = %.2f\n", x, graph_height, bar_width, current_value, alpha);

//    vec3f srcol = vec3f(0.2,0.2,0.2);//barcol * 0.3;
    vec3f srcol = barcol;

    float bar_fade = graph->isBlackTheme() ? 0.5 : 0.8;

    srcol *= bar_fade;

    if(graph->isVertical()) {
        glPushMatrix();
            glTranslatef(0.0, x, 0.0f);
            glBegin(GL_QUADS);
                glColor4f(srcol.x, srcol.y, srcol.z, alpha*graph_alpha);

                glVertex2f(0.0f, 0.0f);
                glVertex2f(0.0, bar_width);

                glColor4f(barcol.x, barcol.y, barcol.z, alpha*graph_alpha);

                glVertex2f(scaled_bar_length, bar_width);
                glVertex2f(scaled_bar_length, 0.0);
            glEnd();
        glPopMatrix();
    } else {
        glPushMatrix();
            glTranslatef(x, graph_length,0.0f);
            glBegin(GL_QUADS);
                glColor4f(srcol.x, srcol.y, srcol.z, alpha*graph_alpha);

                glVertex2f(0.0f, 0.0f);
                glVertex2f(bar_width, 0.0f);

                glColor4f(barcol.x, barcol.y, barcol.z, alpha*graph_alpha);

                glVertex2f(bar_width, -scaled_bar_length);
                glVertex2f(0.0, -scaled_bar_length);
            glEnd();
        glPopMatrix();
    }
}
