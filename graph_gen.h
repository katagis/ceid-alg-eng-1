#include "LEDA/graph/graph.h"

#include <vector>
#include <array>
#include <ctime>

//#define USE_12GB_TEST

using leda::graph;
using leda::node;
using leda::edge;
using leda::list;



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

	for (int i = 0; i < 9; ++i) {
		node Node = Graph.new_node();
		Nodes.push_back(Node);
	}

	ConnectIndex(Graph, Nodes, 0, 2);
	ConnectIndex(Graph, Nodes, 1, 2);
	ConnectIndex(Graph, Nodes, 2, 3);
	ConnectIndex(Graph, Nodes, 3, 4);
	ConnectIndex(Graph, Nodes, 3, 5);
	ConnectIndex(Graph, Nodes, 5, 6);
	ConnectIndex(Graph, Nodes, 1, 6);
	ConnectIndex(Graph, Nodes, 7, 8);
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


void Gen_Custom(graph& Graph, int Size) {
	// Generate two distinct circles with even nodes.
	// Both will be bipartite.
	Gen_Circle(Graph, Size * 2);
	Gen_Circle(Graph, Size * 2);

	// Connect one node of the first with one of the second.
	// The resulting graph is still bipartite.
	// first and last node is the most efficient and guaranteed to be in different circles.
	Connect(Graph, Graph.first_node(), Graph.last_node());
	

	// select 2 random nodes from any circle and connect them
	// there is now equal chance for the graph to be bipartite or not
	node Random1 = Graph.choose_node();
	node Random2 = Graph.choose_node();
	
	Connect(Graph, Random1, Random2);
}



// Used to hold a "test". A specific graph and a name.
struct GraphTest {
	graph Graph;
	std::string Name;

	GraphTest(const std::string& InName)
		: Name(InName) {}
};

// Adds tests to the vector.
void GenerateTestGraphs(std::vector<GraphTest>& Tests) {
#define ADD_TEST(Name, Code) do{ Tests.push_back(GraphTest(Name)); graph& G = Tests.back().Graph; Code; }while(0)

	ADD_TEST("Debug Case"     , Gen_DebugGraph(G));

	ADD_TEST("Squares 10000"  , Gen_Squares(G, 10000));
	ADD_TEST("Squares 40000"  , Gen_Squares(G, 40000));
	ADD_TEST("Squares 90000"  , Gen_Squares(G, 90000));

	ADD_TEST("Circle 10001"   , Gen_Circle(G, 10001));
	ADD_TEST("Circle 40001"   , Gen_Circle(G, 40001));
	ADD_TEST("Circle 90001"   , Gen_Circle(G, 90001));
	ADD_TEST("Circle 90000"   , Gen_Circle(G, 90000));

	ADD_TEST("Groups 500"     , Gen_ParallelRandom<500>(G));
	ADD_TEST("Groups 1000"    , Gen_ParallelRandom<1000>(G));
	ADD_TEST("Groups 1500"    , Gen_ParallelRandom<1500>(G));
	ADD_TEST("Groups 20000"   , Gen_ParallelRandom<20000>(G));

	ADD_TEST("Bonus 400"  , Gen_Custom(G, 400));
	ADD_TEST("Bonus 10000", Gen_Custom(G, 10000));
	ADD_TEST("Bonus 20000", Gen_Custom(G, 20000));
	ADD_TEST("Bonus 40000", Gen_Custom(G, 40000));

	ADD_TEST("2 Cir 90000", { Gen_Circle(G, 90000); Gen_Circle(G, 90000); });

#undef ADD_TEST
}
