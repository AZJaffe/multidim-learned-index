//
//  Basic_Data_Types.h
//  KD_Tree
//
//  Created by Plaza, Rafael I on 10/15/15.
//  Copyright (c) 2015 Plaza, Rafael I. All rights reserved.
//

#ifndef KD_Tree_Basic_Data_Types_h
#define KD_Tree_Basic_Data_Types_h


#include <algorithm>


//data
template<typename var>
struct PointType{
    var x;
    var y;
};


// sort points with respect to their x-coordinate
template<typename var>
bool sortPointsX(const PointType<var> &lhs ,const PointType<var> &rhs)
{
    return (lhs.x < rhs.x);
}

//sort points with respect to their y-coordinate
template<typename var>
bool sortPointsY(const PointType<var> &lhs, const PointType<var> &rhs)
{
    return (lhs.y < rhs.y);
}


template <class elemType>
struct treeNode
{
    elemType x;
    elemType y;
    treeNode<elemType>* llink;
    treeNode<elemType>* rlink;
};




#endif