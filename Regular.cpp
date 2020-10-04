#include "Regular.h"
#include<deque>
#include<algorithm>
using namespace std;
using namespace placeholders;
unordered_set<char> Regular::symbol{ '(',')','|','*','.' };
Regular::Regular(string reg) :parsing(reg)
{
	DFA = minimizeDFA(createDFA(createNFA(parsing.postfixExpr())));
	eliminateDeadState();
}

bool Regular::match(string str)
{
	bool is_match = true;
	shared_ptr<StateNode> ptr = DFA->start;
	for (char c : str)
	{
		string char_str{ c };
		if (ptr->adjacent.find(char_str) != ptr->adjacent.end())  //Can match the character
		{
			ptr = ptr->adjacent[char_str][0];
		}
		else
		{
			is_match = false;
			break;
		}
	}
	if (is_match && (DFA->ends.find(ptr->number) != DFA->ends.end()))  //Determine whether the termination state is the acceptance state
		return true;
	return false;
}

shared_ptr<Regular::StateGraph> Regular::singleMatch(string c)
{
	auto start = make_shared<StateNode>(to_string(stateNumber++));
	auto end = make_shared<StateNode>(to_string(stateNumber++));
	start->adjacent[c] = vector < shared_ptr<StateNode>>{ end };
	stateTable[start->number] = start;
	stateTable[end->number] = end;
	return make_shared<StateGraph>(start, end);
}

shared_ptr<Regular::StateGraph> Regular::orMatch(shared_ptr<StateGraph> graph_lt, shared_ptr<StateGraph> graph_rt)
{
	auto start = make_shared<StateNode>(to_string(stateNumber++));
	start->adjacent[" "] = vector<shared_ptr<StateNode>>{ graph_lt->start,graph_rt->start };
	auto end = make_shared<StateNode>(to_string(stateNumber++));
	graph_lt->end->adjacent[" "] = vector<shared_ptr<StateNode>>{ end };
	graph_rt->end->adjacent[" "] = vector<shared_ptr<StateNode>>{ end };
	stateTable[start->number] = start;
	stateTable[end->number] = end;
	return make_shared<StateGraph>(start, end);
}

shared_ptr<Regular::StateGraph> Regular::andMatch(shared_ptr<StateGraph> graph_lt, shared_ptr<StateGraph> graph_rt)
{
	/*Make graph_rt's start is graph_lt' end and Integrated adjacency list
	 *整合邻接表*/

	for (auto node : graph_rt->start->adjacent)
	{
		if (graph_lt->end->adjacent.find(node.first) != graph_lt->end->adjacent.end())
		{
			graph_lt->end->adjacent[node.first].insert(graph_lt->end->adjacent[node.first].end(), node.second.begin(), node.second.end()); 
			sort(graph_lt->end->adjacent[node.first].begin(), graph_lt->end->adjacent[node.first].end(), [](const shared_ptr<StateNode>& n1, const shared_ptr<StateNode>&n2) {return n1->number < n2->number; });
			auto end_unique = unique(graph_lt->end->adjacent[node.first].begin(), graph_lt->end->adjacent[node.first].end(), [](const shared_ptr<StateNode>& n1, const shared_ptr<StateNode>&n2) {return n1->number == n2->number; });
			graph_lt->end->adjacent[node.first].erase(end_unique, graph_lt->end->adjacent[node.first].end());   //Delete duplicate nodes
		}
		else
		{
			graph_lt->end->adjacent[node.first] = graph_rt->start->adjacent[node.first];
		}
	}
	stateTable.erase(graph_rt->start->number);
	graph_rt->start = graph_lt->end;
	return make_shared<StateGraph>(graph_lt->start, graph_rt->end);
}

shared_ptr<Regular::StateGraph> Regular::closureMatch(shared_ptr<StateGraph> graph)
{
	auto start = make_shared<StateNode>(to_string(stateNumber++));
	auto end = make_shared<StateNode>(to_string(stateNumber++));
	start->adjacent[" "] = vector<shared_ptr<StateNode>>{ graph->start,end };
	graph->end->adjacent[" "] = vector<shared_ptr<StateNode>>{ graph->start,end };
	stateTable[start->number] = start;
	stateTable[end->number] = end;
	return make_shared<StateGraph>(start, end);
}

shared_ptr<Regular::StateGraph> Regular::createNFA(string suffix)
{
	stack<shared_ptr<StateGraph>> ptr_stack;
	shared_ptr<StateGraph> graph1(new StateGraph());
	shared_ptr<StateGraph> graph2(new StateGraph());
	shared_ptr<StateGraph> nullGraph(new StateGraph());
	for (char c:suffix)
	{
		if (symbol.find(c) == symbol.end())
		{
			if (c != '#')  //Ignore meaningless characters
				ptr_stack.push(singleMatch(string{ c }));
			else
				ptr_stack.push(nullGraph);
		}
		else
		{
			for (int i = 0; i < 2; ++i)
			{
				if (ptr_stack.top() != nullGraph)
				{
					if (graph1->start == nullptr&&graph1->end == nullptr)   //graph1 is empty
						graph1 = ptr_stack.top();
					else
						graph2 = ptr_stack.top();
				}
				ptr_stack.pop();
			}
			if (c!='('&&c!=')')   //Ignore '(' and ')'
			{
				switch (c)    //Warning:The order of the parameters is (graph2,graph1),the order cannot be exchanged
				{
				case '|':graph1 = orMatch(graph2, graph1); break;
				case '.':graph1 = andMatch(graph2, graph1); break;
				case '*':graph1 = closureMatch(graph1); break;
				default:
					break;
				}
			}
			ptr_stack.push(graph1);
			graph1 = graph2 = nullGraph;
		}

	}
	return ptr_stack.top();
}

shared_ptr<Regular::StateDFA> Regular::createDFA(shared_ptr<StateGraph> nfa)
{
	shared_ptr<StateDFA> dfa(new StateDFA());
	deque<vector<string>> state;
	auto function = bind(&Regular::hasher, this, _1);
	unordered_map<vector<string>, shared_ptr<StateNode>, decltype(function)> stateVec_Mapping(10, function);
	set<string> firstStateSet = getBlankClosure(nfa->start);
	vector<string> firstStateVec(firstStateSet.begin(), firstStateSet.end());
	char init_char = 'A';
	int count = 0;
	dfa->start = make_shared<StateNode>(string{ char(init_char + count++) });
	stateVec_Mapping[firstStateVec] = dfa->start;
	state.push_back(firstStateVec);
	unordered_set<string> states;
	for (auto node : stateTable)
		states.insert(node.first);
	while (!states.empty())
	{
		for (char c : parsing.alphabet)
		{
			set<string> result1 = getMove(state.front(), c);
			set<string> temp;
			for (string str : result1)
			{
				set<string> result2 = getBlankClosure(stateTable[str]);
				temp.insert(result2.begin(), result2.end());
			}
			if (temp.empty())  //Skip when empty character set
				continue;
			vector<string> newStateVec(temp.begin(), temp.end());
			bool flag = false;     //Determine whether to push the stack
			if (stateVec_Mapping.find(newStateVec) == stateVec_Mapping.end())  //The state set is a new state set
			{
				stateVec_Mapping[newStateVec] = make_shared<StateNode>(string{ char(init_char + count++) });
				flag = true;
			}
			/*Determine whether the state set contains an acceptance state
			 *If the state set contains the acceptance state, the set is also the acceptance state set*/
			for (string str : newStateVec)
			{
				if (str == nfa->end->number)
				{
					dfa->ends.insert({ stateVec_Mapping[newStateVec]->number,stateVec_Mapping[newStateVec] });
					break;
				}
			}
			stateVec_Mapping[state.front()]->adjacent[string{ c }] = vector < shared_ptr<StateNode>>{ stateVec_Mapping[newStateVec] };
			if(flag)
				state.push_back(newStateVec);
		}
		for (string str : state.front())  //Delete flagged status 删除已经标记的状态
		{
			if (states.find(str) != states.end())
				states.erase(str);
		}
		state.pop_front();
	}
	stateTable.clear();
	for (auto node : stateVec_Mapping)  //Update status table
	{
		stateTable[node.second->number] = node.second;
	}
	return dfa;
}

set<string> Regular::getBlankClosure(shared_ptr<StateNode> state)
{
	deque<shared_ptr<StateNode>> deque;
	set<string> result;
	deque.push_back(state);
	while (!deque.empty())
	{
		result.insert(deque.front()->number);
		if (deque.front()->adjacent.find(" ") != deque.front()->adjacent.end())
		{
			for (auto ptr : deque.front()->adjacent[" "])
			{
				if (result.find(ptr->number) == result.end())
					deque.push_back(ptr);
			}
		}
		deque.pop_front();
	}
	return result;
}

set<string> Regular::getMove(vector<string> stateVec, char c)
{
	set<string> result;
	for (string str : stateVec)
	{
		if (stateTable[str]->adjacent.find(string{ c }) != stateTable[str]->adjacent.end())
		{
			for (auto node : stateTable[str]->adjacent[string{ c }])
			{
				result.insert(node->number);
			}
		}
	}
	return result;
}

shared_ptr<Regular::StateDFA> Regular::minimizeDFA(shared_ptr<StateDFA> dfa)
{
	shared_ptr<StateDFA> m_DFA(new StateDFA());
	vector<vector<string>> equStateTable;
	set<shared_ptr<StateNode>> div1, div2;
	for (auto node : stateTable)
	{
		if (dfa->ends.find(node.first) == dfa->ends.end())
			div1.insert(node.second);
		else
			div2.insert(node.second);
	}
	divideEquState(div1,equStateTable);
	divideEquState(div2, equStateTable);
	for (auto vec : equStateTable)
	{
		for (int i = 1; i < vec.size(); ++i)   //Take the first state in the equivalence partition as the representative state of its set
			stateTable.erase(vec[i]);
		for (auto node1 : stateTable)
		{
			for (auto &node2 : node1.second->adjacent)
			{
				if ((find(vec.begin(), vec.end(), node2.second[0]->number) != vec.end())&&(node2.second[0]->number!=vec[0]))  //Replace all states in the state table with representative states
				{
					node2.second[0] = stateTable[vec[0]];
				}
			}
		}
	}
	/*Find the starting state and receiving state of the NFA*/
	bool is_break = false;
	for (auto node : equStateTable)
	{
		if (find(node.begin(), node.end(), dfa->start->number) != node.end())
		{
			m_DFA->start = stateTable[node[0]];
			is_break = true;
			break;
		}
	}
	if (!is_break)
		m_DFA->start = dfa->start;
	for (auto end : dfa->ends)  //Unordered_map don’t add duplicates so don’t worry
	{
		bool is_break = false;
		for (auto node : equStateTable)
		{
			if (find(node.begin(), node.end(), end.second->number) != node.end())
			{
				m_DFA->ends.insert({ stateTable[node[0]]->number,stateTable[node[0]] });
				is_break = true;
				break;
			}
		}
		if (!is_break)
			m_DFA->ends.insert(end);
	}
	return m_DFA;

}

void Regular::divideEquState(set<shared_ptr<StateNode>> divVec, vector<vector<string>>& equDivVecs)
{

	set<shared_ptr<StateNode>> div1(divVec);
	/*A collection of elements stored in div1 that can be transferred to the same state set under any input character condition
	 *div1中保存在任何输入字符的条件下都可以转移到同一状态集的元素集合*/
	for (char c:parsing.alphabet)
	{
		set<shared_ptr<StateNode>> div_lt;
		set<shared_ptr<StateNode>> div_rt;
		for (auto node : divVec)
		{
			string char_str{ c };
			if (node->adjacent.find(char_str)!=node->adjacent.end())  //The symbol exists
			{
				if (divVec.find(node->adjacent[char_str][0]) != divVec.end())  //Distinguish left and right partition set
					div_lt.insert(node);
				else
					div_rt.insert(node);
			}
			else
			{
				div_rt.insert(node);
			}
		}
		/*Get intersection*/
		vector<shared_ptr<StateNode>> vec_lt, vec_rt;
		set_intersection(div1.begin(), div1.end(), div_lt.begin(), div_lt.end(), back_inserter(vec_lt));
		set_intersection(div1.begin(), div1.end(), div_rt.begin(), div_rt.end(), back_inserter(vec_rt));
		div1 = set<shared_ptr<StateNode>>((vec_lt.size() < vec_rt.size() ? vec_rt : vec_lt).begin(), (vec_lt.size() < vec_rt.size() ? vec_rt : vec_lt).end());  //选择交集较大的部分
	}
	vector<shared_ptr<StateNode>> temp;
	set_difference(divVec.begin(), divVec.end(), div1.begin(), div1.end(), back_inserter(temp));
	set<shared_ptr<StateNode>> div2(temp.begin(), temp.end());  //Another partition set Warning: The partition set is not necessarily an equivalent set
	if (div1.size()==divVec.size())   //Cannot be subdivided, it is an equivalent state collection
	{
		if (div1.size() < 2)   //No need to merge when there are less than 2 states in the set
			return;
		vector<string> vec;
		for (auto ptr : div1)
		{
			vec.push_back(ptr->number);
		}
		equDivVecs.push_back(vec);
		return;
	}
	else
	{
		divideEquState(div1, equDivVecs);
		divideEquState(div2, equDivVecs);
	}

}

void Regular::eliminateDeadState()
{
	for (auto node : stateTable)
	{
		bool is_break = false;
		for (char c : parsing.alphabet)
		{
			string chars{ c };
			if (node.second->adjacent.find(chars) != node.second->adjacent.end())  //That side exists
			{
				if (node.second->adjacent[chars][0]->number != node.first)
				{
					is_break = true;
					break;
				}
			}
		}
		if (!is_break&&(DFA->ends.find(node.first)==DFA->ends.end()))  //If it is not exited early and is not in the accepting state, the state is dead and all outgoing edges can be cleared
		{
			node.second->adjacent.clear();
		}
	}
}
