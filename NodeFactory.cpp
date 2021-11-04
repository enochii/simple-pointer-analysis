#include "NodeFactory.h"

NodeIdx NodeFactory::createValNode(const Value* value) {
  int idx = nodes.size();
	nodes.emplace_back(idx, value, AndersonNode::ValueNode);
	assert(valNodes.find(value) == valNodes.end() && "Node already exist");
	valNodes.emplace(value, idx);
	return idx;
}

NodeIdx NodeFactory::createObjNode(const Value* value) {
	int idx = nodes.size();
	nodes.emplace_back(idx, value, AndersonNode::ObjectNode);
	assert(objNodes.find(value) == objNodes.end() && "Node already exist");
	objNodes.emplace(value, idx);
	return idx;
}

NodeIdx NodeFactory::getValNode(const Value* value)const {
  assert(valNodes.find(value) != valNodes.end() && "Node dose NOT exist");
  return valNodes.at(value);
}

NodeIdx NodeFactory::getObjNode(const Value* value)const {
  assert(objNodes.find(value) != objNodes.end() && "Node dose NOT exist");
  return objNodes.at(value);
}

const Value* NodeFactory::getValueByNodeIdx(NodeIdx idx)const {
	assert(idx < nodes.size() && "node idx out of bound");
	return nodes[idx].getValue();
}