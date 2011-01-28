#include "slideapp.h"

//SlideShow

SlideShow::SlideShow(float duration, vec2f screen_ratio) : SDLApp() {
    this->duration = duration;
    this->screen_ratio = screen_ratio;
    
    image = 0;
    image_index = 0;
    elapsed = 0.0;
    alpha = 0.0;
}

SlideShow::~SlideShow() {
    if(image!=0) delete image;
}

void SlideShow::addImageFile(std::string imagefile) {
    images.push_back(imagefile);
}

void SlideShow::randomize() {
    std::random_shuffle(images.begin(), images.end());
}

bool SlideShow::loadNextImage() {
    if(images.size()==0) return false;

    //only one slide, already loaded
    if(image!=0 && images.size()==1) return true;

    //try and load next image
    std::string imagefile = images[image_index];

    try {
        TextureResource* newimage = new TextureResource(imagefile, 1, 1, 1, true);
        if(image!=0) delete image;
        image = newimage;
    } catch(TextureException& exception) {
        debugLog("failed to load slideshow image '%s'\n", exception.what());
        return false;
    }

    //increment index
    image_index = (image_index + 1) % images.size();

    return true;
}

void SlideShow::logic(float t, float dt) {

    dt = std::min(1.0f/60.0f, dt);

    if(image == 0 || alpha <= 0.0 && elapsed >= duration ) {

        //dont loop. if this is the last image, we will stop
        if(image != 0 && image_index==0) {
            appFinished=true;
            return;
        }

        bool success = loadNextImage();

        if(!success) {
            appFinished = true;
            return;
        }
        elapsed = 0.0;
    }

    if(elapsed >= duration) {
        alpha = std::max(0.0f, alpha - dt);
    } else {
        alpha = std::min(1.0f, alpha + dt);
        elapsed += dt;
    }

}

vec2f SlideShow::getAspectRatio() {

    vec2f aspectRatio;

    float sratiox = screen_ratio.x / screen_ratio.y;
    float sratioy = screen_ratio.y / screen_ratio.x;

    //calc aspect raio
    float ratiox = image->w / (float) image->h;
    float ratioy = image->h / (float) image->w;

    if(ratiox < 1.0) {
        if(ratiox > sratiox) {
            aspectRatio = vec2f( sratiox / ratiox, 1.0 );
        } else {
            aspectRatio = vec2f( ratiox / sratiox, 1.0 );
        }
    } else {
        if(ratioy > sratioy) {
            aspectRatio = vec2f( 1.0, sratioy / ratioy );
        } else {
            aspectRatio = vec2f( 1.0, ratioy / sratioy );
        }
    }

    return aspectRatio;
}

void SlideShow::draw(float t, float dt) {
    if(appFinished || image==0) return;

    display.mode2D();

    glPushMatrix();

    //glScalef(alpha, 1.0, 0.0);

    vec2f ratio = getAspectRatio();

    float max_percent = 1.0f;

    float scaled_w = display.width * ratio.x * max_percent;
    float scaled_h = display.height * ratio.y * max_percent;

    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    glTranslatef( (display.width - scaled_w) * 0.5f,
                  (display.height - scaled_h) * 0.5f,
                  0.0f);

    glDisable(GL_TEXTURE_2D);

    glColor4f(0.0, 0.0, 0.0, alpha * 0.25f);

    glPushMatrix();

        glTranslatef( 2.0f,
                      2.0f,
                      0.0f);

//        glTranslatef( (display.height * (1.0-max_percent)) * 0.25f,
//                      (display.height * (1.0-max_percent)) * 0.25f,
//                      0.0f);

        glBegin(GL_QUADS);
            glVertex2f(0.0f, 0.0);
            glVertex2f(scaled_w, 0.0);
            glVertex2f(scaled_w, scaled_h);
            glVertex2f(0.0f, scaled_h);
        glEnd();

    glPopMatrix();

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, image->textureid);

    glColor4f(1.0, 1.0, 1.0, alpha);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0f, 0.0f);
        glVertex2f(0.0f, 0.0);

        glTexCoord2f(1.0, 0.0f);
        glVertex2f(scaled_w, 0.0);

        glTexCoord2f(1.0, 1.0f);
        glVertex2f(scaled_w, scaled_h);

        glTexCoord2f(0.0f, 1.0f);
        glVertex2f(0.0f, scaled_h);
    glEnd();

    glPopMatrix();
}

//SlideShowApp

SlideShowApp::SlideShowApp(std::string conffile) : ConfApp(conffile) {
}

void SlideShowApp::init() {

    std::string imagedir = config.getString("slideshow", "image_dir");

    float duration = config.getFloat("slideshow", "image_duration");

    if(duration==0.0f) duration = 10.0f;

    vec2f screen_ratio = config.getVec2("slideshow", "screen_ratio");

    if(screen_ratio.x <= 0.0 || screen_ratio.y <= 0.0) {
        screen_ratio = vec2f(display.width, display.height);
    }

    app = new SlideShow(duration, screen_ratio);
    app->init();

    int image_count = 0;

    if(imagedir.size()) {
        //append slash
        if(imagedir[imagedir.size()-1] != '/') {
            imagedir += std::string("/");
        }

        //get tga, jpg and png images in dir
        DIR *dp;
        struct dirent *dirp;

        if((dp = opendir(imagedir.c_str())) == 0) return;

        while ((dirp = readdir(dp)) != 0) {
            std::string dirfile = std::string(dirp->d_name);

            int ext = 0;

            if(   (ext=dirfile.rfind(".jpg"))  == std::string::npos
               && (ext=dirfile.rfind(".jpeg")) == std::string::npos
               && (ext=dirfile.rfind(".tga")) == std::string::npos
               && (ext=dirfile.rfind(".png"))  == std::string::npos) {
               continue;
            }

            std::string imagefile = imagedir + dirfile;

            ((SlideShow*)app)->addImageFile(imagefile);
            image_count++;
        }

        closedir(dp);

	((SlideShow*)app)->randomize();
    }

    if(image_count>0) ready=true;
}

