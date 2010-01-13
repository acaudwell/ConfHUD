/*
    Copyright (C) 2009 Andrew Caudwell (acaudwell@gmail.com)

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License
    as published by the Free Software Foundation; either version
    3 of the License, or (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "main.h"

int main(int argc, char *argv[]) {

    int width  = 1280;
    int height = 768;
    bool fullscreen=true;
    bool multisample=false;
    vec3f background = vec3f(0.25, 0.25, 0.25);

    std::vector<std::string> arguments;

    SDLAppParseArgs(argc, argv, &width, &height, &fullscreen, &arguments);

    for(int i=0;i<arguments.size();i++) {
        std::string args = arguments[i];

        if(args == "-h" || args == "-?" || args == "--help") {
            confhud_help("");
        }

        // unknown argument
        std::string arg_error = std::string("unknown option ") + std::string(args);

        confhud_help(arg_error);
    }

    // this causes corruption on some video drivers
    if(multisample) {
        display.multiSample(4);
    }

    //needed because we use the glActiveTextureARB extension ...
    display.enableShaders(true);

    //enable vsync
    display.enableVsync(true);

    display.init("ConfHUD", width, height, fullscreen);

    if(multisample) glEnable(GL_MULTISAMPLE_ARB);

    ConfHUD* conf = new ConfHUD("confhud.conf");

    conf->run();

    delete conf;

    //free resources
    display.quit();

    return 0;
}
