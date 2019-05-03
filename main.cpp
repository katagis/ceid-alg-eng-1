#include "LEDA/graph/graph.h"
#include "LEDA/core/list.h"

#include <iostream>
#include <queue>


#ifdef _MSC_VER
#define MODERN
#endif

using leda::node;
using leda::graph;
using leda::edge;
using leda::list;
using leda::node_array;

#define INT_INF 999999



//
// Graph: A leda graph to perform bfs on
// Start: A leda node used as starting point for our bfs
// OutDist: Output distance for each node
// OutPred: Store the edge that visited each node
// Return: All visited nodes
//

void GraphDebug(const graph& Graph);

std::ostream& operator<< (std::ostream& Stream, const node& Node) {
	Stream << Node->id();
	return Stream;
}

list<node> MyBFS(const graph& Graph, node Start, node_array<int>& OutDist, node_array<edge>& OutPred) {
	list<node> Result;
	
    std::queue<node> Queue;

	OutDist.init(Graph, INT_INF);
	OutPred.init(Graph);

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
		forall_out_edges(Edge, Front) {
			node Node = Graph.target(Edge);
			if (OutDist[Node] == INT_INF) {
				OutDist[Node] = CurrentDistance;
				OutPred[Node] = Edge; 
				Result.push_back(Node);
				Queue.push(Node);
			}
		}
    }

	return Result;
}

void Connect(graph& Graph, const std::vector<node>& Nodes, int NodeIndex1, int NodeIndex2) {
	Graph.new_edge(Nodes[NodeIndex1], Nodes[NodeIndex2]);
	Graph.new_edge(Nodes[NodeIndex2], Nodes[NodeIndex1]);
}



#ifdef MODERN
template<typename PRED>
void ForEachNode(const graph& Graph, PRED pred) {
	node Node;
	forall_nodes(Node, Graph) {
		pred(Node);
	}
}
#endif

bool TestBFS() {
	graph Graph;
	std::vector<node> Nodes;

	for (int i = 0; i < 9; ++i) {
		node Node = Graph.new_node();
		Nodes.push_back(Node);
	}

	Connect(Graph, Nodes, 0, 2);
	Connect(Graph, Nodes, 1, 2);
	Connect(Graph, Nodes, 2, 3);
	Connect(Graph, Nodes, 3, 4);
	Connect(Graph, Nodes, 3, 5);
	Connect(Graph, Nodes, 5, 6);
	Connect(Graph, Nodes, 7, 8);


	node_array<int> Distance;
	node_array<edge> VisitPred;
	list<node> ret = MyBFS(Graph, Nodes[0], Distance, VisitPred);

	if (ret.size() != 7) {
		return false;
	}

	return Distance[Nodes[6]] == 4
		&& Distance[Nodes[7]] == INT_INF
		&& Distance[Nodes[0]] == 0;
}

int main() {

	if (!TestBFS()) {
		std::cerr << "MyBFS is broken.\n";
		return 0;
	}

	graph Graph;
	std::vector<node> Nodes;
	
	for (int i = 0; i < 9; ++i) {
		node Node = Graph.new_node();
		Nodes.push_back(Node);
	}

	Connect(Graph, Nodes, 0, 2);
	Connect(Graph, Nodes, 1, 2);
	Connect(Graph, Nodes, 2, 3);
	Connect(Graph, Nodes, 3, 4);
	Connect(Graph, Nodes, 3, 5);
	Connect(Graph, Nodes, 5, 6);
	Connect(Graph, Nodes, 7, 8);


	node_array<int> Distance;
	node_array<edge> VisitPred;
	auto ret = MyBFS(Graph, Nodes[0], Distance, VisitPred);

	getchar();
}

void GraphDebug(const graph& Graph) {
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