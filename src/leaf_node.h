#ifndef _LEAF_NODE_H
#define _LEAF_NODE_H
#include "node.h"
namespace top_k{
	struct LeafNode : public Node{
		LeafNode(uint16_t doc_id, uint32_t pos):Node(),doc_id(doc_id),pos(pos)
		{}		
		uint16_t doc_id;
		uint32_t pos;
	};
}// end top-k
#endif