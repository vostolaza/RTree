#ifndef R_TREE_RTREE_H
#define R_TREE_RTREE_H

#include "Node.h"

class RTree {
private:
    int m;
    int M;
    Node *root = nullptr;

    static double deltaArea(Point &data, MBR &mbr) {
        if (dataInMBR(data, mbr)) return 0;
        auto temp = mbr;
        if (data.x < mbr.lowerLeft.x) temp.lowerLeft.x = data.x;
        else if (data.x > mbr.upperRight.x) temp.upperRight.x = data.x;
        if (data.y < mbr.lowerLeft.y) mbr.lowerLeft.y = data.y;
        else if (data.y > mbr.upperRight.y) mbr.upperRight.y = data.y;
        return temp.area() - mbr.area();
    }

    static bool dataInMBR(Point data, MBR &mbr) {
        return data.x >= mbr.lowerLeft.x && data.x <= mbr.upperRight.x && data.y <= mbr.upperRight.y &&
               data.y >= mbr.lowerLeft.y;
    }

    Node *chooseLeaf(Node *&N, Point data) {
        if (N->children.empty()) return N;
        Node *container = nullptr, *backupContainer = nullptr;
        auto minArea = DBL_MAX, minDelta = DBL_MAX;
        for (auto &i : N->children) {
            if (dataInMBR(data, i->mbr)) {
                if (i->mbr.area() < minArea) {
                    minArea = i->mbr.area();
                    container = i;
                }
            } else if (!container) {
                if (deltaArea(data, i->mbr) < minDelta) {
                    minDelta = deltaArea(data, i->mbr);
                    backupContainer = i;
                }
            }
        }
        if (container) return chooseLeaf(container, data);
        return chooseLeaf(backupContainer, data);
    }

    static MBR buildBiggestMBR(MBR &E1, MBR &E2) {
        MBR J{Point{0, 0}, Point{0, 0}};
        J.lowerLeft.x = E1.lowerLeft.x < E2.lowerLeft.x ? E1.lowerLeft.x : E2.lowerLeft.x;
        J.lowerLeft.y = E1.lowerLeft.y < E2.lowerLeft.y ? E1.lowerLeft.y : E2.lowerLeft.y;
        J.upperRight.x = E1.upperRight.x > E2.upperRight.x ? E1.upperRight.x : E2.upperRight.x;
        J.upperRight.y = E1.upperRight.y > E2.upperRight.y ? E1.upperRight.y : E2.upperRight.y;
        return J;
    }

    static std::pair<Node *, Node *> pickSeeds(Node *&node) {
        Node *seed1 = nullptr, *seed2 = nullptr;
        auto largestD = 0;
        int seed2Index = 0;
        for (int i = 0; i < node->children.size() - 1; ++i) {
            for (int j = i + 1; j < node->children.size() - 1; ++j) {
                MBR J = buildBiggestMBR(node->children[i]->mbr, node->children[j]->mbr);
                auto d = J.area() - node->children[i]->mbr.area() - node->children[j]->mbr.area();
                if (d > largestD) {
                    largestD = d;
                    seed1 = node->children[i];
                    seed2 = node->children[j];
                    seed2Index = j;
                }
            }
        }
        node->children.erase(node->children.begin() + seed2Index);
        return {seed1, seed2};
    }

    static double euclidean(Point a, Point b) {
        return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
    }

    static std::pair<Point, Point> pickLeafSeeds(Node *&node) {
        Point seed1, seed2;
        auto largestD = 0;
        int seed2Index = 0, seed1Index = 0;
        for (int i = 0; i < node->data.size() - 1; ++i) {
            for (int j = i + 1; j < node->data.size() - 1; ++j) {
                auto d = euclidean(node->data[i], node->data[j]);
                if (d > largestD) {
                    largestD = d;
                    seed1 = node->data[i];
                    seed2 = node->data[j];
                    seed1Index = i;
                    seed2Index = j;
                }
            }
        }
        if (seed2Index > seed1Index) std::swap(seed1Index, seed2Index);
        node->data.erase(node->data.begin() + seed1Index);
        node->data.erase(node->data.begin() + seed2Index);
        return {seed1, seed2};
    }

    Node *splitLeaf(Node *&node, Point &data) {
        node->insertData(data);
        auto seeds = pickLeafSeeds(node);
        std::vector<Point> newData;
        newData.push_back(seeds.first);
        auto temp = node->data;
        node->data = newData;
        Node *LL = new Node();
        LL->insertData(seeds.second);
        for (int i = 0; i < temp.size(); ++i) {
            if (temp.size()-i + node->data.size() == M){
                node->insertData(temp[i]);
                continue;
            }
            else if(temp.size()-i + LL->data.size() == M){
                LL->insertData(temp[i]);
                continue;
            }
            if (euclidean(temp[i], seeds.first) <= euclidean(temp[i], seeds.second)) node->insertData(temp[i]);
            else LL->insertData(temp[i]);
        }
        return LL;
    }

    void adjustTree(Node *&L, Node *&LL) {
        // TODO
    }

public:
    RTree(int m_, int M_) : m(m_), M(M_) {}

    void insert(Point data) {
        if (!root) {
            root = new Node();
            root->data.push_back(data);
        }
        auto L = chooseLeaf(root, data);
        if (L->data.size() < M) {
            L->data.push_back(data);
            return;
        }
        auto LL = splitLeaf(L, data);
        L->parent->children.push_back(LL);
        adjustTree(L, LL);
    }

    void query(Point A, Point B) {

    }

    // TODO
    ~RTree() {};
};


#endif //R_TREE_RTREE_H
