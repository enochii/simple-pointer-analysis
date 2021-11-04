#include "NodeFactory.h"

NodeIdx NodeFactory::createValueNode(const Value* value) {
  int idx = getNextIdx();
	auto node = make_unique<AndersonNode>(idx, value, AndersonNode::ValueNode);
	assert(nodeMap.find(idx) == nodeMap.end());
	nodeMap.emplace(idx, std::move(node));
	return idx;
}

NodeIdx NodeFactory::createObjectNode(const Value* value) {
	int idx = getNextIdx();
	auto node = make_unique<AndersonNode>(idx, value, AndersonNode::ObjectNode);
	assert(nodeMap.find(idx) == nodeMap.end());
	nodeMap.emplace(idx, std::move(node));
	return idx;
}