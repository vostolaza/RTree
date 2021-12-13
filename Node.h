#ifndef R_TREE_NODE_H
#define R_TREE_NODE_H

#include "headers.h"


struct Point{
    double x;
    double y;
};

// Euclidean distance bewtween two points
static double euclidean(Point a, Point b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}


struct MBR{
    Point lowerLeft;
    Point upperRight;

    double area() const{
        return (upperRight.x-lowerLeft.x)*(upperRight.y-lowerLeft.y);
    }

    bool contains(Point p) const{
        return p.x >= lowerLeft.x && p.x <= upperRight.x && p.y >= lowerLeft.y && p.y <= upperRight.y;
    }

    bool intersects(MBR other) const{
        return !(other.upperRight.x < lowerLeft.x || other.lowerLeft.x > upperRight.x || other.upperRight.y < lowerLeft.y || other.lowerLeft.y > upperRight.y);
    }

};

struct Node {
    Node* parent;
    std::vector<Node*> children;
    std::vector<Point> data;
    MBR mbr;

    bool isLeaf() const{
        return children.empty();
    }

    void updateMBR(){
        if (this->isLeaf()){
            if (data.size() == 1){
                mbr.lowerLeft = mbr.upperRight = data[0];
                return;
            }
            if (data.back().x < mbr.lowerLeft.x) mbr.lowerLeft.x = data.back().x;
            else if(data.back().x > mbr.upperRight.x) mbr.upperRight.x = data.back().x;
            if (data.back().y < mbr.lowerLeft.y) mbr.lowerLeft.y = data.back().y;
            else if (data.back().y > mbr.upperRight.y) mbr.upperRight.y = data.back().y;
        }
        else {
            if (children.size() == 1){
                mbr = children[0]->mbr;
                return;
            }
            for (auto child: children){
                if (child->mbr.lowerLeft.x < mbr.lowerLeft.x) mbr.lowerLeft.x = child->mbr.lowerLeft.x;
                else if (child->mbr.upperRight.x > mbr.upperRight.x) mbr.upperRight.x = child->mbr.upperRight.x;
                if (child->mbr.lowerLeft.y < mbr.lowerLeft.y) mbr.lowerLeft.y = child->mbr.lowerLeft.y;
                else if (child->mbr.upperRight.y > mbr.upperRight.y) mbr.upperRight.y = child->mbr.upperRight.y;
            }
        }
    }

    void insertData(Point data_){
        data.push_back(data_);
        updateMBR();
    }

    void insertChild(Node* child_){
        children.push_back(child_);
        child_->parent = this;
        updateMBR();
    }
};


#endif //R_TREE_NODE_H
