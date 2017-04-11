#include <sys/stat.h>
#include <unistd.h>
#include <iostream>
#include <fcntl.h>
#include <utility>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>
#include <cstring>
#include <vector>
#include <list>
#include <stack>
#include <cassert>
#include <queue>
#include <chrono>
#include <unistd.h>
#include <sdsl/util.hpp>
#include "gst.h"
#include "leaf_node.h"
#include "utility.h"

//using top_k::Node;
//using top_k::Edge;
//using top_k::LeafNode;
//using sdsl::bit_vector;
//using sdsl::rank_support_v;
using std::cout;
using std::pair;
using std::priority_queue;
using sdsl::rmq_succinct_sct;
using sdsl::util::clear;
using sdsl::int_vector;
namespace top_k{

void GST::load_file(const char *path)
{
	DIR *pdir = opendir(path);
	struct dirent* ptr = nullptr;
	struct stat st_buf;
	char tmp_path[100];
	strncpy(tmp_path, path, 100);
	strcat(tmp_path,"/");
	char fullpath[100];
	while((ptr = readdir(pdir) )!= NULL)
	{
		if(strcmp(ptr->d_name,".") && strcmp(ptr->d_name, ".."))
		{
			++num_doc;
			doc_names.push_back(ptr->d_name);
			strcpy(fullpath,tmp_path);
			if(lstat(strcat(fullpath,ptr->d_name), &st_buf) < 0)
			{
				std::cerr << "lstat error" << std::endl;
			}
			docs_len += st_buf.st_size;
		}
	}
	closedir(pdir);
	pdir = opendir(path);
	this->docs_len += num_doc;
	this->docs = new uint8_t[this->docs_len];
	this->doc_array = new bit_vector(this->docs_len, 0);
	int read_len = 0;
	int fd = 0;
	while((ptr = readdir(pdir) )!= NULL)
	{
		if(strcmp(ptr->d_name,".")  && strcmp(ptr->d_name, ".."))
		{
			strcpy(fullpath,tmp_path);
			strcat(fullpath,ptr->d_name);
			if ((fd = open(fullpath,O_RDONLY)) < 0)
				std::cerr << "read error" << std::endl;
			int num = 0;
			while((num = read(fd, this->docs + read_len, 4096)) > 0) 
			{
				read_len += num;
			}
			this->docs[read_len++] = '\0';
			set_doc_array(read_len - 1);
			close(fd);
		}
	}
	this->doc_array_rank = new rank_support_v<1>(&(*(this->doc_array)));
	closedir(pdir);
	#ifdef GTEST
		std::cout << "finish loading file! " << std::endl;
	#endif
}

void GST::construct(const char* file_path)
{
	load_file(file_path);
	//std::cout << "constructing..." << std::endl;
	// deals with '\0' 
	for(int i = 0; i != num_doc; ++i) {
		root->doc_id_map[256 + i] = 
			new Edge(this->cur_ind, INT_MAX, 
				new LeafNode(i, 0)), 	
			dumy_root->doc_id_map[256 + i] =
			new Edge(0,0,this->root);
	}
	for (; cur_ind != docs_len; cur_ind++)
	{
		//std::cout << "index: " << cur_ind << std::endl;
		this->state = update(this->state.first, this->state.second, this->cur_ind); 
		//deal with the '\0', rm suffixes started by '\0'
		this->state = canonize(this->state.first , this->state.second, this->cur_ind);
		if (docs[cur_ind] == '\0')
		{
			// rm suf started by '\0'
			++doc_index;
		} 
			
	}
	//std::cout << "leaving construction. " << std::endl;
	preprocess();
}

pair_t GST::update(Node* s, int32_t k, int32_t i)
// (s,(k, i -1 )) is the canonical reference pair for the active point
// it means we will extend to the ith stage from (i-1)th stage 
{
	Node* oldr = this->root;
	tas_t tst_r = test_and_split(s, k, i - 1, i); 	//deals with different seperators
	while(!tst_r.first)
	{
		insert(tst_r.second, i);
		if (oldr != this->root)
			oldr->suf = tst_r.second;
		oldr = tst_r.second;
		this->state = canonize(this->state.first->suf, this->state.second, this->cur_ind - 1);
		tst_r = test_and_split(this->state.first, this->state.second,
						 this->cur_ind - 1, this->cur_ind);
	}
	if (oldr != root) oldr->suf = this->state.first;
	return {this->state.first,this->state.second};
}

tas_t GST::test_and_split(Node* s, int32_t k, int32_t p, int32_t pos)
{
	if (k <= p)
	{
		int32_t _k = 0; 
		//uint32_t _p = 0;
		Edge* active_edge = get_edge(s,k);
		_k = active_edge->start;
		//_p = active_edge->end;
		if (this->docs[_k + p - k + 1] != '\0' ? 
			this->docs[pos] == this->docs[_k + p - k + 1] : pos == _k + p - k + 1)
		{
			return {true, s};
		}else{
			Node* new_node = new Node();
			insert(new_node, _k + p - k + 1, 
				new Edge(_k + p - k + 1, active_edge->end, active_edge->lead_to_node));
			new_node->set_parent(s);
			active_edge->lead_to_node->set_parent(new_node);
			active_edge->end = _k + p - k;
			active_edge->lead_to_node = new_node;
			return {false, new_node};
		}
	}else {		//when k > p?
		return this->docs[pos] == '\0' ? 
			std::make_pair(s->has(256 + doc_rank(pos)), s) : 
			std::make_pair(s->has(this->docs[pos]), s);
	}
}

pair_t GST::canonize(Node* s, int32_t k, int32_t p)
{
	if (p < k) return {s, k};
	int32_t _k = 0; 
	int32_t _p = 0;
	Edge* active_edge = get_edge(s,k);
	_k = active_edge->start;
	_p = active_edge->end;
	while( _p - _k <= p - k)
	{
		k += _p - _k + 1;
		s = active_edge->lead_to_node;
		if (k <= p)
		{
			active_edge = get_edge(s,k);
			_k = active_edge->start;
			_p = active_edge->end;
		}
	}
	return {s, k};
}

//for leaf node insert edge
inline void GST::insert(Node *node, uint32_t pos)
{
	docs[pos] != '\0' ? node->character_map[docs[pos]] = 
			new Edge(this->cur_ind, INT_MAX, 
				new LeafNode(doc_rank(pos),this->cur_ind - this->state.second))
		: (node->doc_id_map[256 + doc_rank(pos)] = 
			new Edge(this->cur_ind, INT_MAX, 
				new LeafNode(doc_rank(pos), this->cur_ind)), 	
			this->dumy_root->doc_id_map[256 + doc_rank(pos)] =
			new Edge(0,0,this->root));

}
//for internal insert edge
inline void GST::insert(Node *node, uint32_t pos, Edge* edge)  //which '\0' in the original edge
{
	docs[pos] != '\0' ? node->character_map[docs[pos]] = edge : 
		node->doc_id_map[256 + doc_rank(pos)] = edge,
		this->dumy_root->doc_id_map[256 + doc_rank(pos)] =
			new Edge(0,0,this->root);
}

bool GST::verify()
{
	uint8_t * docp = this->docs;
	uint8_t * endp = this->docs + this->docs_len;
	for (; docp != endp; docp++)
	{
		std::cout << "veryfing " << docp - this->docs << " th suffix" << std::endl;
		if (*docp != '\0')
		{
			if (!verify_helper(docp)) 
			{
				return false;
			}
		}
	}

	std::cout << "doc_len: " << this->docs_len << std::endl;
	std::cout << "the num of docs: " << this->doc_index << std::endl;
	return true;
}

bool GST::verify_helper(const uint8_t* suffix)
{
	const uint8_t *p = suffix;
	Node* active_node = root;
	Edge* active_edge = root->has(*p) ? root->at(*p) : nullptr;
	int32_t el = active_edge->start;
	int32_t er = active_edge->end;
	if (active_edge == nullptr) return false;
	while(*p != '\0')
	{
		for(; el != er + 1; el++)
		{
			if (*p == '\0') return true;
			if (docs[el] != *p++) return false; 
		}
		active_node = active_edge->lead_to_node;
		if (*p == '\0') return true;
		active_edge = root->has(*p) ? root->at(*p) : nullptr;
		if (active_edge == nullptr) return false;
		el = active_edge->start;
		er = active_edge->end;
		
	}
	return true;
}
void GST::euler_tour()
{
	Node* node = this->root;
	int level = 0;
	euler_tour_helper(node, level);
}
void GST::euler_tour_helper(Node *node, int& level)
{
	static int euler_index = 0;
	euler_tour_vec.push_back(node);
	level_vec.push_back(level);
	if (!node->visited){
		++nnodes;
		node->visited = true;
		node->euler_index = euler_index;
		node->level = level;
		if (node->is_leaf()){
			leaf_node_vec.push_back(static_cast<LeafNode*>(node));
		}
	}
	euler_index++;
	if (node->is_leaf()){
		level--;
		return ;
	}
	for (auto ite = node->character_map.begin(); ite != node->character_map.end(); ite++){
		level++;
		euler_tour_helper(ite->second->lead_to_node, level);
		euler_index++;
		euler_tour_vec.push_back(node);
		level_vec.push_back(level);
	}
	for (auto ite = node->doc_id_map.begin(); ite != node->doc_id_map.end(); ite++){
		level++;
		euler_tour_helper(ite->second->lead_to_node, level);
		euler_index++;
		euler_tour_vec.push_back(node);
		level_vec.push_back(level);
	}
	level--;
} 

void GST::show_euler_tour()
{
	#ifdef GTEST
		std::cout << "............euler_tour_vec.............." << std::endl;
		for (auto ite = euler_tour_vec.begin(); ite != euler_tour_vec.end(); ite++){
			std::cout << (*ite)->euler_index << " ";
		}
		std::cout << std::endl << "............level_vec............" << std::endl;
		for (auto ite = level_vec.begin(); ite != level_vec.end(); ite++){
			std::cout << *ite << " ";
		}
		std::cout << std::endl << ".............leaf_node_vec......." << std::endl;
		for (auto ite = leaf_node_vec.begin(); ite != leaf_node_vec.end(); ite++){
			std::cout << (*ite)->euler_index << " ";
		}
		std::cout << std::endl;
	#endif
}

void GST::init_bps(Node* node, uint64_t& bps_ind, uint64_t &dfs_ind)
{
	node->dfs_index = bps_ind;
	(*bp)[bps_ind++] = 1;
	for (auto ite = node->character_map.begin(); ite != node->character_map.end(); ite++){
		init_bps(ite->second->lead_to_node, bps_ind, dfs_ind);
	}
	for (auto ite = node->doc_id_map.begin(); ite != node->doc_id_map.end(); ite++){
		init_bps(ite->second->lead_to_node, bps_ind, dfs_ind);
	}
	(*bp)[bps_ind++] = 0;
}

void GST::preprocess()
{
	euler_tour();
	show_euler_tour();
	//init bps
	this->bp = new bit_vector(nnodes * 2, 0);
	uint64_t bps_ind = 0;
	uint64_t dfs_ind = 0;
	init_bps(root, bps_ind, dfs_ind);
	assert(bps_ind == nnodes * 2);
	long int *level_array = new long int[level_vec.size()];
	for (int i = 0; i != level_vec.size(); i++){
		level_array[i] = level_vec[i];
	}
	level_rmq = new RMQRMM64(level_array, level_vec.size());
	level_vec.clear();
	delete[] level_array;
	//std::cout << std::endl << lca(4, 7)->euler_index << std::endl;
	init_n_structure();
	//std::cout << "--------------dump_n_structure---------------" << std::endl;
	//dump_n_structure(root);
	init_i_structure();
	preprocess_i_structure();
	//std::cout << "--------------dump_i_structure---------------" << std::endl;
	//dump_i_structure(root);
#ifdef GTEST
	std::cout <<  "---------------informations-----------" << std::endl;
	std::cout << "docs_len: " << docs_len << std::endl;
	std::cout << "leaf_node_vec : " << leaf_node_vec.size() << std::endl;
	std::cout << "euler_tour_vec size: " << euler_tour_vec.size() << std::endl;
	std::cout << "doc_names" << std::endl;
	copy(doc_names.begin(), doc_names.end(), ostream_iterator<string>(std::cout, ", "));
	std::cout << std::endl;
#endif
}

Node* GST::lca(int begin, int end)
{
	if (begin > end){
		swap(begin, end);
	}
	int pos = level_rmq->queryRMQ(begin, end);
	return euler_tour_vec[pos];
}

void GST::init_leaf_n_structure()
{
	for(auto ite = leaf_node_vec.begin(); ite != leaf_node_vec.end(); ite++){
		(*ite)->n_stru_vec.push_back(
			N_Structure((*ite)->doc_id, 1, (*ite)));
	}

}

void GST::init_n_structure()
{
	init_leaf_n_structure();
	std::list<Node*> doc_leaf;
	for (int i = 0; i != doc_index; i++){
		for (auto ite = leaf_node_vec.begin(); ite != leaf_node_vec.end(); ite++){
			if ((*ite)->doc_id == i)
				doc_leaf.push_back(static_cast<Node*>(*ite));
		}
		doc_leaf.push_back(root);
		init_n_structure_helper(i,doc_leaf);
		doc_leaf.clear();
	}
	for (auto ite = root->n_stru_vec.begin(); ite != root->n_stru_vec.end(); ite++){
		(*ite).target = dumy_root;
	}
}

void GST::init_n_structure_helper(int cur_doc, list<Node*>& doc_leaf)
{	
	using std::stack;
	stack<Node*> lca_stack;
	lca_stack.push(root);
	for (auto ite = doc_leaf.begin(); ite != prev(doc_leaf.end()); ){
		Node *lca_with_top = lca((*ite)->euler_index, (lca_stack.top())->euler_index);
		Node *lca_with_next = lca((*ite)->euler_index, (*next(ite))->euler_index);

		if (lca_with_top->level < (lca_stack.top())->level){
			Node *old_top = lca_stack.top();
			lca_stack.pop();
			if (lca_with_top->level > lca_stack.top()->level)
				lca_stack.push(lca_with_top);
			old_top->get_n_structure(cur_doc).origin = old_top;
			old_top->get_n_structure(cur_doc).target = lca_stack.top();
			if (!lca_stack.top()->has_n_structure(cur_doc)){
				lca_stack.top()->n_stru_vec.push_back(N_Structure(cur_doc, 0, lca_stack.top()));
			}
			lca_stack.top()->get_n_structure(cur_doc).score += old_top->get_n_structure(cur_doc).score;
		} else {
			if (lca_with_next->level < lca_with_top->level){
				(*ite)->get_n_structure(cur_doc).origin = (*ite);
				(*ite)->get_n_structure(cur_doc).target = lca_stack.top();
				if (!lca_stack.top()->has_n_structure(cur_doc)) {
					lca_stack.top()->n_stru_vec.push_back(N_Structure(cur_doc, 0, lca_stack.top()));
				}
				lca_stack.top()->get_n_structure(cur_doc).score += (*ite)->get_n_structure(cur_doc).score;
				++ite;
			} else {
				if (lca_with_next != lca_stack.top())
					lca_stack.push(lca_with_next);
				(*ite)->get_n_structure(cur_doc).origin = (*ite);
				(*ite)->get_n_structure(cur_doc).target = lca_stack.top();
				if (!lca_stack.top()->has_n_structure(cur_doc)){
					lca_stack.top()->n_stru_vec.push_back(N_Structure(cur_doc, 0, lca_stack.top()));
				}
				lca_stack.top()->get_n_structure(cur_doc).score += (*ite)->get_n_structure(cur_doc).score;
				++ite;
			}
		}
	}
}

void GST::dump_n_structure(const Node *dump_node)
{
	if (dump_node->is_leaf())
		return ;
	cout << dump_node->euler_index << ": " << std::endl;
	for(auto ite = dump_node->n_stru_vec.begin(); ite != dump_node->n_stru_vec.end(); ite++){
		cout << *ite;
	}
	cout << std::endl;
	for (auto ite = dump_node->character_map.begin(); ite != dump_node->character_map.end(); ite++){
		dump_n_structure(ite->second->lead_to_node);
	}
	for (auto ite = dump_node->doc_id_map.begin(); ite != dump_node->doc_id_map.end(); ite++){
		dump_n_structure(ite->second->lead_to_node);
	}
}

void GST::init_i_structure()
{
	init_i_structure_helper(root);
}
void GST::init_i_structure_helper(Node* node)
{
	if (true == node->visited){
		for (auto ite = node->n_stru_vec.begin(); ite != node->n_stru_vec.end(); ite++){
			Node *target = ite->target;
			if (target != nullptr){
				target->i_stru_vec.push_back(
					I_Structure(ite->doc_id, ite->score, ite->origin->euler_index));
			}
		}
		vector<N_Structure>().swap(node->n_stru_vec);
	}
	node->visited = false;
	for (auto ite = node->character_map.begin(); ite != node->character_map.end(); ite++){
		init_i_structure_helper(ite->second->lead_to_node);
	}
	for (auto ite = node->doc_id_map.begin(); ite != node->doc_id_map.end(); ite++){
		init_i_structure_helper(ite->second->lead_to_node);
	}
}
void GST::preprocess_i_structure()
{
	preprocess_i_structure_helper(root);
}
void GST::preprocess_i_structure_helper(Node* node)
{
	if (false == node->visited){
		node->i_stru_vec.shrink_to_fit();
		sort(node->i_stru_vec.begin(), node->i_stru_vec.end(), i_structure_comp);
		int_vector<> score_temp(node->i_stru_vec.size(), 0);
		//long int* score_temp = new long int[node->i_stru_vec.size()];
		for (int  i =0; i != (node->i_stru_vec).size(); i++)
		{
			score_temp[i] = (node->i_stru_vec)[i].score * -1;
		}
		//node->score_rmq = new RMQRMM64(score_temp, node->i_stru_vec.size());	
		node->score_rmq = rmq_succinct_sct<>(&score_temp);
		clear(score_temp);
	}
	node->visited = true;
	for (auto ite = node->character_map.begin(); ite != node->character_map.end(); ite++){
		preprocess_i_structure_helper(ite->second->lead_to_node);
	}
	for (auto ite = node->doc_id_map.begin(); ite != node->doc_id_map.end(); ite++){
		preprocess_i_structure_helper(ite->second->lead_to_node);
	}
}

void GST::dump_i_structure(const Node *dump_node)
{
	if (dump_node->is_leaf())
		return ;
	cout << dump_node->euler_index << ": " << std::endl;
	for(auto ite = dump_node->i_stru_vec.begin(); ite != dump_node->i_stru_vec.end(); ite++){
		cout << *ite;
	}
	cout << std::endl;
	for (auto ite = dump_node->character_map.begin(); ite != dump_node->character_map.end(); ite++){
		dump_i_structure(ite->second->lead_to_node);
	}
	for (auto ite = dump_node->doc_id_map.begin(); ite != dump_node->doc_id_map.end(); ite++){
		dump_i_structure(ite->second->lead_to_node);
	}
}

Node* GST::locate_locus(const string str)
{
	if (str.empty())
		return nullptr;
	Node *active_node = root;
	uint16_t active_char = str.at(0);
	int active_len = 0;
	Edge *active_edge = active_node->at(active_char);
	if (nullptr == active_edge){
		return nullptr;
	} else {
		for (size_t i = 0; i != str.length(); i++){
			if (str.at(i) != docs[active_edge->start + active_len])
				return nullptr;
			else 
				++active_len;
			if (active_edge->len() <= active_len)
			{
				active_node = active_edge->lead_to_node;
				active_len = 0;
				if (i + 1 != str.length()){
					active_char = str[i + 1];
					active_edge = active_node->at(active_char);
					if (nullptr == active_edge)
						return nullptr;
				}
			}
		}
	}
	return active_edge->lead_to_node;
}

void GST::search(const string pattern, const int k)
{
	const static bp_support_sada<> bps(this->bp);
	auto start = std::chrono::steady_clock::now();
	vector<int> result;
	Node *locus_node = locate_locus(pattern);
	if (nullptr == locus_node){
		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration<double, ratio<1, 1000000>>(end - start).count() << std::endl;  
		//std::cout << "no doc contain pattern: " << pattern << std::endl;
		return ; 
	}
	if (locus_node->is_leaf())
	{
		auto end = std::chrono::steady_clock::now();
		std::cout << std::chrono::duration<double, ratio<1, 1000000>>(end - start).count() << std::endl;  
		#ifdef GTEST
			std::cout << doc_names[static_cast<LeafNode*>(locus_node)->doc_id] << std::endl;
		#endif
		return ;
	}
	std::pair<int, int> range;
	//auto range_start = std::chrono::steady_clock::now();
	range.first = locus_node->euler_index;
	range.second = range.first + bps.find_close(locus_node->dfs_index) - locus_node->dfs_index; 
	
	// {
		
	// 	auto lower = lower_bound(euler_tour_vec.begin(), euler_tour_vec.end(), locus_node);
	// 	auto upper = upper_bound(euler_tour_vec.begin(), euler_tour_vec.end(), locus_node);
	// 	//TODO, it may some cause mistake
	// 	//range.second = (*(++ite))->euler_index;
	// 	range.second = range.first + upper - lower;
		
		// auto ite = euler_tour_vec.rbegin();
		// while ((*ite)->euler_index != range.first) 
		// 	ite++;
		// ++ite;
		// range.second = euler_tour_vec.size() - (ite - euler_tour_vec.rbegin());
	// 	std::cout << "range.first: " << range.first << "range.second: " << range.second << endl;;
	// }
	// auto range_end = std::chrono::steady_clock::now();

	
	//std::cout << "bprange.first: " << range.first << "bprange.second: " << range.second << endl;
	//range_end = std::chrono::steady_clock::now();
	//std::cout << std::chrono::duration<double, ratio<1, 1000000>>(range_end - range_start).count() << std::endl;
	assert(range.first <= range.second);
	Node *ancestor = locus_node->parent;

	priority_queue< pair<int16_t, pair<Node*, pair<ptrdiff_t, ptrdiff_t>>>> score_que;

	int max_score_index = 0;

	while(nullptr != ancestor)
	{
		if(ancestor->i_stru_vec.empty())
		{
			ancestor = ancestor->parent;
			continue;
		}
		pair<ptrdiff_t, ptrdiff_t> tran_range;
		//auto ite = ancestor->i_stru_vec.begin();
		auto low = ancestor->i_stru_vec.begin();
		auto high = prev(ancestor->i_stru_vec.end());
		while(low < high) {
			auto mid  = low + (high - low) / 2;
			if (mid->origin == range.first)
				{low = mid; break;}
			else if (mid->origin < range.first)
				low = next(mid);
			else
				high = prev(mid); 
		}

		if (low->origin == range.first)
			tran_range.first = low - ancestor->i_stru_vec.begin();
		else 
			tran_range.first = low - ancestor->i_stru_vec.begin() - 1;
		high = prev(ancestor->i_stru_vec.end());

		// while( ite != ancestor->i_stru_vec.end() && 
		// 	ite->origin < range.first)
		// 	ite++;
		if (low != ancestor->i_stru_vec.end()){
		// 	tran_range.first = ite - ancestor->i_stru_vec.begin();
		// 	while (ite != ancestor->i_stru_vec.end()
		// 		&& ite->origin < range.second)
		// 		ite++;
		while(low < high) {
			auto mid  = low + (high - low) / 2;
			if (mid->origin == range.second)
				{low = mid; break;}
			else if (mid->origin < range.second)
				low = next(mid);
			else
				high = prev(mid); 
		}
		if (low->origin == range.second)
			tran_range.second = low - ancestor->i_stru_vec.begin();
		else 
			tran_range.second = low - ancestor->i_stru_vec.begin() - 1;
			#ifdef GTEST
				std::cout << "tran_range.first: " << tran_range.first 
						  << " tran_range.second: " << tran_range.second << std::endl;
			#endif
			if (tran_range.first >=0 && tran_range.second >= 0 && tran_range.first <= tran_range.second){
				//assert(tran_range.first >= 0);
				//assert(tran_range.second >= 0);
				max_score_index = ancestor->score_rmq(tran_range.first, tran_range.second);
				int16_t score = (ancestor->i_stru_vec[max_score_index]).score;
				score_que.push(make_pair(score, 
					make_pair(ancestor, tran_range)));
			}
		}
		ancestor = ancestor->parent;
	}
	while(!score_que.empty() && result.size() < k)
	{
		auto top_item = score_que.top();
		score_que.pop();
		pair<ptrdiff_t, ptrdiff_t> old_range = get<1>(top_item.second);
		pair<ptrdiff_t, ptrdiff_t> tran_range1;
		pair<ptrdiff_t, ptrdiff_t> tran_range2;

		Node* top_item_node = top_item.second.first;
		max_score_index = top_item_node->score_rmq(old_range.first, old_range.second);
		//assert(max_score_index >= old_range.first && max_score_index <= old_range.second);
		result.push_back(top_item_node->i_stru_vec[max_score_index].doc_id);
		
		//Node *old_node = get<0>(top_item.second);
		
		tran_range1.first = old_range.first;
		tran_range1.second = max_score_index - 1;
		tran_range2.first = max_score_index + 1;
		tran_range2.second = old_range.second;
		#ifdef GTEST
			std::cout << max_score_index << endl;
			std::cout << std::endl << "old_range.first: " << tran_range1.first << " old_range.second: " << tran_range1.second << std::endl;
			std::cout << "tran_range1.first: " << tran_range1.first << " tran_range1.second: " << tran_range1.second << std::endl;
			std::cout << "tran_range2.first: " << tran_range2.first << " tran_range2.second: " << tran_range2.second << std::endl;
		#endif
		if (tran_range1.first <= tran_range1.second){
			int score_index =  top_item_node->score_rmq(tran_range1.first, tran_range1.second);
			//std::cout << "score_inde: " << score_index << "tran_range1.second: " << tran_range1.second << std::endl;
			int score = ( top_item_node->i_stru_vec).at(score_index).score;
			score_que.push(make_pair(score, make_pair( top_item_node, tran_range1)));
		}

		if (tran_range2.first <= tran_range2.second){
			int score_index =  top_item_node->score_rmq(tran_range2.first, tran_range2.second);
			//std::cout << "score_inde: " << score_index << "tran_range2.second: " << tran_range2.second << std::endl;
			int score = ( top_item_node->i_stru_vec).at(score_index).score;
			score_que.push(make_pair(score, make_pair( top_item_node, tran_range2)));
		}
	}
	auto end = std::chrono::steady_clock::now();
	 #ifdef GTEST
	 	std::cout << std::endl << "using :" << std::chrono::duration<double, ratio<1, 1000000>>(end - start).count() << "microseconds" << std::endl;  
	 #else
	 	std::cout << std::chrono::duration<double, ratio<1, 1000000>>(end - start).count() << std::endl;
	 #endif
	 #ifdef GTEST
	 	std::cout << "Top_K document ids: " << std::endl;
	 	for (auto ite = result.begin(); ite != result.end(); ite++)
			std::cout << doc_names.at(*ite) << " ";
	 	std::cout << std::endl;
	 #endif
	result.clear();
}
}//end top-k
