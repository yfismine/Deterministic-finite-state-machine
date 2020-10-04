#include"Regular.h"
#include<iostream>
#include<algorithm>
using namespace std;
int main(void)
{
	/**********************test******************/
	Regular regular1("aa*|bb*");
	cout << boolalpha << regular1.match("aaa") << endl;
	Regular regular2("(a|b)*abb");
	cout << boolalpha << regular2.match("aaaabababb") << endl;
	return 0;
}