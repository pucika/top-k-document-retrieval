#ifndef _NODE_H
#define _NODE_H
#include <map>
#include <list>
#include <cstdint>
#include <sdsl/rmq_succinct_sct.hpp>
#include "n_structure.h"
#include "i_structure.h"
#include "edge.h"
#include "RMQRMM64.h"

using std::list;
using std::vector;
using sdsl::rmq_succinct_sct;
using top_k::N_Structure;
using top_k::I_Structure;
using top_k::Edge;
using namespace rmqrmm;

namespace top_k{

	struct Node{
		Node(Node* suf = nullptr, int32_t euler_index = 0)
			:euler_index(euler_index),dfs_index(0),visited(false),level(0),
			suf(suf),parent(nullptr),n_stru_vec(),i_stru_vec(),character_map(),
			doc_id_map()
		{}
		Edge* at(uint16_t ch){
			return (ch < 256 ? 
				character_map[ch] : doc_id_map[ch]);
		}
		void set_parent(Node *parent)
		{
			this->parent = parent;
		}
		bool has(int16_t ch){
			return ch < 256 ? character_map.find(ch) != character_map.end():
				doc_id_map.find(ch) != doc_id_map.end();
		}
		bool is_leaf() const
		{
			return character_map.empty() && doc_id_map.empty();
		}
		N_Structure& get_n_structure(int doc_id)
		{
			for (auto ite = n_stru_vec.begin(); ite != n_stru_vec.end(); ite++){
				if (ite->doc_id == doc_id)
					return *ite;
			}
		}
		bool has_n_structure(int doc_id)
		{
			for (auto ite = n_stru_vec.begin(); ite != n_stru_vec.end(); ite++){
				if (ite->doc_id == doc_id)
					return true;
			}
			return false;
		}
		~Node()
		{
			if (suf != nullptr)
			{
				delete suf;
				suf = nullptr;
			}
				
			if (parent != nullptr)
			{
				delete parent;
				parent = nullptr;
			}
			for(auto ite = character_map.begin(); ite != character_map.end(); ite++)
				delete ite->second;
			for(auto ite = doc_id_map.begin(); ite != doc_id_map.end(); ite++)
				delete ite->second;
		}
		uint32_t euler_index;
		uint32_t dfs_index;
		bool visited;
		int32_t level;
		Node* suf;
		Node* parent;
		vector<N_Structure> n_stru_vec;
		vector<I_Structure> i_stru_vec; 
		std::map<uint8_t, Edge*> character_map;
		//deals with doc's separator; no more than 2^16 docs 
		std::map<uint16_t, Edge*> doc_id_map;
		rmq_succinct_sct<> score_rmq;
	};
}//end top-k
#endif