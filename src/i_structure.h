#ifndef _I_STRUCTURE_H
#define _I_STRUCTURE_H
#include <utility>

namespace top_k{

	struct I_Structure{
		friend std::ostream& operator <<(std::ostream &out, const I_Structure &ns);

		I_Structure(int32_t doc_id, int16_t score,int32_t origin)
			:doc_id(doc_id),score(score), origin(origin)
		{}
		int32_t doc_id;
		int16_t score;
		int32_t origin;
	};
}

#endif