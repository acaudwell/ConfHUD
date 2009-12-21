#include "coast.h"

LCACoast::LCACoast() : SDLApp() {
    footer = texturemanager.grab("footer.jpg");

    //make footer repeat horizontally
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

}

LCACoast::~LCACoast() {
}

void LCACoast::drawBackground() {

    float footer_start_y = display.height - footer->h;
    float footer_w_ratio = ((float) display.width / footer->w) * 0.75;

    glDisable(GL_TEXTURE_2D);

    float bar1 = display.height * 0.15;
    float bar2 = display.height * 0.30;
    float bar3 = display.height * 0.50;
    float bar4 = display.height * 0.75;

    glBegin(GL_QUADS);


        glColor3f(0.17, 0.47, 0.76);

        //quad1

        glVertex2f(0.0f, 0.0f);
        glVertex2f(display.width, 0.0f);

        glColor3f(0.66, 0.80, 0.91);

        glVertex2f(display.width, bar1);
        glVertex2f(0.0f,          bar1);

        //quad2

        glVertex2f(0.0f,          bar1);
        glVertex2f(display.width, bar1);

        glColor3f(0.93, 0.96, 0.98);

        glVertex2f(display.width, bar2);
        glVertex2f(0.0f,          bar2);

        //quad3

        glVertex2f(0.0f,          bar2);
        glVertex2f(display.width, bar2);

        glColor3f(0.85, 0.92, 0.95);

        glVertex2f(display.width, bar3);
        glVertex2f(0.0f,          bar3);

        //quad4

        glVertex2f(0.0f,          bar3);
        glVertex2f(display.width, bar3);

        glColor3f(0.97, 0.98, 0.99);

        glVertex2f(display.width, bar4);
        glVertex2f(0.0f,          bar4);


        glColor3f(0.97, 0.98, 0.99);

        //quad5

        glVertex2f(0.0f,          bar4);
        glVertex2f(display.width, bar4);

        glColor3f(1.0, 1.0, 1.0);

        glVertex2f(display.width, footer_start_y);
        glVertex2f(0.0f,          footer_start_y);
    glEnd();

    glEnable(GL_TEXTURE_2D);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glBindTexture(GL_TEXTURE_2D, footer->textureid);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, footer_start_y);

        glTexCoord2f(footer_w_ratio, 0.0f);
        glVertex2f(display.width, footer_start_y);

        glTexCoord2f(footer_w_ratio, 1.0f);
        glVertex2f(display.width, display.height);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, display.height);
    glEnd();
}

void LCACoast::draw(float t, float dt) {
    drawBackground();
}
