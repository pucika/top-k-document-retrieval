#include <iostream>
#include <fstream>
#include <string>
#include <random>
using namespace std;

const int FILES = 512;

static default_random_engine e;
static uniform_int_distribution<unsigned short> gen(1, 255);
const string pattern = "abcde";

int main()
{
	for(int i = 0; i != FILES; i++)
	{

		string name = to_string(i) + ".txt";
		ofstream out(name, ofstream::app);
		for(int i = 0; i != 1024; i++)
		{
			char ch = gen(e);
			out << ch;
		}
		for(int j = 0; j != i; j++)
		{
			out << pattern;
		}
		out.close();
	}
}