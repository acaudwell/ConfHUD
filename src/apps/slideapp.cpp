#include "slideapp.h"

SlideShowApp::SlideShowApp(std::string conffile) : ConfApp(conffile) {
}

void SlideShowApp::init() {

    std::string imagedir = config.getString("slideshow", "image_dir");

    float duration = config.getFloat("slideshow", "image_duration");

    if(duration==0.0f) duration = 10.0f;

    app = new SlideShow(duration);
    app->init();

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
        }

        closedir(dp);
    }

    ready=true;
}

