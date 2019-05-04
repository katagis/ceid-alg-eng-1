#include "testbench.h"
#include "graph_gen.h"

#include "LEDA/graph/graph.h"
#include "LEDA/core/list.h"

#include <iostream>
#include <queue>
#include <set>
#include <array>
#include <ctime>
#include <climits>

using leda::node;
using leda::graph;
using leda::edge;
using leda::list;
using leda::node_array;
using leda::edge_array;


bool GVerbose = false;

enum Color {
	CNone,
	CGreen,
	CBlue
};


// Not actually used anywhere, since we don't need all this info to determine if graph is bipartite
list<node> MyBFS(const graph& Graph, node Start, node_array<int>& OutDist, node_array<edge>& OutPred, edge_array<Color>& OutColorArray) {
	list<node> Result;
	
    std::queue<node> Queue;

	OutDist.init(Graph, INT_MAX);
	OutPred.init(Graph);
	OutColorArray.init(Graph, CNone);

    Queue.push(Start);
	Result.push_back(Start);



	int CurrentDistance = 0;
	node NextIncrement = Start;
	OutDist[Start] = 0;
	
	while (!Queue.empty()) {
        node Front = Queue.front();
        Queue.pop();
		CurrentDistance = OutDist[Front] + 1;


		edge Edge;
		
		forall_inout_edges(Edge, Front) {
			node Node = Graph.target(Edge);
			if (Node == Front) {
				Node = Graph.source(Edge);
			}

			if (OutDist[Node] == INT_MAX) {
				OutDist[Node] = CurrentDistance;
				OutPred[Node] = Edge; 

				OutColorArray[Edge] = CurrentDistance % 2 == 0 ? CGreen : CBlue;

				Result.push_back(Node);
				Queue.push(Node);
			}
		}
    }

	return Result;
}

bool MyIsBipartite(const graph& Graph, list<node>& PartA, list<node>& PartB) {
	list<node> Result;

	node_array<Color> NodeColors;
	NodeColors.init(Graph, CNone);

	std::queue<node> Queue;
	node FirstNode = Graph.first_node();

	Color CurrentColor = CBlue;
	NodeColors[FirstNode] = CurrentColor;
	Queue.push(FirstNode);
	
	while (!Queue.empty()) {
		node Front = Queue.front();
		Queue.pop();

		CurrentColor = NodeColors[Front] == CBlue ? CGreen : CBlue;

		node Node;
		forall_adj_nodes(Node, Front) {
			if (NodeColors[Node] == CNone) {
				NodeColors[Node] = CurrentColor;
				Queue.push(Node);
			}
			else if (NodeColors[Node] != CurrentColor) {
				return false;
			}
		}
	}

	// TODO: Implement disjointed graphs:
	// Count nodes actually colored and select an uncolored one if not all where colored.

	// Assume no non-connected nodes. 
	// Perform a different loop to reduce cache misses when accessing PartA and PartB
	// Also since we dont care returning anything in the lists this ensures no memory
	// operations are done if the graph is not bipartite.
	node Node;
	forall_nodes(Node, Graph) {
		if (NodeColors[Node] == CBlue) {
			PartA.push_back(Node);
		}
		else {
			PartB.push_back(Node);
		}
	}

	return true;
}
/*

bool IsListSame(const list<node>& ListA, const list<node>& ListB) {
	if (ListA.size() != ListB.size()) {
		return false;
	}
	std::set<node> SetA;

	node Node;
	forall(Node, ListA) {
		SetA.insert(Node);
	}

	forall(Node, ListB) {
		if (SetA.count(Node) == 0) {
			return false;
		}
	}
	return true;
}

bool TestGraph(const graph& Graph, int TestNum, const std::string& TestName) {
	if (Graph.empty()) {
		return true;
	}

	list<node> MyA, MyB;
	list<node> LedaA, LedaB;
	bool MyResult;
	bool LedaResult;

	
	Bench.StartTest();
	LedaResult = leda::Is_Bipartite(Graph, LedaA, LedaB);
	Bench.SwitchTest();
	MyResult = MyIsBipartite(Graph, MyA, MyB);
	Bench.StopTest();
	

	bool SameResult = MyResult == LedaResult;
	
	// If both results are false, skip checking the list
	bool TestResult = SameResult && (MyResult == false || (IsListSame(MyA, LedaA) && IsListSame(MyB, LedaB)));

	if (GVerbose) {
		std::cout << "Ret\t> My: " << MyResult << " | Leda: " << LedaResult << "\n";
		
		// If both calculated false we should just not print anything
		if (MyResult || LedaResult) {
			std::cout << "Size A\t> My: " << MyA.size() << " | Leda: " << LedaA.size() << "\n";
			std::cout << "Size B\t> My: " << MyB.size() << " | Leda: " << LedaB.size() << "\n";


			std::cout << "MyA  : " << MyA << "\n";
			std::cout << "LedaA: " << LedaA << "\n";

			std::cout << "MyB  : " << MyB << "\n";
			std::cout << "LedaB: " << LedaB << "\n";
		}
	}


	const int MaxNameLen = 15;
	std::ostringstream PaddedName;
	PaddedName <<  ": " << TestName.substr(0, MaxNameLen - 2);
	std::cout << "# Test " << std::setw(2) << TestNum 
			<< std::left << std::setw(MaxNameLen) << PaddedName.str() << "| " << std::right; 
	
	if (TestResult){
		std::string ResultStr = MyResult == true ? "yes" : "no ";
		std::cout << ResultStr;
		Bench.PrintLast();
	}
	else {
		std::cout << "# Test failed. Results are different.\n";
	}

	return TestResult;
}
*/
int main() {
	graph Graphs[26];

	GenerateTestGraphs(Graphs, 26);
	std::cout << "Finished Generating Tests...\nStarting testing...\n";

	bool PassedTests = true;
	for (int i = 0; i < 26; ++i) {
		PassedTests &= TestGraph(Graphs[i], i, "some test");
	}

	if (PassedTests) {
		std::cout << "===================\nAll tests PASSED.\n";
		Bench.Print();
	}
	else {
		std::cout << "===================\nAtleast one test failed.\n";
	}
}
