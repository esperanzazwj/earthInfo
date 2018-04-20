#pragma once
#include<string>
using namespace std;

class NameGenerator
{
public:

	NameGenerator(const string &prefix);
	string Generate();
	string m_Prefix = "HW";
	unsigned int m_NameGenCount = 0;

};

