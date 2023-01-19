#pragma once

#include "WME.h"

class Token {
public:
    Token* parent;
    int higher_token_i;
    WME* wme;
    Token(WME* wme, Token* parent) :wme(wme), parent(parent) {
        if (!parent) {
            higher_token_i = 0;
        }
        else {
            higher_token_i = parent->higher_token_i + 1;
        }
    }
    WME* index(int i);
};
