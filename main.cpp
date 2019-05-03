#include "LEDA/graph/graph.h"
#include "LEDA/core/list.h"

#include <iostream>
#include <queue>
#include <set>


using namespace leda;

//
// Graph: A leda graph to perform bfs on
// Start: A leda node used as starting point for our bfs
// 
//
list<node> MyBFS(const graph& Graph, node Start, node_array<int>& OutDist, node_array<edge>& OutPred) {
    list<node> Result;

    std::queue<node> Queue;
    std::set<node>  Marked;

    Queue.push(Start);

    while (!Queue.empty()) {
        node Front = Queue.front();
        Queue.pop();

        for (list<edge>::iterator It = Graph.adj_edges(Front).begin(); *It; ++It) { 
            const node Node = (*It)->terminal(1);
            
            std::pair<std::set<node>::iterator, bool> ret = Marked.insert(Node);
            if (ret.second) {
                Queue.push(Node);
            }
        }
    }

	return Result;
}

int main() {
    std::cout << "Hi world\n";
	getchar();
}