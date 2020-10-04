#include "ParsingTree.h"
using namespace std;
ParsingTree::ParsingTree(string reg)
{
	nullNode = make_shared<ParsingNode>(' ', nullptr, nullptr);
	nullNode->left = nullNode;
	nullNode->right = nullNode;
	creatPriorityMapping();
	for (char c : reg)
	{
		if (mapping.find(c) == mapping.end())
			alphabet.insert(c);
	}
	creatParsingTree(reg);
}

string ParsingTree::postfixExpr()
{
	string str;
	postfixExpr(root, str);
	return str;
}

void ParsingTree::creatPriorityMapping()
{
	mapping['.']['.'] = false;
	mapping['.']['*'] = false;
	mapping['.']['|'] = true;
	mapping['.']['('] = false;
	mapping['.'][')'] = false;
	mapping['*']['.'] = true;
	mapping['*']['*'] = false;
	mapping['*']['|'] = true;
	mapping['*']['('] = false;
	mapping['*'][')'] = false;
	mapping['|']['.'] = false;
	mapping['|']['*'] = false;
	mapping['|']['|'] = false;
	mapping['|']['('] = false;
	mapping['|'][')'] = false;
	mapping['(']['.'] = true;
	mapping['(']['*'] = true;
	mapping['(']['|'] = true;
	mapping['(']['('] = false;
	mapping['('][')'] = false;
	mapping[')']['.'] = false;
	mapping[')']['*'] = false;
	mapping[')']['|'] = false;
	mapping[')']['('] = false;
	mapping[')'][')'] = false;
}

void ParsingTree::creatParsingTree(string reg)
{
	normalReg(reg);
	stack<shared_ptr<ParsingNode>> char_stack;
	stack<shared_ptr<ParsingNode>> operator_stack;
	for (char c : reg)
	{
		if (mapping.find(c) == mapping.end())    //The character is not an operator
		{
			auto chars = make_shared<ParsingNode>(c, nullNode, nullNode);
			char_stack.push(chars);
		}
		else
		{
			while (!operator_stack.empty()&&!mapping[c][operator_stack.top()->character])
			{
				operator_stack.top()->right = char_stack.top();
				char_stack.pop();
				operator_stack.top()->left = char_stack.top();
				char_stack.pop();
				char_stack.push(operator_stack.top());
				operator_stack.pop();
			}
			auto operators = make_shared<ParsingNode>(c, nullNode, nullNode);
			operator_stack.push(operators);
		}
	}
	while (!operator_stack.empty())  //Ensure that all operators in the operator stack are used
	{
		operator_stack.top()->right = char_stack.top();
		char_stack.pop();
		operator_stack.top()->left = char_stack.top();
		char_stack.pop();
		char_stack.push(operator_stack.top());
		operator_stack.pop();
	}
	root = char_stack.top();
	char_stack.pop();
}

void ParsingTree::postfixExpr(shared_ptr<ParsingNode> ptr, string& str)  //Post-order traversal of expression tree
{
	if (ptr == nullNode)
		return;
	else
	{
		postfixExpr(ptr->left, str);
		postfixExpr(ptr->right, str);
		str.push_back(ptr->character);
	}
}

void ParsingTree::normalReg(string& reg)
{
	bool flag = true;
	for (auto ptr = reg.begin(); ptr != reg.end(); ++ptr)
	{
		if (*ptr == '(')  //Adding meaningless characters turns unary operators into binary operators
		{
			ptr = reg.insert(ptr, '#');
			++ptr;
		}
		else if (*ptr == ')' || *ptr == '*')
		{
			ptr = reg.insert(++ptr, '#');
		}
	}
	for (auto ptr = reg.begin(); ptr != reg.end(); ++ptr)
	{
		if (mapping.find(*ptr) == mapping.end())
		{
			if (!flag)  //Add implicit concatenation operator
			{
				ptr = reg.insert(ptr, '.');
				++ptr;
			}
			else
			{
				flag = false;
			}
		}
		else
		{
			if (!flag)
				flag = true;
		}
	}
}
