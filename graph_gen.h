#include "LEDA/graph/graph.h"

#include <vector>
#include <array>
#include <ctime>

//#define USE_12GB_TEST

using leda::graph;
using leda::node;
using leda::edge;
using leda::list;


// Utility Output stream operators used for debugging...
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

// Utility to "export" a graph for online viewing.
void PrintGraph(const graph& Graph) {
	std::cerr << "Paste this to: https//:dreampuf.github.io/GraphvizOnline\n";

	std::cerr << "digraph G {" << "\n";
	edge e;
	forall_edges(e, Graph) {
		std::cerr << Graph.source(e)->id() << " -> " << Graph.target(e)->id() << ";\n";
	}
	std::cerr << "}" << "\n";
}

void ConnectIndex(graph& Graph, const std::vector<node>& Nodes, int NodeIndex1, int NodeIndex2) {
	Graph.new_edge(Nodes[NodeIndex1], Nodes[NodeIndex2]);
	Graph.new_edge(Nodes[NodeIndex2], Nodes[NodeIndex1]);
}

void Connect(graph& Graph, const node& Node1, const node& Node2) {
	Graph.new_edge(Node1, Node2);
	Graph.new_edge(Node2, Node1);
}

// Custom graph used for debugging.
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

// Generates a circle with Size nodes
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

// Generates a single square where each node is connected to previous and next one.
// Fills out the OutNodes with the nodes for further connections.
void Gen_OutSquare(graph& Graph, node* OutNodes) {
	OutNodes[0] = Graph.new_node();
	for (int i = 1; i < 4; ++i) {
		OutNodes[i] = Graph.new_node();
		Connect(Graph, OutNodes[i - 1], OutNodes[i]);
	}
	Connect(Graph, OutNodes[3], OutNodes[0]);
}

// Generates multiple levels of squares.
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

// Generates the test case from the assignment with 4 groups
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
    // These tests are too big to run on the remote test environment
    // I tested them locally and the results are comparable with the smaller tests
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
