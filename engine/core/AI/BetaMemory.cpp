#include "BetaMemory.h"
#include "JoinNode.h"

void BetaMemory::join_activation(Token* t, WME* w) {
    Token* new_token = new Token(w, t);
    items.push_front(new_token);
    for (JoinNode* child : children) { child->beta_activation(t); }
}