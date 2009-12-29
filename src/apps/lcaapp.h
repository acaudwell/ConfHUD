#ifndef LCA_CONFAPP_H
#define LCA_CONFAPP_H

#include "confapp.h"
#include "lca/lca.h"

class LCAApp : public ConfApp {
public:
    LCAApp(std::string conffile);
    void init();
};

#endif
