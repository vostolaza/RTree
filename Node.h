#ifndef R_TREE_NODE_H
#define R_TREE_NODE_H

#include "headers.h"

struct Point{
    double x;
    double y;
};

struct MBR{
    Point lowerLeft;
    Point upperRight;

    double area() const{
        return (upperRight.x-lowerLeft.x)*(upperRight.y-lowerLeft.y);
    }
};

struct Node {
    Node* parent;
    std::vector<Node*> children;
    std::vector<Point> data;
    MBR mbr;

    void updateMBR(){
        if (data.size() == 1){
            mbr.lowerLeft = mbr.upperRight = data[0];
            return;
        }
        if (data.back().x < mbr.lowerLeft.x) mbr.lowerLeft.x = data.back().x;
        else if(data.back().x > mbr.upperRight.x) mbr.upperRight.x = data.back().x;
        if (data.back().y < mbr.lowerLeft.y) mbr.lowerLeft.y = data.back().y;
        else if (data.back().y > mbr.upperRight.y) mbr.upperRight.y = data.back().y;
    }

    void insertData(Point data_){
        data.push_back(data_);
        updateMBR();
    }
};


#endif //R_TREE_NODE_H
