#include "LEDA/graph/graph.h"
#include "LEDA/core/list.h"

#include <iostream>
#include <queue>
#include <set>


#ifdef _MSC_VER
#define MODERN
#endif

#ifndef VERBOSITY
#define VERBOSITY 2
#endif

using leda::node;
using leda::graph;
using leda::edge;
using leda::list;
using leda::node_array;
using leda::edge_array;

#define INT_INF 999999





//
// Graph: A leda graph to perform bfs on
// Start: A leda node used as starting point for our bfs
// OutDist: Output distance for each node
// OutPred: Store the edge that visited each node
// Return: All visited nodes
//

void GraphPrint(const graph& Graph);

std::ostream& operator<< (std::ostream& Stream, const node& Node) {
	Stream << Node->id();
	return Stream;
}

std::ostream& operator<< (std::ostream& Stream, const edge& Edge) {
	Stream << Edge->terminal(0) << "->" << Edge->terminal(1);
	return Stream;
}

std::ostream& operator<< (std::ostream& Stream, list<node>& List) {
	node Node;
	forall(Node, List) {
		std::cout << Node << ", ";
	}
	return Stream;
}

enum Color {
	CNone,
	CGreen,
	CBlue
};

list<node> MyBFS(const graph& Graph, node Start, node_array<int>& OutDist, node_array<edge>& OutPred, edge_array<Color>& OutColorArray) {
	list<node> Result;
	
    std::queue<node> Queue;

	OutDist.init(Graph, INT_INF);
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

			if (OutDist[Node] == INT_INF) {
				OutDist[Node] = CurrentDistance;
				OutPred[Node] = Edge; 

				OutColorArray[Edge] = CurrentDistance % 2 ? CGreen : CBlue;

				Result.push_back(Node);
				Queue.push(Node);
			}
		}
    }

	return Result;
}

// Assumes List 
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

	// Assume no non-connected nodes. 
	// Perform a different loop to reduce cache misses when accessing PartA and PartB
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

bool IsListSame(const list<node>& ListA, const list<node>& ListB) {
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

bool TestGraph(const graph& Graph) {
	if (Graph.empty()) {
		return true;
	}

	list<node> MyA, MyB;
	list<node> LedaA, LedaB;

	bool MyResult = MyIsBipartite(Graph, MyA, MyB);
	bool LedaResult = leda::Is_Bipartite(Graph, LedaA, LedaB);

	const bool MatchingSize = MyA.size() == LedaA.size() && MyB.size() == LedaB.size();

	const bool TestResult = MatchingSize && IsListSame(MyA, LedaA) && IsListSame(MyB, LedaB);

#if VERBOSITY == 2
	std::cout << "Ret\t> My: " << MyResult << " | Leda: " << LedaResult << "\n";
	std::cout << "Size A\t> My: " << MyA.size() << " | Leda: " << LedaA.size() << "\n";
	std::cout << "Size B\t> My: " << MyB.size() << " | Leda: " << LedaB.size() << "\n";


	std::cout << "MyA  : " << MyA << "\n";
	std::cout << "LedaA: " << LedaA << "\n";

	std::cout << "MyB  : " << MyB << "\n";
	std::cout << "LedaB: " << LedaB << "\n";
#endif
#if VERBOSITY >= 1
	std::cout << "\nTEST RESULT: ";
	if (TestResult){
		std::cout << "PASS\n\n";
	}
	else {
		std::cout << "FAIL\n\n";
	}
#endif
	return TestResult;
}

void GenerateTestGraphs(graph* Graphs, int Count);

int main() {

	graph Graphs[10];

	GenerateTestGraphs(Graphs, 10);

	bool PassedTests = true;
	for (int i = 0; i < 10; ++i) {
		PassedTests &= TestGraph(Graphs[i]);
	}

	if (PassedTests) {
		std::cout << "All tests PASSED.\n";
	}
	else {
		std::cout << "Atleast one test failed.\n";
	}
	

	getchar();
}

void Connect(graph& Graph, const std::vector<node>& Nodes, int NodeIndex1, int NodeIndex2) {
	Graph.new_edge(Nodes[NodeIndex1], Nodes[NodeIndex2]);
	Graph.new_edge(Nodes[NodeIndex2], Nodes[NodeIndex1]);

}


void Gen_DebugGraph(graph& Graph) {
	std::vector<node> Nodes;

	for (int i = 0; i < 7; ++i) {
		node Node = Graph.new_node();
		Nodes.push_back(Node);
	}

	Connect(Graph, Nodes, 0, 2);
	Connect(Graph, Nodes, 1, 2);
	Connect(Graph, Nodes, 2, 3);
	Connect(Graph, Nodes, 3, 4);
	Connect(Graph, Nodes, 3, 5);
	Connect(Graph, Nodes, 5, 6);
}

void GenerateTestGraphs(graph* Graphs, int Count) {

	switch (Count) {
	default:

		// Fallthrough
	case 2:
		Gen_DebugGraph(Graphs[1]);

		// Fallthrough
	case 1:
		Gen_DebugGraph(Graphs[0]);

		// Fallthrough
	case 0:
		;
	}
}

void GraphPrint(const graph& Graph) {
	int i = 0;

	edge e;
	node v;

	node_array<int> A(Graph);

	std::cerr << "Pastable to: https:dreampuf.github.io/GraphvizOnline\n";

	forall_nodes(v, Graph) {
		A[v] = i++;
	}
	std::cerr << "digraph G {" << "\n";
	forall_edges(e, Graph) {
		std::cerr << A[Graph.source(e)] << " -> " << A[Graph.target(e)] << ";" << "\n";
	}
	std::cerr << "}" << "\n";
	
}