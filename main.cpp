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


enum Color {
	CNone,
	CGreen,
	CBlue
};


// Function used to mark the odd cycle.
void IterateBack(const graph& Graph, list<node>& FillCircle, node_array<edge>& From, edge& LastEdge) {
	
	// LastEdge is the edge that when followed results in odd cycle.
	// both of these nodes belong to the circle
	node OrigPrev = LastEdge->terminal(0);
	node CircPrev = LastEdge->terminal(1);

	// While instead of do {} while to ensure edges to oneself work properly.
	while (OrigPrev != CircPrev) {

		// Add both nodes to the list. 
		// Pushing from different sides ensures we get nodes connected to each other in the correct oreder.
		// also pushing this to front returns the exact same circle as Leda in all of the tests.
		FillCircle.push_back(OrigPrev);
		FillCircle.push_front(CircPrev);

		// backtracking following the edges we came from will result in an odd circle (not always the shortest one)
		OrigPrev = From[OrigPrev]->terminal(0);
		CircPrev = From[CircPrev]->terminal(0);
	}
	
	// Lastly add the final odd node.
	FillCircle.push_back(CircPrev);
}

// Checks a connected graph. If it finds an odd circle it will clear OutOnFalse and fill it with the circle.
bool IsBipartiteSubgraph(const graph& Graph, 
						 node Start, 
						 node_array<Color>& NodeColors, 
						 node_array<edge>& CameFrom, 
						 list<node>& OutOnFalse) {

	std::queue<node> Queue;

	Color CurrentColor = CBlue;
	NodeColors[Start] = CurrentColor;
	Queue.push(Start);

	while (!Queue.empty()) {
		node Front = Queue.front();
		Queue.pop();

		CurrentColor = NodeColors[Front] == CBlue ? CGreen : CBlue;

		edge Edge;

		forall_out_edges(Edge, Front) {
			node Node = Graph.target(Edge);

			if (NodeColors[Node] == CNone) {
				NodeColors[Node] = CurrentColor;
				CameFrom[Node] = Edge;
				Queue.push(Node);
			}
			else if (NodeColors[Node] != CurrentColor) {
				OutOnFalse.clear();
				IterateBack(Graph, OutOnFalse, CameFrom, Edge);
				return false;
			}
		}
	}
	return true;
}


bool MyIsBipartite(const graph& Graph, list<node>& PartA, list<node>& PartB) {
	node_array<Color> NodeColors;
	node_array<edge> CameFrom;

	NodeColors.init(Graph, CNone);
	CameFrom.init(Graph);

	std::queue<node> Queue;
	node FirstNode = Graph.first_node();

	if (!IsBipartiteSubgraph(Graph, Graph.first_node(), NodeColors, CameFrom, PartA)) {
		return false;
	}

	node Node;
	forall_nodes(Node, Graph) {
		if (NodeColors[Node] == CBlue) {
			PartA.push_back(Node);
		}
		else if (NodeColors[Node] == CGreen) {
			PartB.push_back(Node);
		}
		else {

			if (!IsBipartiteSubgraph(Graph, Node, NodeColors, CameFrom, PartA)) {
				return false;
			}

			if (NodeColors[Node] == CBlue) {
				PartA.push_back(Node);
			}
			else {
				PartB.push_back(Node);
			}
		}
		
	}

	return true;
}



int main() {
	std::vector<GraphTest> Tests;

	std::cout << "Generating Tests...\n";

	GenerateTestGraphs(Tests);

	std::cout << "Finished Generating Tests...\nStarting testing...\n";

	bool PassedTests = true;
	for (int i = 0; i < Tests.size(); ++i) {
		PassedTests &= TestGraph(Tests[i].Graph, i, Tests[i].Name);
	}

	if (PassedTests) {
		std::cout << "===================\nAll tests PASSED.\n";
		Bench.Print();
	}
	else {
		std::cout << "===================\nAtleast one test failed.\n";
	}
}


// This does not return the odd circle on fail and is much faster in non biparitite graphs.
// This also asummes it is given only connected graphs.
bool MyIsBipartiteNoReturnOnFail(const graph& Graph, list<node>& PartA, list<node>& PartB) {
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
