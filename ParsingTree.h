#ifndef _PARSINGTREE_H_
#define _PARSINGTREE_H_
#include<vector>
#include<stack>
#include<string>
#include<set>
#include<unordered_map>
#include<memory>
using std::vector;
using std::stack;
using std::string;
using std::set;
using std::unordered_map;
using std::shared_ptr;
class ParsingTree
{
	friend class Regular;
public:
	ParsingTree(string reg);
	string postfixExpr(); //Return the generated postfix expression
private:
	struct ParsingNode
	{
		char character = ' ';
		shared_ptr<ParsingNode> left = nullptr;
		shared_ptr<ParsingNode> right = nullptr;
		ParsingNode() = default;
		ParsingNode(const char m_char, shared_ptr<ParsingNode> lt = nullptr, shared_ptr<ParsingNode> rt = nullptr)
			:character(m_char), left(lt), right(rt) {};
	};
	shared_ptr<ParsingNode> root;
	shared_ptr<ParsingNode> nullNode;
	set<char> alphabet;  //Character set, it will be used by its friend class when constructing DFA and NFA
	unordered_map<char, unordered_map<char, bool>> mapping;  //Priority mapping table
	void creatPriorityMapping();
	void creatParsingTree(string reg);
	void postfixExpr(shared_ptr<ParsingNode> ptr, string& str);

	/*Regularize regular expressions, such as adding implicit concatenation operations, and converting unary operators to binary by adding null characters
	 *���滯������ʽ�����������ʽ���������㡢ͨ����ӿ��ַ�ת��һԪ�����Ϊ��Ԫ
	 */
	void normalReg(string& reg); 
};

#endif // !_PARSINGTREE_H_