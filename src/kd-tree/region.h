//
//  region.h
//  KD_Tree
//
//  Created by Plaza, Rafael I on 10/15/15.
//  Copyright (c) 2015 Plaza, Rafael I. All rights reserved.
//

#ifndef KD_Tree_region_h
#define KD_Tree_region_h

#include <iostream>

using namespace std;

// class to describe rectangular regions in the xy-plane
template <class elemType>
class region
{
public:
    // default constructor
    region();
    
    // set x_min=n1, x_max=n_2 , y_min=n3, y_max=n4
    region(elemType n1, elemType n2, elemType n3, elemType n4);
    
    //change the boundary of the rectagular region
    void updateX_left(elemType l_left);
    void updateX_right(elemType l_right);
    void updateY_below(elemType l_below);
    void updateY_above(elemType l_above);
    
    //check if region is contained in otherRegion. It returns true
    //if it is contained and false otherwise
    bool isContained(region<elemType> otherRegion);
    
    //it returns true if the intersection between region and otherRegion
    //is not empty and flase otherwise
    bool intersects(region<elemType> otherRegion);
    
    
    void printRegion();
    
    
    bool empty;
    
    //The boundary of the rectangular region is completely defined by the following variables
    elemType x_min;
    elemType x_max;
    elemType y_min;
    elemType y_max;
};



//**********************************//
//methods class region              //
//**********************************//
template <class elemType>
region<elemType>::region()
{
    empty=false;
    x_min=0;
    x_max=0;
    y_min=0;
    y_max=0;
}

template <class elemType>
region<elemType>::region(elemType n1, elemType n2, elemType n3, elemType n4)
{
    empty=false;
    x_min=n1;
    x_max=n2;
    y_min=n3;
    y_max=n4;
    
}


template <class elemType>
bool region<elemType>::intersects(region<elemType> otherRegion)
{
    region<elemType> corner1(x_max,x_max,y_max,y_max);
    if(corner1.isContained(otherRegion))
        return true;
    
    region<elemType> corner2(x_max,x_max,y_min,y_min);
    if(corner2.isContained(otherRegion))
        return true;
    
    region<elemType> corner3(x_min,x_min,y_max,y_max);
    if(corner3.isContained(otherRegion))
        return true;
    
    region<elemType> corner4(x_min,x_min,y_min,y_min);
    if(corner4.isContained(otherRegion))
        return true;
    
    region<elemType> corner5(otherRegion.x_min,otherRegion.x_min,otherRegion.y_min,otherRegion.y_min);
    if(corner5.isContained(*this))
        return true;
    region<elemType> corner6(otherRegion.x_min,otherRegion.x_min,otherRegion.y_max,otherRegion.y_max);
    if(corner6.isContained(*this))
        return true;
    
    region<elemType> corner7(otherRegion.x_max,otherRegion.x_max,otherRegion.y_min,otherRegion.y_min);
    if(corner7.isContained(*this))
        return true;
    region<elemType> corner8(otherRegion.x_max,otherRegion.x_max,otherRegion.y_max,otherRegion.y_max);
    if(corner8.isContained(*this))
        return true;
    
    if((y_min<=otherRegion.y_max) and (otherRegion.y_max<=y_max) and (otherRegion.x_min<=x_min) and (x_min<=otherRegion.x_max))
        return true;
    
    if((x_min<=otherRegion.x_max) and (otherRegion.x_max<=x_max) and (otherRegion.y_min<=y_min) and (y_min<=otherRegion.y_max))
        return true;
    
    
    return false;
}


template <class elemType>
bool region<elemType>::isContained(region<elemType> otherRegion)
{
    if (empty or otherRegion.empty)
        return false;
    else
        return (x_max <= otherRegion.x_max) and (otherRegion.x_min <= x_min) and (y_max <= otherRegion.y_max) and (otherRegion.y_min <= y_min);
}

template <class elemType>
void region<elemType>::printRegion()
{
    if (!empty)
        cout << "[" << x_min << "," << x_max << "] x [" << y_min << "," << y_max << "]" << endl;
    else
        cout << "it is empty" << endl;
}


//function to compute the intersection between region and the half plane to the left of the line x=l_left
// Postcondition: The values of x_min, x_max, y_min and y_max represents the intersection of the former region
//                and the halpf plane to the left of x=l_left;
template <class elemType>
void region<elemType>::updateX_left(elemType l_left)
{
    if((!empty) and x_min <= l_left and l_left <= x_max)
        x_max=l_left;
    else if((!empty) and l_left < x_min)
        empty=true;
}



template <class elemType>
void region<elemType>::updateX_right(elemType l_right)
{
    if((!empty) and x_min <= l_right and l_right <= x_max)
        x_min=l_right;
    else if((!empty) and x_max<l_right)
        empty=true;
}

template <class elemType>
void region<elemType>::updateY_below(elemType l_below)
{
    if((!empty) and y_min <= l_below and l_below <= x_max)
        y_max=l_below;
    else if((!empty) and l_below < y_min)
        empty=true;
}


template <class elemType>
void region<elemType>::updateY_above(elemType l_above)
{
    if((!empty) and y_min <= l_above and l_above <= y_max)
        y_min=l_above;
    else if((!empty)  and y_max<l_above)
        empty=true;
}




#endif