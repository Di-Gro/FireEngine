#include "ProductionNode.h"

void ProductionNode::join_activation(Token* t, WME* w)
{
	t = new Token(w, t);
	items.push_back(t);
}
