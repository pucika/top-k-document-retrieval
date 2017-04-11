#ifndef _UTILITY_H
#define _UTILITY_H
#include <utility>
#include "i_structure.h"
#include "edge.h"
#include "node.h"

namespace top_k{
	class Node;
	using pair_score_arg = pair<int16_t, 
		std::tuple<Node*, ptrdiff_t, ptrdiff_t> >;

	inline std::ostream& operator <<(std::ostream &out,
		const Node &node)
	{
		for(auto ite = node.doc_id_map.begin(); ite != node.doc_id_map.end(); ite++)
		{
			out << ite->first << ":" << std::endl;
			out << ite->second << std::endl;
		}
		return out;
	}
	inline std::ostream& operator <<(std::ostream &out,
		const Edge &e)
	{
		out << "Edge(lead_to_node size): " << e.lead_to_node->character_map.size() << std::endl;
		return out;
	}
	inline std::ostream& operator <<(std::ostream &out, 
			const N_Structure &ns)
	{
		out << " doc_id:" << ns.doc_id 
			<< " score: " << ns.score 
			<< " origin:" << ns.origin->euler_index
			<< " target: " << ns.target->euler_index
			<< std::endl;
		return out;
	}
	inline std::ostream& operator <<(std::ostream &out,
	 		const I_Structure &is)
	{
		out << " euler_index: " << is.origin
			<< " score: " << is.score
		 	<< " doc_id: " << is.doc_id
		 	<< std::endl;
		return out;
	}
	bool i_structure_comp(const I_Structure& a, const I_Structure& b)
	{
		if (a.origin == b.origin)
			return a.doc_id < b.doc_id;
		return a.origin < b.origin;
	}

	bool score_que_comp(const pair_score_arg &a, const pair_score_arg &b)
	{
		return a.first < b.first;
	}
}//end top_k
#endif