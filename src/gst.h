#ifndef _GST_H
#define _GST_H
#include <string>
#include <list>
#include <vector>
#include <utility>
#include <cstdint>
#include <sdsl/bit_vectors.hpp>
//#include <sdsl/suffix_trees.hpp>
#include <sdsl/bp_support.hpp>
#include "node.h"
#include "edge.h"
#include "leaf_node.h"
#include "RMQRMM64.h"

using top_k::Edge;
using top_k::Node;
using top_k::LeafNode;
using sdsl::bit_vector;
using sdsl::rank_support_v;
using sdsl::bp_support_sada;
//using sdsl::cst_sct3;
using std::list;
using std::string;
using namespace rmqrmm;

typedef std::pair<Node*, int32_t> pair_t;
typedef std::pair<bool, Node*> tas_t;

namespace top_k{
	
	class GST {
	public:
		GST():dumy_root(new Node(nullptr, -1)),root(new Node(dumy_root)), 
			cur_ind(0),docs(nullptr), docs_len(0), num_doc(0),doc_index(0),nnodes(0),
			state{root, 0},doc_array(nullptr),doc_array_rank(nullptr)
		{
			for (uint32_t i = 0; i != 256; i++)
			{
				dumy_root->character_map[i] = new Edge(i,i,this->root);
			}
		}
		~GST()
		{
			delete [] docs;
			//TODO release every node's N-Structure
		}	
		void construct(const char*);
		void search(const string pattern, const int k);
		bool verify(); 
	
	private:
		
		void 	insert(Node *node, uint32_t pos);
		void 	insert(Node *node, uint32_t pos, Edge* edge);
		pair_t 	update(Node* s, int32_t k, int32_t i);
		tas_t 	test_and_split(Node* s, int32_t k, int32_t p, int32_t ch);
		pair_t 	canonize(Node* s, int32_t k, int32_t p);
		Edge* 	get_edge(Node* node, uint32_t pos)
		{
			return docs[pos] == '\0' ? node->at(256 + doc_rank(pos)) 
								: node->at(docs[pos]);
		}
		bool 	verify_helper(const uint8_t*);
		void 	load_file(const char*);
		void 	set_doc_array(int32_t i)
		{
			(*doc_array)[i] = 1;
		}
		int32_t doc_rank(int32_t i)
		{
			return doc_array_rank->rank(i);
		}
		void euler_tour();
		void euler_tour_helper(Node *node, int& level);
		void show_euler_tour();
		void preprocess();
		Node* lca(int , int );
		void init_leaf_n_structure();
		void init_n_structure();
		void init_n_structure_helper(int, list<Node*>&);
		void dump_n_structure(const Node*);
		void dump_i_structure(const Node*);
		void init_i_structure();
		void init_i_structure_helper(Node*);
		void preprocess_i_structure();
		void preprocess_i_structure_helper(Node*);
		Node *locate_locus(const string pattern);
		void init_bps(Node* node, uint64_t& bps_ind, uint64_t& dfs_ind);
	private:
//		cst_sct3<> cst;
		Node *dumy_root;
		Node *root;
 		int32_t cur_ind;		// i
 		//uint32_t last_ind; 		// k
		uint8_t* docs;		//doc set
		int32_t docs_len;
		int32_t num_doc;
		int32_t doc_index;
		int32_t nnodes;		//used for bps
		bit_vector* bp;		//bp
		pair_t state;
		bit_vector* doc_array;
		rank_support_v<1>* doc_array_rank;
		vector<Node*> euler_tour_vec;
		vector<int> level_vec;		//cleared in preprocess
		RMQRMM64 *level_rmq;
		vector<LeafNode*> leaf_node_vec;
		vector<string> doc_names;
	}; 

}//end top_k

#endif