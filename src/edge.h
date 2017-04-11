#ifndef _EDGE_H
#define _EDGE_H
#include <climits>
#include <cstdint>
namespace top_k{
	class Node;

	struct Edge{
		Edge(int32_t start = 0, int32_t end = INT_MAX, 
			Node* lead_to_node = nullptr)
			:start(start), end(end),lead_to_node(lead_to_node)
		{} 
		~Edge()
		{
			if(lead_to_node != nullptr)
			{
				delete lead_to_node;
				lead_to_node = nullptr;
			}
		}
		int len()
		{
			return end - start + 1;
		}
		int32_t start;
		int32_t end;
		Node *lead_to_node;
	};

}//end top_k
#endif