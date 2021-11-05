#pragma once

#include <llvm/IR/Value.h>

#include <map>
#include <memory>
#include <vector>

using namespace llvm;
using namespace std;

/// Anderson Node definition
typedef unsigned NodeIdx;
class AndersonNode {
public:
	enum NodeType {
		ValueNode,  // kind of like pointer?
		ObjectNode, // alloc site 
	};

private:
	int idx; // node id
	const Value* value; // correspinding inst
	NodeType nodeType;

public:
	AndersonNode(int idx, const Value* value, NodeType nodeType)
				:idx(idx), value(value), nodeType(nodeType) {}
	const Value* getValue() const { return value; }
};

class NodeFactory {
	vector<AndersonNode> nodes;
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
};