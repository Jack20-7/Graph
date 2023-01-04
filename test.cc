#include "graph.h"

bool print_node(GVex& vex){
    std::cout << vex.data << " -> ";
    return true;
}

int main(int argc,char** argv){
    CGraph graph(false);
    std::vector<VexType> vex = {"A","B","C","D","E","F"};
    std::vector<std::pair<VexType,VexType>> arc = {
            {"A","B"},
            {"A","C"},
            {"A","D"},
            {"B","C"},
            {"B","F"},
            {"C","E"},
            {"E","F"},
            {"E","D"},
        };
    std::vector<int> weights = {
            7,
            9,
            11,
            8,
            20,
            6,
            1,
            3,
        };

    graph.CreateGraph(vex,arc,weights);
    {
        //测试迪杰斯特拉算法
        size_t size = graph.VexSize();
        std::vector<bool> visited(size,false);
        std::vector<int> dis(size,INF);
        std::vector<int> parent(size,-1);
        graph.Dijkstra("A",visited,dis,parent);
        for(size_t i = 0;i < size;++i){
            std::cout << vex[i] << " -> " << (visited[i] ? "true" : "false") << " -> " 
                      << dis[i] << " -> " << vex[parent[i]] << std::endl;
        }
    }

    {
        size_t size = graph.VexSize();
        std::cout << "dfs traverse: " << std::endl;
        std::vector<bool> visited(size,false);
        graph.DFSTraverse(visited,print_node);
        std::cout << "NULL" << std::endl;
    }

    {
        size_t size = graph.VexSize();
        std::cout << "bfs traverse:" << std::endl;
        std::vector<bool> visited(size,false);
        graph.BFSTraverse(visited,print_node);
        std::cout << "NULL" << std::endl;
    }

    return 0;
}
