#include "Token.h"
#include <assert.h>
#include <iostream>
WME* Token::index(int i) {
    if (!((i >= 0) && (i < higher_token_i))) {
        std::cerr << "i: " << i << " higher_token_i: " << higher_token_i << "\n";
    }
    assert(i >= 0);
    assert(i <= higher_token_i);
    if (i == higher_token_i) { return wme; }
    assert(parent != nullptr);
    return parent->index(i);
}