#pragma once

#include <llvm/IR/Value.h>

#include <map>
#include <memory>

using namespace llvm;
using namespace std;

/// Anderson Node definition
typedef unsigned NodeIdx;
class AndersonNode {
public:
	enum NodeType {
		ValueNode, ObjectNode, 
	};

private:
	int idx; // node id
	const Value* value; // correspinding inst
	NodeType nodeType;

public:
	AndersonNode(int idx, const Value* value, NodeType nodeType)
				:idx(idx), value(value), nodeType(nodeType) {}
};

class NodeFactory {
	int nextIdx = 0;
	typedef unique_ptr<AndersonNode> NodePtr;
	map<NodeIdx, NodePtr> nodeMap;
public:
	NodeIdx createValueNode(const Value* value);
	NodeIdx createObjectNode(const Value* value);
	NodeIdx getNextIdx() { return nextIdx++; }
};