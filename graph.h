#ifndef _GRAPH_H_
#define _GRAPH_H_

#include <vector>
#include <list>
#include <string>
#include <map>
#include <deque>
#include <algorithm>
#include <iostream>
#include <assert.h>

#define INF 0x7F7F7F7F

//这里为了方便实现.临接表中每一个节点使用16位的int表示.
//其中前8位表示权重，后8位表示节点idx
#define WEIGHT(i)        ((i & 0xff00) >> 8)
#define INDEX(i)         (i & 0xff)
#define MAKE(w,i)        ((((w) & 0xff) << 8) | i)

typedef std::vector<std::list<int>> GraphArc;
typedef std::string VexType;
typedef struct Vex{
    int idx;
    VexType data;
}GVex;

typedef std::pair<VexType,VexType> GArc;
typedef bool (* VisitFunc) (GVex& vex);
typedef std::vector<GVex> GraphVex;

/*
 * 图的实现方式有两种，分别是临接矩阵和临接表
 * 在实际的开发中，都是采用的临接表来实现的.因为临接矩阵在点个数多的情况下，空间利用率是很低的.
 */

//图类
class CGraph{
public:
    CGraph(bool direct = true){};
    void CreateGraph(std::vector<VexType>& vex,std::vector<std::pair<VexType,VexType>>& arc,
                        const std::vector<int>& weights = std::vector<int>()){
        int i = 0;
        //遍历顶点数组
        for(auto iter = vex.begin();iter != vex.end();iter++,i++){
            _graph_vex.push_back({i,*iter});
            _vex_idx.insert({*iter,i});       //建立映射关系
        }
        for(int j = 0;j < i;++j){
            _graph_arc.push_back(std::list<int>());
        }
        bool has = weights.size() > 0;
        //建立边与边之间的关系
        for(size_t i = 0;i < arc.size();++i){
            InsertArc(arc[i],has ? weights[i] : 0);
        }
    }

    inline size_t VexSize(){
        return _graph_vex.size();
    }

    int LocateVex(VexType& v){
        return _get_idx(v);
    }

    VexType GetVex(int idx){
        assert(idx >= 0 && idx < _graph_vex.size());
        return _graph_vex[idx].data;
    }

    bool PutVex(int idx,int value);

    //插入节点
    bool InsertVex(VexType& v){
        if(-1 != _get_idx(v)){
            return false;
        }
        int i = (int)_graph_vex.size();
        _graph_vex.push_back({i,v});
        _vex_idx.insert({v,i});

        return true;
    }
    //删除节点
    bool DeleteVex(VexType& v){
        int idx = _get_idx(v);
        if(-1 == idx){
            return false;
        }

        auto& vexlist = _graph_arc[idx];
        for(auto iter = vexlist.begin();iter != vexlist.end();++iter){
            _graph_arc[*iter].remove(idx);
        }
        _graph_arc[idx].clear();
        _graph_vex[idx].data.clear();
        _vex_idx.erase(v);

        return true;
    }
    //插入边
    bool InsertArc(const GArc& arc,int weight = 0){
        int iFirst = _get_idx(arc.first);
        if(-1 == iFirst){
            return false;
        }
        int iSecond = _get_idx(arc.second);
        if(-1 == iSecond){
            return false;
        }
        int wFirst = MAKE(weight,iFirst);
        int wSecond = MAKE(weight,iSecond);
        auto& vexlist = _graph_arc[iFirst];
        if(find(vexlist.begin(),vexlist.end(),wSecond) != vexlist.end()){
            return false;
        }
        vexlist.push_back(wSecond);
        if(!_direct){
            //如果无方向的话
            auto& revlist = _graph_arc[iSecond];
            if(find(revlist.begin(),revlist.end(),wFirst) != revlist.end()){
                return false;
            }
            revlist.push_back(wFirst);
        }
        return true;
    }
    //删除边
    bool DeleteArc(GArc& arc){
        int iFirst = _get_idx(arc.first);
        if(-1 == iFirst){
            return false;
        }
        int iSecond = _get_idx(arc.second);
        if(-1 == iSecond){
            return false;
        }

        auto& vexlist = _graph_arc[iFirst];
        bool has = false;
        auto it = vexlist.begin();
        for(;it != vexlist.end();++it){
            if(INDEX(*it) == iSecond){
                has = true;
                break;
            }
        }
        if(!has){
            return false;
        }

        vexlist.erase(it);

        if(!_direct){
            has = false;
            auto& revlist = _graph_arc[iSecond];
            it = revlist.begin();
            for(;it != revlist.end();++it){
                if(INDEX(*it) == iFirst){
                    has = true;
                    break;
                }
            }
            if(!has){
                return false;
            }
            revlist.erase(it);
        }
        return true;
    }

    //深度优先遍历
    void DFSTraverse(std::vector<bool>& visited,VisitFunc func){
        //为了遍历连通图的存在，以每一个节点作为起点依次判断
        for(size_t i = 0;i < _graph_vex.size();++i){
            if(!visited[i]){
                _dfs_traverse(i,visited,func);
            }
        }
    }
    //广度优先遍历
    void BFSTraverse(std::vector<bool>& visited,VisitFunc func){
        for(size_t i = 0;i < _graph_vex.size();++i){
            if(!visited[i]){
                _bfs_traverse(i,visited,func);
            }
        }
    }

    //迪杰斯特拉最短路径算法
    void Dijkstra(VexType s,std::vector<bool>& visited,std::vector<int>& dis,std::vector<int>& parent){
        int iStart = _get_idx(s);
        if(-1 == iStart){
            return ;
        }
        visited[iStart] = true;
        dis[iStart] = 0;
        parent[iStart] = iStart;

        int i = iStart;
        while(true){
            auto& arclist = _graph_arc[i];
            int min = INF;
            int midx = -1;
            //修改值
            for(auto it = arclist.begin();it != arclist.end();++it){
                int idx = INDEX(*it);
                int wgt = WEIGHT(*it);
                if(dis[idx] > dis[i] + wgt){
                    dis[idx] = dis[i] + wgt;
                    parent[idx] = i;
                }
            }
            //扫描
            for(size_t j = 0;j < dis.size();++j){
                if(!visited[j] && min > dis[j]){
                    min = dis[j];
                    midx = j;
                }
            }
            if(midx == -1){
                break;
            }
            visited[midx] = true;
            i = midx;
        }
    }
    
private:
    void _dfs_traverse(int idx,std::vector<bool>& visited,VisitFunc func){
        //结束条件
        if(visited[idx]){
            return ;
        }
        visited[idx] = true;
        func(_graph_vex[idx]);
        auto& arclist = _graph_arc[idx];
        for(auto it = arclist.begin();it != arclist.end();++it){
            int next = INDEX(*it);
            if(!visited[next]){
                _dfs_traverse(next,visited,func);
            }
        }
    }
    void _bfs_traverse(int idx,std::vector<bool>& visited,VisitFunc func){
        std::deque<int> queue;
        queue.push_back(idx);
        visited[idx] = true;
        func(_graph_vex[idx]);
        while(queue.size() != 0){
            int i = queue.front();
            queue.pop_front();
            auto& arclist = _graph_arc[i];
            for(auto it = arclist.begin();it != arclist.end();++it){
                int next = INDEX(*it);
                if(!visited[next]){
                    queue.push_back(next);
                    visited[next] = true;
                    func(_graph_vex[next]);
                }
            }
        }
    }

    int _get_idx(const VexType& v){
        auto it = _vex_idx.find(v);
        if(it != _vex_idx.end()){
            return it->second;
        }
        return -1;
    }
private:
    GraphArc _graph_arc;        //临接表
    GraphVex _graph_vex;

    std::map<VexType,int> _vex_idx;  //元素值和下标之间的映射
    bool  _direct;             //是否是有向图
};


#endif
