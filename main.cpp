#include "LEDA/graph/graph.h"
#include "LEDA/core/list.h"

#include <iostream>
#include <queue>
#include <set>
#include <array>
#include <algorithm>


#ifdef MODERNCPP
#include <chrono>
#endif

#ifndef VERBOSITY
#define VERBOSITY 1
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

namespace ch = std::chrono;


struct Benchmark {
private:
	ch::time_point<ch::system_clock> StartTime;

	std::vector<long long> MyTime;
	std::vector<long long> LedaTime;

	long long GetCurrent() const {
		return ch::duration_cast<ch::microseconds>(ch::system_clock::now() - StartTime).count();
	}
	std::string TimestepStr = " micros";

	void PrintBenchLine(long long MyT, long long LedaT) {
		std::cout << " | Mine: " << std::setw(8) << MyT << TimestepStr
			<< "\t| Leda: " << std::setw(8) << LedaT << TimestepStr;

		long long Hi = std::max(std::max(MyT, LedaT), 1ll);
		long long Lo = std::min(MyT, LedaT);
		
		int Percent = 100 - std::round(((float)Lo / Hi) * 100);
		long long AbsDiff = Hi - Lo;

		std::string Who = MyT < LedaT ? "Mine" : "Leda";
		std::cout << " \t" << Who << " is faster by: " << Percent << "% (" << std::setw(8) << AbsDiff << TimestepStr << ")\n" ;
	}

public:
	void StartTest() {
		StartTime = ch::system_clock::now();
	}

	// The leda test has finished and my test is starting
	void SwitchTest() {
		long long Duration = GetCurrent();
		LedaTime.push_back(Duration);
		StartTime = ch::system_clock::now();
	}

	void StopTest() {
		long long Duration = GetCurrent();
		MyTime.push_back(Duration);
	}

	void PrintLast() {
		int Index = MyTime.size() - 1;
		PrintBenchLine(MyTime[Index], LedaTime[Index]);
	}

	void Print() {
		long long MyTotal = 0;
		long long LedaTotal = 0;
		for (int i = 0; i < MyTime.size(); ++i) {
			std::cout << "Test " << i << ":";
			PrintBenchLine(MyTime[i], LedaTime[i]);

			MyTotal += MyTime[i];
			LedaTotal += LedaTime[i];
		}

		std::cout << "\nTotals:";
		PrintBenchLine(MyTotal, LedaTotal);
	}
};

static Benchmark Bench;

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

bool TestGraph(const graph& Graph) {
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
	

	// Both results where false, no point in checking the lists.
	bool SameResult = MyResult == LedaResult;
	
	bool TestResult = SameResult && (MyResult == false || (IsListSame(MyA, LedaA) && IsListSame(MyB, LedaB)));

#if VERBOSITY == 2
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
#endif
#if VERBOSITY >= 1
	if (TestResult){
		std::cout << "# pass | ";
		Bench.PrintLast();
	}
	else {
		std::cout << "# fail\n";
	}
#endif
	return TestResult;
}

void GenerateTestGraphs(graph* Graphs, int Count);

int main() {

	graph Graphs[26];

	GenerateTestGraphs(Graphs, 26);

	std::cout << "Finished Generating Tests...\nStarting testing...\n";

	bool PassedTests = true;
	for (int i = 0; i < 26; ++i) {
		PassedTests &= TestGraph(Graphs[i]);
	}


	if (PassedTests) {
		std::cout << "===================\nAll tests PASSED.\n\n";
		Bench.Print();
	}
	else {
		std::cout << "===================\nAtleast one test failed.\n";
	}


	getchar();
}


void ConnectIndex(graph& Graph, const std::vector<node>& Nodes, int NodeIndex1, int NodeIndex2) {
	Graph.new_edge(Nodes[NodeIndex1], Nodes[NodeIndex2]);
	Graph.new_edge(Nodes[NodeIndex2], Nodes[NodeIndex1]);
}

void Connect(graph& Graph, const node& Node1, const node& Node2) {
	Graph.new_edge(Node1, Node2);
	Graph.new_edge(Node2, Node1);
}

void Gen_DebugGraph(graph& Graph) {
	std::vector<node> Nodes;

	for (int i = 0; i < 7; ++i) {
		node Node = Graph.new_node();
		Nodes.push_back(Node);
	}

	ConnectIndex(Graph, Nodes, 0, 2);
	ConnectIndex(Graph, Nodes, 1, 2);
	ConnectIndex(Graph, Nodes, 2, 3);
	ConnectIndex(Graph, Nodes, 3, 4);
	ConnectIndex(Graph, Nodes, 3, 5);
	ConnectIndex(Graph, Nodes, 5, 6);
}

void Gen_Circle(graph& Graph, int Size) {
	node Initial = Graph.new_node();

	node Prev = Initial;
	for (int i = 1; i < Size; ++i) {
		node Current = Graph.new_node();
		Connect(Graph, Prev, Current);
		Prev = Current;
	}
	Connect(Graph, Initial, Prev);
}

void Gen_OutSquare(graph& Graph, std::array<node, 4>& OutNodes) {
	OutNodes[0] = Graph.new_node();
	for (int i = 1; i < 4; ++i) {
		OutNodes[i] = Graph.new_node();
		Connect(Graph, OutNodes[i - 1], OutNodes[i]);
	}
	Connect(Graph, OutNodes[3], OutNodes[0]);
}


void Gen_Squares(graph& Graph, int Size) {
	std::array<node, 4> PrevSquare;
	std::array<node, 4> NewSquare;

	Gen_OutSquare(Graph, PrevSquare);

	for (int i = 4; i < Size; i += 4) {

		Gen_OutSquare(Graph, NewSquare);
		
		for (int j = 0; j < 4; ++j) {
			Connect(Graph, NewSquare[j], PrevSquare[j]);
		}

		std::swap(NewSquare, PrevSquare);
	}
}

void GenerateTestGraphs(graph* Graphs, int Count) {

	switch (Count) {
	default:
	case 9:
		Gen_Squares(Graphs[8], 90001);
		// Fallthrough
	case 8:
		Gen_Squares(Graphs[7], 90000);
		// Fallthrough
	case 7:
		Gen_Squares(Graphs[6], 40000);
		// Fallthrough
	case 6:
		Gen_Squares(Graphs[5], 10000);
		// Fallthrough
	case 5:
		Gen_Circle(Graphs[4], 90000);
		// Fallthrough
	case 4:
		Gen_Circle(Graphs[3], 90001);
		// Fallthrough
	case 3:
		Gen_Circle(Graphs[2], 40001);
		// Fallthrough
	case 2:
		Gen_Circle(Graphs[1], 10001);
		// Fallthrough
	case 1:
		Gen_DebugGraph(Graphs[0]);
		// Fallthrough
	case 0:
		;
	}
}

void GraphPrint(const graph& Graph) {
	std::cerr << "Pastable to: https//:dreampuf.github.io/GraphvizOnline\n";

	std::cerr << "digraph G {" << "\n";

	edge e;
	forall_edges(e, Graph) {
		std::cerr << Graph.source(e)->id() << " -> " << Graph.target(e)->id() << ";\n";
	}
	std::cerr << "}" << "\n";
}