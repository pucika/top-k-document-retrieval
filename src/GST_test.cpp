#include <iostream>
#include <chrono>
#include "gst.h"

using namespace top_k;

int main(int argc, char* argv[])
{
	GST gst;
	//gst.construct(argv[1]);
	gst.construct("../docs/");
	// #ifdef TEST
	// 	std::cout << std::endl << "verify: " << gst.verify() << std::endl;
	// #endif
	string pattern;
	int k;
	while(true){
		#ifdef GTEST
			std::cout << "input pattern and k :" << std::endl;
		#endif
		getline(cin, pattern);
		if (pattern == "q")
			break;
		//std::cout << pattern << std::endl;
		std::cin >> k;
		std::cin.ignore();
		gst.search(pattern, k);
	} 
	return 0;
}

