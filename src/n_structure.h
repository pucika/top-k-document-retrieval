#ifndef _N_STRUCTURE_H
#define _N_STRUCTURE_H
#include <utility>

namespace top_k{
	class Node;
	//first_depth, last_depth haven't used. TODO
	struct N_Structure{
		friend std::ostream& operator <<(std::ostream &out, const N_Structure &ns);

		N_Structure(int32_t doc_id, int16_t score,
			Node* origin = nullptr, Node* target = nullptr)
		:doc_id(doc_id),score(score), origin(origin), target(target)
		{}
		~N_Structure()
		{
			origin = nullptr;
			target = nullptr;
		}
		int32_t doc_id;
		int16_t score;
		Node* origin;
		Node* target;
		//int32_t first_depth;
		//int32_t last_depth;

	};
}

#endif