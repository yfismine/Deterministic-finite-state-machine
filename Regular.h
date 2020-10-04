#ifndef _REGULAR_H_
#define _REGULAR_H_
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>
#include<memory>
#include<functional>
#include"ParsingTree.h"
using std::string;
using std::vector;
using std::unordered_map;
using std::shared_ptr;
using std::unordered_set;
using std::hash;
class Regular
{
public:
	Regular(string reg);
	bool match(string str);
private:
	struct StateNode
	{
		string number;
		unordered_map<string, vector<shared_ptr<StateNode>>> adjacent;
		StateNode() = default;
		StateNode(string num, unordered_map<string, vector<shared_ptr<StateNode>>> adj=unordered_map<string,vector<shared_ptr<StateNode>>>())
			:number(num), adjacent(adj) {};
	};
	struct StateGraph
	{
		shared_ptr<StateNode> start = nullptr;
		shared_ptr<StateNode> end = nullptr;
		StateGraph() = default;
		StateGraph(shared_ptr<StateNode> s, shared_ptr<StateNode> e)
			:start(s), end(e) {};
	};
	struct StateDFA
	{
		shared_ptr<StateNode> start = nullptr;
		unordered_map<string,shared_ptr<StateNode>> ends;
	};
	size_t hasher(vector<string> vec)
	{
		string strs;
		for (auto str : vec)
		{
			strs += str;
		}
		return hash<string>()(strs);
	}
	ParsingTree parsing;
	shared_ptr<StateDFA> DFA;
	static unordered_set<char> symbol;   //Operator symbol collection
	unordered_map<string,shared_ptr<StateNode>> stateTable;
	int stateNumber = 0;
	shared_ptr<StateGraph> singleMatch(string c);
	shared_ptr<StateGraph> orMatch(shared_ptr<StateGraph> graph_lt, shared_ptr<StateGraph> graph_rt);
	shared_ptr<StateGraph> andMatch(shared_ptr<StateGraph> graph_lt, shared_ptr<StateGraph> graph_rt);
	shared_ptr<StateGraph> closureMatch(shared_ptr<StateGraph> graph);
	shared_ptr<StateGraph> createNFA(string suffix);
	shared_ptr<StateDFA> createDFA(shared_ptr<StateGraph> nfa);
	set<string> getBlankClosure(shared_ptr<StateNode> state);
	set<string> getMove(vector<string> stateVec, char c);
	shared_ptr<StateDFA> minimizeDFA(shared_ptr<StateDFA> dfa);
	void divideEquState(set<shared_ptr<StateNode>> divVec, vector<vector<string>>& equDivVecs);   //Get equivalent state partition
	void eliminateDeadState();
};

#endif // !_REGULAR_H_

