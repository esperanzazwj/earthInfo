#include "NameGenerator.h"
using namespace std;

NameGenerator::NameGenerator(const string &prefix) :m_Prefix(prefix)
{

}

string NameGenerator::Generate()
{
	// init a stack
	//int stack[100];
	//int sp = 0;

	//unsigned int temp = m_NameGenCount++;
	//while(temp)
	//{
	//	// push stack
	//	stack[sp++] = temp%10;
	//	temp/=10;
	//}

	//String res = m_Prefix;

	//while(sp) // stack not empty
	//{
	//	char c = '0' + stack[--sp];
	//	res += c;
	//}
	return m_Prefix + "_" + to_string(m_NameGenCount);
}
