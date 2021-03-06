#pragma once

#include <llvm/IR/Value.h>

#include <map>
#include <memory>
#include <vector>

using namespace llvm;
using namespace std;

/// Anderson Node definition
typedef unsigned NodeIdx;
class LocationNode {
public:
	enum NodeType {
		ValueNode,  // kind of like pointer?
		ObjectNode, // alloc site 
	};

private:
	int idx; // node id
	const Value* value; // correspinding inst
	NodeType nodeType;
friend class NodeFactory;
public:
	LocationNode(int idx, const Value* value, NodeType nodeType)
				:idx(idx), value(value), nodeType(nodeType) {}
	const Value* getValue() const { return value; }
};

class NodeFactory {
	vector<LocationNode> nodes;
	map<const Value*, NodeIdx> objNodes;
	map<const Value*, NodeIdx> valNodes;
	map<const Value*, NodeIdx> retNodes;
public:
	NodeIdx createValNode(const Value* value);
	NodeIdx createObjNode(const Value* value);
	NodeIdx createRetNode(const Value* value);
	NodeIdx getValNode(const Value* value)const;
	NodeIdx getObjNode(const Value* value)const;
	NodeIdx getRetNode(const Value* value)const;

	// get the "llvm::Value*"(maybe a inst, func) by node index
	const Value* getValueByNodeIdx(NodeIdx idx)const;
	bool isValueNode(NodeIdx idx)const;
	unsigned getNumNode() { return nodes.size(); }
};