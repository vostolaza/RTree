#ifndef R_TREE_RTREE_H
#define R_TREE_RTREE_H

#include "Node.h"

class RTree {
private:
    int m;
    int M;
    size_t dataPoints = 0;
    Node *root = nullptr;

    // Returns the the additional area the MBR would have if data were inserted
    static double deltaArea(Point &data, MBR &mbr) {
        if (mbr.contains(data)) return 0;
        auto temp = mbr;
        if (data.x < mbr.lowerLeft.x) temp.lowerLeft.x = data.x;
        else if (data.x > mbr.upperRight.x) temp.upperRight.x = data.x;
        if (data.y < mbr.lowerLeft.y) mbr.lowerLeft.y = data.y;
        else if (data.y > mbr.upperRight.y) mbr.upperRight.y = data.y;
        return temp.area() - mbr.area();
    }

    // Returns the optimal leaf node in which to insert data. If the data is
    // contained in an MBR in the tree, that node is returned (the smallest one if theres a tie)
    // If the data is not contained in any MBR, the node whose MBR area would increase the least is returned.
    Node *chooseLeaf(Node *&N, Point data) {
        if (N->isLeaf()) return N;
        Node *container = nullptr, *backupContainer = nullptr;
        auto minArea = DBL_MAX, minDelta = DBL_MAX;
        for (auto &i : N->children) {
            if (i->mbr.contains(data)) {
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

    // Returns the MBR that would contain both bounding rectangles.
    static MBR joinMBR(MBR &E1, MBR &E2) {
        MBR J{Point{0, 0}, Point{0, 0}};
        J.lowerLeft.x = E1.lowerLeft.x < E2.lowerLeft.x ? E1.lowerLeft.x : E2.lowerLeft.x;
        J.lowerLeft.y = E1.lowerLeft.y < E2.lowerLeft.y ? E1.lowerLeft.y : E2.lowerLeft.y;
        J.upperRight.x = E1.upperRight.x > E2.upperRight.x ? E1.upperRight.x : E2.upperRight.x;
        J.upperRight.y = E1.upperRight.y > E2.upperRight.y ? E1.upperRight.y : E2.upperRight.y;
        return J;
    }


    // Returns a pair of nodes whose MBRs are the furthest apart.
    static std::pair<Node *, Node *> pickSeeds(Node *&node) {
        Node *seed1 = nullptr, *seed2 = nullptr;
        auto largestD = 0;
        int seed2Index = 0;
        int seed1Index = 0;
        for (int i = 0; i < node->children.size() - 1; ++i) {
            for (int j = i + 1; j < node->children.size() - 1; ++j) {
                // Creates the MBR "J" from the two child nodes
                MBR J = joinMBR(node->children[i]->mbr, node->children[j]->mbr);
                // Calculates the blank area left in J
                auto d = J.area() - node->children[i]->mbr.area() - node->children[j]->mbr.area();
                if (d > largestD) {
                    largestD = d;
                    seed1 = node->children[i];
                    seed2 = node->children[j];
                    seed1Index = i;
                    seed2Index = j;
                }
            }
        }
        if (seed1Index > seed2Index) std::swap(seed1, seed2);
        node->children.erase(node->children.begin() + seed2Index);
        node->children.erase(node->children.begin() + seed1Index);
        return {seed1, seed2};
    }

    // Returns the furthest apart pair of points within a node.
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

    // Splits a leaf node, returning the newly created node to be inserted on the parent.
    Node *splitLeaf(Node *&node, Point &data) {
        node->insertData(data);
        // Obtains the furthest apart pair of points and places each on their own node.
        auto seeds = pickLeafSeeds(node);
        auto temp = node->data;
        node->data.clear();
        node->insertData(seeds.first);
        Node *LL = new Node();
        LL->insertData(seeds.second);
        // Inserts the remaining data in whichever node is closest. If m hasn't been reached and the amount of
        // unassigned data would get it to m, all the data is inserted in that node.
        for (int i = 0; i < temp.size(); ++i) {
            if (temp.size()-i + node->data.size() == m){
                node->insertData(temp[i]);
                continue;
            }
            else if(temp.size()-i + LL->data.size() == m){
                LL->insertData(temp[i]);
                continue;
            }
            if (euclidean(temp[i], seeds.first) <= euclidean(temp[i], seeds.second))
                node->insertData(temp[i]);
            else
                LL->insertData(temp[i]);
        }
        return LL;
    }

    // Adjusts the tree accordingly to insert the newly created node.
    void adjustTree(Node *&L, Node *&LL) {
        // If splitting root, create a new node and make it the root, add both L and LL as children
        if (!L->parent) {
            Node *newRoot = new Node();
            newRoot->insertChild(L);
            newRoot->insertChild(LL);
            root = newRoot;
            return;
        }

        // Select the node in which to insert LL
        auto parent = L->parent;
        parent->insertChild(LL);

        // If there was enough space we return
        if (parent->children.size() <= M) return;

        // Else we split the parent recursively
        auto seeds = pickSeeds(parent);
        auto temp = parent->children;
        parent->children.clear();
        parent->insertChild(seeds.first);
        auto newNode = new Node();
        newNode->insertChild(seeds.second);
        // Inserts the remaining data in whichever node would have the smallest area.
        //If m hasn't been reached and the amount of unassigned data would get it to m, all the data is inserted in that node.
        for (int i = 0; i < temp.size(); ++i) {
            if (temp.size() - i + parent->children.size() == m) {
                newNode->insertChild(temp[i]);
                continue;
            }
            else if (temp.size() - i + newNode->children.size() == m) {
                parent->insertChild(temp[i]);
                continue;
            }
            if (joinMBR(parent->mbr, temp[i]->mbr).area() <= joinMBR(newNode->mbr, temp[i]->mbr).area())
                parent->insertChild(temp[i]);
            else
                newNode->insertChild(temp[i]);
        }
        adjustTree(parent, newNode);

    }

    // Recursively searches nodes for points that intersect with the given MBR.
    void search(Node *node, MBR q, std::vector<Point> &result) {
        if (node->isLeaf()){
            if (!q.intersects(node->mbr)) return;
            for (auto &p : node->data) {
                if (q.contains(p)) result.push_back(p);
            }
            return;
        }
        for (const auto &n : node->children) {
            if (q.intersects(n->mbr)) search(n, q, result);
    }

    }

public:
    RTree(int m_, int M_) : m(m_), M(M_) {}

    size_t size() {
        return this->dataPoints;
    }

    void insert(Point data) {
        dataPoints++;
        if (!root) {
            root = new Node();
            root->insertData(data);
            return;
        }
        auto L = chooseLeaf(root, data);
        if (L->data.size() < M) {
            L->insertData(data);
            return;
        }
        auto LL = splitLeaf(L, data);
        adjustTree(L, LL);
    }

    std::vector<Point> query(Point A, Point B) {
        MBR q;
        // Creates the MBR from the given points.
        if (A.x < B.x) {
            q.lowerLeft.x = A.x;
            q.upperRight.x = B.x;
        } else {
            q.lowerLeft.x = B.x;
            q.upperRight.x = A.x;
        }
        if (A.y < B.y) {
            q.lowerLeft.y = A.y;
            q.upperRight.y = B.y;
        } else {
            q.lowerLeft.y = B.y;
            q.upperRight.y = A.y;
        }

        // Starts the recursive search from the root.
        auto curr = this->root;
        std::vector<Point> result;
        search(curr, q, result);
        return result;
    }

    void loadFromCSV(std::string fileName) {
        std::ifstream file(fileName);
        std::string line;
        bool header = true;
        while (std::getline(file, line)) {
            if (header) {
                header = false;
                continue;
            }
            std::stringstream ss(line);
            std::string row;
            std::vector<std::string> tokens;
            while (std::getline(ss, row, ',')) {
                tokens.push_back(row);
            }
            Point p;
            p.x = std::stod(tokens[5]);
            p.y = std::stod(tokens[6]);
            insert(p);
        }
    }

    // TODO: Destructor
    ~RTree() {};
};


#endif //R_TREE_RTREE_H
