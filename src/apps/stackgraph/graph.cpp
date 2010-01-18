#include "graph.h"

float gStackGraphFetchDelay     = 0.5;
bool  gStackGraphDrawBackground = true;

StackGraph::StackGraph() {

    vertical = false;

    in_transition = 0.0;

    border_colour = vec3f(0.5, 0.5, 0.5);

    debug = false;

    // todo.. need a resource dir variable.. and put fonts under data/fonts/
    titlefont = fontmanager.grab("FreeSans.ttf", 18);
    titlefont.dropShadow(true);
    titlefont.shadowStrength(gFontShadow);

    timefont  = fontmanager.grab("FreeSans.ttf", 14);
    timefont.dropShadow(true);
    timefont.shadowStrength(gFontShadow);

    keyfont  = fontmanager.grab("FreeSans.ttf", 14);
    keyfont.dropShadow(true);
    keyfont.shadowStrength(gFontShadow);

    theme_black = false;

    activeNode = 0;

    resetView();
}

StackGraph::~StackGraph() {
    if(activeNode!=0) delete activeNode;
}

void StackGraph::setColourTheme() {

    if(theme_black) {
        clearcolour = vec3f(0.0, 0.0, 0.0);
        gTextBaseColour = vec3f(1.0, 1.0, 1.0);
    } else {
        clearcolour = vec3f(1.0, 1.0, 1.0);
        gTextBaseColour = vec3f(0.25, 0.25, 0.25);
    }
}

bool StackGraph::isBlackTheme() {
    return theme_black;
}

void StackGraph::resetView() {

    setColourTheme();

    scale_factor         = 0.0;
    current_scale_factor = 0.0;

    if(isVertical()) {
        margin = 55.0;
        corner = vec2f(display.width * 0.25, display.height * 0.125);

        width  = display.width * 0.667;
        height = display.height * 0.75;
    } else {
        margin = 0.0;
        corner = vec2f(display.width * 0.125, display.height * 0.125);

        width  = display.width * 0.75;
        height = display.height * 0.5;
    }

    section_bounds = Bounds2D(vec2f(-1.0, -30.0), vec2f(width+1.0, 0.0));

    gMaxBarWidth  = getBreadth() / 30.0;
    gMinBetHeight = getLength() / 50.0;
    gMinBarHeight = 0.0;

    paused=false;
    mousemoved=false;
    mouseclicked=false;

    mouseOverNode = 0;
    selectedNode = 0;

    last_update = 0.0;

    alpha = 0.0;

    if(activeNode!=0) activeNode->positionChildren();
}

void StackGraph::init() {
}

void StackGraph::updateGraph(StackNode* node) {
    resetView();

    if(activeNode != 0) delete activeNode;
    activeNode = 0;

    setActiveNode(node);
}

void StackGraph::setActiveNode(StackNode* node) {

    if(activeNode != 0) activeNode->onBlur();

    activeNode = node;

//    activeNode->onFocus();
}

void StackGraph::update(float t, float dt) {
    logic(t, dt);
    draw(t, dt);
}

void StackGraph::mouseMove(SDL_MouseMotionEvent *e) {
    mousepos = vec2f(e->x, e->y);
    mousemoved=true;
}

void StackGraph::mouseClick(SDL_MouseButtonEvent *e) {
    if(e->type != SDL_MOUSEBUTTONDOWN) return;

    mousepos = vec2f(e->x, e->y);

    mouseclicked = true;
}

void StackGraph::toggleVertical() {
    vertical = !vertical;
    resetView();
}

void StackGraph::keyPress(SDL_KeyboardEvent *e) {
    if (e->type == SDL_KEYUP) return;

    if (e->type == SDL_KEYDOWN) {
        if (e->keysym.sym == SDLK_ESCAPE) {
            appFinished=true;
        }

        if (e->keysym.sym == SDLK_SPACE) {
            paused = !paused;
        }

        if (e->keysym.sym == SDLK_c) {
            theme_black = !theme_black;
            resetView();
        }

        if (e->keysym.sym == SDLK_v) {
            toggleVertical();
        }

        if (e->keysym.sym == SDLK_q) {
            debug = !debug;
        }
    }
}

FXFont StackGraph::getTitleFont() {
    return titlefont;
}

FXFont StackGraph::getTimeFont() {
    return timefont;
}

FXFont StackGraph::getKeyFont() {
    return keyfont;
}

float StackGraph::getMargin() {
    return margin;
}

vec2f StackGraph::getCorner() {
    return corner;
}

float StackGraph::getAlpha() {
    return alpha;
}

float StackGraph::getWidth() {
    return width;
}

bool StackGraph::isVertical() {
    return vertical;
}

float StackGraph::getBreadth() {
    return (vertical) ? height : width;
}

float StackGraph::getLength() {
    return (vertical) ? width : height;
}

float StackGraph::getHeight() {
    return height;
}

float StackGraph::getScaleFactor() {
    return scale_factor;
}

float StackGraph::getCurrentScaleFactor() {
    return current_scale_factor;
}

void StackGraph::setBackgroundColour(vec3f background) {
    clearcolour = background;
}

std::string StackGraph::dollarAmount(double num) {

    char suffix = 0;

    if(num >= 1000000) {
        num /= 1000000.0;
        suffix = 'M';
    }
    else if(num >= 1000) {
        num /= 1000.0;
        suffix = 'K';
    }

    char buff[256];

    if(suffix == 0) {
        snprintf(buff, 256, "$%.0f", num);
    } else if (int(num) == num) {
        snprintf(buff, 256, "$%.0f%c", num, suffix);
    } else {
        snprintf(buff, 256, "$%.1f%c", num, suffix);
    }

    return std::string(buff);
}

void StackGraph::selectNode(StackNode* node) {
    if(in_transition > 0.0) return;

    //cant select empty nodes
    if(node->getChildCount()==0) return;

    selectedNode = node;
    selected_bounds = node->getBounds();
    in_transition = 1.0;
}

void StackGraph::updateTransition(float dt) {

    in_transition -= dt;

    alpha = std::max(0.0f, alpha - dt);

    // animate transition_bar

    if(in_transition<=0.0) {
        setActiveNode(selectedNode);
        in_transition  = 0.0;

        resetView();
    }
}

void StackGraph::updateScale(float dt) {

    if(activeNode==0) return;

    //update active nodes children
    float max_value = activeNode->updateChildren(dt);

    //scale up if we passed scale
    if(max_value > scale_factor || (max_value < scale_factor * 0.5) || (max_value < scale_factor * 0.25)) {
        //find next power
        float next;

        for(next = 1.0; next <= max_value; next*=10.0);

        //if quarter of next is greater than value, use that
        if(next*0.25>max_value) {
            next *= 0.25;
        }
        //if half next is greater than value, use that
        else if(next*0.5f>max_value) {
            next *= 0.5;
        }

        scale_factor = next;
    }

    //move current scale to wards scale
    current_scale_factor += (scale_factor-current_scale_factor) * dt * 3.0;
}

void StackGraph::logic(float t, float dt) {

    if(paused) return;

    if(activeNode==0) return;

    StackNode* clickedNode = mouseOverNode = 0;

    StackNode* nodeAtMouse = activeNode->mouseOverChildren(mousepos);

    if(nodeAtMouse != 0 && !nodeAtMouse->isLoading()) {
        mouseOverNode = nodeAtMouse;
        if(mouseclicked) clickedNode = nodeAtMouse;
    }

    if(clickedNode != 0) {

        selectNode(clickedNode);

    } else if(mouseclicked) {

        vec2f barclick = mousepos - corner;

        //select parent node
        if(section_bounds.contains(barclick)) {
            StackNode* parent = activeNode->getParent();

            if(parent != 0) {
                setActiveNode(parent);
                resetView();
            }
        }
    }

    if(in_transition>0.0) {
        updateTransition(dt);
    }
    else if(alpha < 1.0) {
        alpha = std::min(1.0f, alpha + dt);
    }

    if(last_update > gStackGraphFetchDelay) {
        last_update = 0.0;
//        activeNode->fetchBets();
    }

    last_update += dt;

    //update the scale of the graph
    updateScale(dt);

    //update active node bets
    activeNode->updateBets(dt);

    //time
    char datestr[256];
    long currtime = time(0);
    struct tm* timeinfo = localtime ( &currtime );
    strftime(datestr, 256, "%d/%m/%Y %X", timeinfo);

    displaydate = datestr;

    mouseclicked = false;
}

void StackGraph::drawTime() {
    glEnable(GL_TEXTURE_2D);
    glColor4f(gTextBaseColour.x, gTextBaseColour.y, gTextBaseColour.z, 1.0);

    int text_width = timefont.getWidth(displaydate);

    timefont.print(getWidth() - 5.0 - text_width, 5.0, "%s", displaydate.c_str());
}

void StackGraph::drawScaleKey() {
    glEnable(GL_TEXTURE_2D);

    glColor4f(gTextBaseColour.x, gTextBaseColour.y, gTextBaseColour.z, 1.0);

    std::string max_dollar_value = dollarAmount(scale_factor);

    int text_width = keyfont.getWidth(max_dollar_value);

    if(vertical) {
        keyfont.print(getWidth(), getHeight() + 10.0, "%s", max_dollar_value.c_str());
    } else {
        keyfont.print(0.0 - 10.0 - text_width, -10.0, "%s", max_dollar_value.c_str());
    }
}

void StackGraph::drawTransition(float dt) {
    Bounds2D moving_bounds;

    moving_bounds.min = selected_bounds.min * in_transition + section_bounds.min * (1.0 - in_transition);
    moving_bounds.max = selected_bounds.max * in_transition + section_bounds.max * (1.0 - in_transition);

    vec3f moving_colour = vec3f(1.0, 1.0, 1.0);
    vec3f moving_colour2 = moving_colour * 0.3;


    glBegin(GL_QUADS);
        glColor3fv(moving_colour);
        glVertex2f(moving_bounds.min.x, moving_bounds.min.y);
        glVertex2f(moving_bounds.max.x, moving_bounds.min.y);

        glColor3fv(moving_colour2);

        glVertex2f(moving_bounds.max.x, moving_bounds.max.y);
        glVertex2f(moving_bounds.min.x, moving_bounds.max.y);
    glEnd();
}

void StackGraph::draw(float t, float dt) {
    if(activeNode==0) return;

    if(gStackGraphDrawBackground) {
        display.setClearColour(clearcolour);
        display.clear();
    }

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    display.mode2D();

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);

    glLineWidth(2.0);

    glPushMatrix();

    glTranslatef(corner.x, corner.y, 0.0f);

    activeNode->drawContent(dt);

    //draw scale
    drawScaleKey();

    glDisable(GL_TEXTURE_2D);

    glColor3fv(border_colour);

    //border
    glBegin(GL_LINE_STRIP);
        glVertex2f(0.0, 0.0);
        glVertex2f(0.0, height);
        glVertex2f(width, height);
        glVertex2f(width, 0.0);
    glEnd();

    //draw section bar
    vec3f section_colour = vec3f(1.0, 1.0, 1.0); //activeNode->getColour();
    vec3f section_colour2 = section_colour * 0.3;

    glBegin(GL_QUADS);
            glColor3fv(section_colour);
            glVertex2f(section_bounds.min.x, section_bounds.min.y);
            glVertex2f(section_bounds.max.x, section_bounds.min.y);

            glColor3fv(section_colour2);

            glVertex2f(section_bounds.max.x, section_bounds.max.y);
            glVertex2f(section_bounds.min.x, section_bounds.max.y);
    glEnd();

/*
    glColor3fv(border_colour);

    glBegin(GL_LINE_STRIP);
            glVertex2f(section_bounds.min.x, section_bounds.min.y);
            glVertex2f(section_bounds.min.x, section_bounds.max.y);
            glVertex2f(section_bounds.max.x, section_bounds.max.y);
            glVertex2f(section_bounds.max.x, section_bounds.min.y);
    glEnd();
*/

    //transition bar
    if(in_transition > 0.0) {
        drawTransition(dt);
    }

    if(mouseOverNode != 0 && in_transition <= 0.0) {
        mouseOverNode->drawBarToolTip(1.0);
    }

    activeNode->drawChildToolTips();

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    //section title
    std::string section_title = activeNode->getTitle();

    int title_width = titlefont.getWidth(section_title);
    int title_height = titlefont.getHeight();

    titlefont.print(section_bounds.min.x + width*0.5 - title_width*0.5, section_bounds.min.y + section_bounds.height()*0.5 - title_height*0.5, "%s", section_title.c_str());

    //time
    //drawTime();

    // UI
    glPopMatrix();

    display.mode2D();

    std::string max_dollar_value = dollarAmount(scale_factor);

    if(debug) {
        keyfont.print(0,0, "FPS: %.2f", fps);
        keyfont.print(0,20, "Scale Factor: %.2f - %s (%.2f)", scale_factor, max_dollar_value.c_str(),  current_scale_factor);
    }

//    ftglfont->Render("Hello World");

    glDisable(GL_TEXTURE_2D);
}

