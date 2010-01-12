#ifndef STACKBET_H
#define STACKBET_H

#include "node.h"

class StackNode;

class StackBet {
    vec3f colour;
    float amount;
    std::string dollar_amount;

    StackNode* node;

    float y;
    float min_bet_height;
    float max_speed;

    float amount_elapsed;

    float alpha;
    float speed;
    bool applied;
    bool reached;
    bool finished;
public:
    StackBet(StackNode* node, float amount);

    float getHeight();

    bool hasApplied();
    bool reachedBar();
    bool isFinished();

    void apply();

    void logic(float dt);
    void draw(float dt);
    void drawDollarAmount(float dt);
};

extern float gMinBetHeight;

#endif
