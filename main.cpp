#include "LEDA/graph/graph.h"
#include "LEDA/core/list.h"

#include <iostream>
#include <queue>
#include <set>
#include <array>
#include <algorithm>
#include <ctime>

#ifdef USE_CHRONO
// Chrono is not available on g++ 4.4.7
#include <chrono>
namespace ch = std::chrono;
#else 
// Use Unix Time, sometimes fails. 
#include <sys/time.h>
#endif

//#define USE_12GB_TEST

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

// Not actually used anywhere, since we don't need all this info to determine if graph is bipartite
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


struct Benchmark {
private:
	std::vector<long long> MyTime;
	std::vector<long long> LedaTime;

#ifdef USE_CHRONO
	ch::time_point<ch::system_clock> StartTime;
	void RestartTimer() {
		StartTime = ch::system_clock::now();
	}

	long long GetCurrent() const {
		return ch::duration_cast<ch::microseconds>(ch::system_clock::now() - StartTime).count();
	}
#else
	long StartTime;

	long GetUnixMicros() const {
		struct timeval TimeVal;
		struct timezone TimeZone;
		if (gettimeofday(&TimeVal, &TimeZone)) {
            return 0;
        };
		return TimeVal.tv_usec;
	}

	void RestartTimer() {
		StartTime = GetUnixMicros();
	}

	long long GetCurrent() const {
        long UnixMicros = GetUnixMicros();
        if (UnixMicros <= 0 || StartTime <= 0) {
            return 0;
        }
        return UnixMicros - StartTime;
	}
#endif

	void PrintBenchLine(long long MyT, long long LedaT) {
		static std::string TimestepStr = " micros";

		std::cout << " | Mine: " << std::setw(8) << MyT << TimestepStr
				 << "\t| Leda: " << std::setw(8) << LedaT << TimestepStr;

		long long Hi = std::max(std::max(MyT, LedaT), 1LL);
		long long Lo = std::min(MyT, LedaT);
		
		int Percent = 100 - std::floor(((float)Lo / Hi) * 100.f + 0.5f);
		long long AbsDiff = Hi - Lo;

		std::string Who = MyT < LedaT ? "Mine" : "Leda";
		std::cout << " \t" << Who << " is faster by: " << std::setw(2) << Percent << "% (" << std::setw(8) << AbsDiff << TimestepStr << ")\n" ;
	}

public:
	void StartTest() {
		RestartTimer();
	}

	// The leda test has finished and my test is starting
	void SwitchTest() {
		long long Duration = GetCurrent();
		LedaTime.push_back(Duration);
		RestartTimer();
	}

	void StopTest() {
		long long Duration = GetCurrent();
		MyTime.push_back(Duration);
	}

	void PrintLast() {
		size_t Index = MyTime.size() - 1;
		PrintBenchLine(MyTime[Index], LedaTime[Index]);
	}

	void Print() {
        bool ContainsInvalidResult = false;
		long long MyTotal = 0;
		long long LedaTotal = 0;
		for (int i = 0; i < MyTime.size(); ++i) {
			std::cout << "Test " << std::setw(2) << i << ":";
			PrintBenchLine(MyTime[i], LedaTime[i]);

            if (MyTime[i] > 0 && LedaTime[i] > 0) {
                MyTotal += MyTime[i];
                LedaTotal += LedaTime[i];
            }
            else {
                ContainsInvalidResult = true;
            }
		}

		std::cout << "\nTotals:";
		PrintBenchLine(MyTotal, LedaTotal);

        if (ContainsInvalidResult) {
            std::cout << "\nUnix timer does not always return a valid time.\n"
                    << "It is HIGHLY recommended to use chrono if possible for better results.\n"
                    << "Results detected with invalid times where not included in the total.\n\n";
        }
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

void Gen_OutSquare(graph& Graph, node* OutNodes) {
	OutNodes[0] = Graph.new_node();
	for (int i = 1; i < 4; ++i) {
		OutNodes[i] = Graph.new_node();
		Connect(Graph, OutNodes[i - 1], OutNodes[i]);
	}
	Connect(Graph, OutNodes[3], OutNodes[0]);
}


void Gen_Squares(graph& Graph, int Size) {
	node PrevSquare[4];
	node NewSquare[4];

	Gen_OutSquare(Graph, PrevSquare);

	for (int i = 4; i < Size; i += 4) {

		Gen_OutSquare(Graph, NewSquare);
		
		for (int j = 0; j < 4; ++j) {
			Connect(Graph, NewSquare[j], PrevSquare[j]);
		}

		std::swap(NewSquare, PrevSquare);
	}
}

template<int Size>
void Gen_ParallelRandom(graph& Graph) {
	// storing all nodes is required to allow random selection later
	std::array<std::array<node, Size>, 4> Lgroup;

	// i iterates over an L group, specifically the Lj group.
	for (int i = 0; i < Size; ++i) {
		
		Lgroup[0][i] = Graph.new_node();
		
		for (int j = 1; j < 4; ++j) {
			Lgroup[j][i] = Graph.new_node();
			// Connect with the one in the previous set
			Connect(Graph, Lgroup[j - 1][i], Lgroup[j][i]);
		}
	}

	std::srand(std::time(NULL));
	
	for (int SetIndex = 0; SetIndex < 3; ++SetIndex) {
		// Select one for each group
		node Selected = Lgroup[SetIndex][std::rand() % Size];
		for (int i = 0; i < Size; ++i) {
			// Connect it with all nodes of the next group
			Connect(Graph, Selected, Lgroup[SetIndex + 1][i]);
		}
	}

	for (int i = 0; i < 2; ++i) {
		node First = Lgroup[i][std::rand() % Size];
		node Second = Lgroup[i+2][std::rand() % Size];
	
		Connect(Graph, First, Second);
	}
}

void GenerateTestGraphs(graph* Graphs, int Count) {

	switch (Count) {
	default:
#ifdef USE_12GB_TEST
	case 15:
		Gen_Squares(Graphs[14], 15000000);
	case 14:
		Gen_Circle(Graphs[13], 9000000);
#endif
	case 13:
		Gen_ParallelRandom<20000>(Graphs[12]);
		// Fallthrough
	case 12:
		Gen_ParallelRandom<1500>(Graphs[11]);
		// Fallthrough
	case 11:
		Gen_ParallelRandom<1000>(Graphs[10]);
		// Fallthrough
	case 10:
		Gen_ParallelRandom<500>(Graphs[9]);
		// Fallthrough
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