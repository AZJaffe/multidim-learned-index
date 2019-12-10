//
//  KD_Tree.h
//  KD_Tree
//
//  Created by Plaza, Rafael I on 10/15/15.
//  Copyright (c) 2015 Plaza, Rafael I. All rights reserved.
//

#ifndef KD_Tree_KD_Tree_h
#define KD_Tree_KD_Tree_h

//*********************//
//KD-Tree              //
//*********************//


#include "Basic_Data_Types.h"
#include "region.h"

using namespace std;


template <class elemType>
class KD_Tree
{
public:
    KD_Tree();
    //default constructor
    
    void inorderTraversal() const;
    //function to do an inorder traversal of the binary tree
    
    void destroyTree();
    //deallocates the memory space occupied by the binary tree
    //Postcondition: root=Null
    
    void buildKD_Tree(PointType<elemType> *p, int n_points);
    
    int searchKD_Tree(region<elemType> query);
    
protected:
    treeNode<elemType>* root;
    
private:
    treeNode<elemType>* build(PointType<elemType> *p_x, PointType<elemType> *p_y, int n_points, int depth);
    
    int search(treeNode<elemType> *p, region<elemType> nodeRegion, region<elemType> query, int depth);
    
    int median(const int n_points);
    
    void inorder(treeNode<elemType>* p) const;
    void destroy(treeNode<elemType>* &p);
    
    bool isLeaf(treeNode<elemType>* p) const;
    int ReportSubtree(treeNode<elemType> *p);
    
    
    
    elemType x_max;
    elemType x_min;
    elemType y_max;
    elemType y_min;
    
    
};



//***************************//
//  Methods Class KD_Tree    //
//***************************//

template <class elemType>
KD_Tree<elemType>::KD_Tree()
{
    root=NULL;
    x_max=0;
    x_min=0;
    y_min=0;
    y_max=0;
}


template <class elemType>
int KD_Tree<elemType>::median(const int n_points)
{
    if((n_points % 2)==0)
        return (n_points/2)-1;
    else
        return n_points/2;
}

template <class elemType>
int KD_Tree<elemType>::search(treeNode<elemType> *p, region<elemType> nodeRegion, region<elemType> query, int depth)
{
    int s = 0;
    if (isLeaf(p))
    {
        if((query.x_min <= p->x) and (p->x <= query.x_max) and (query.y_min <= p->y) and (p->y <= query.y_max)) {
            // cout << "(" << p->x  << "," << p->y << ")" << endl;
            s += 1;
        }
    }
    else
    {
        region<elemType> lc_nodeRegion, rc_nodeRegion;
        lc_nodeRegion=nodeRegion;
        rc_nodeRegion=nodeRegion;
        if((depth % 2) == 0)
        {
            lc_nodeRegion.updateX_left(p->x);
            rc_nodeRegion.updateX_right(p->x);
        }
        else
        {
            lc_nodeRegion.updateY_below(p->y);
            rc_nodeRegion.updateY_above(p->y);
        }
        if(lc_nodeRegion.isContained(query))
            s += ReportSubtree(p->llink);
        else if(lc_nodeRegion.intersects(query))
            s += search(p->llink, lc_nodeRegion, query, depth+1);
        
        if(rc_nodeRegion.isContained(query))
            s += ReportSubtree(p->rlink);
        else if(rc_nodeRegion.intersects(query))
            s += search(p->rlink, rc_nodeRegion, query, depth+1);
        
    }
    return s;
}

template <class elemType>
int KD_Tree<elemType>::searchKD_Tree(region<elemType> query)
{
    region<elemType> nodeRegion(x_min,x_max,y_min,y_max);
    return search(root, nodeRegion, query, 0);
    
}



template <class elemType>
treeNode<elemType>* KD_Tree<elemType>::build(PointType<elemType> *p_x, PointType<elemType> *p_y, int n_points, int depth)
{
    treeNode<elemType> *node;
    node = new treeNode<elemType>;
    
    if (n_points==1)
    {
        node->x=p_x[0].x;
        node->y=p_x[0].y;
        node->llink=NULL;
        node->rlink=NULL;
        
    }
    else
    {
        int m;
        m=median(n_points);
        
        if((depth %2) == 0)
        {
            node->x=p_x[m].x;
            node->y=p_x[m].y;
            
            PointType<elemType> *pleft_x, *pright_x;
            PointType<elemType> *pleft_y, *pright_y;
            
            pleft_x  = new PointType<elemType>[m+1];
            pright_x = new PointType<elemType>[n_points-m-1];
            pleft_y  = new PointType<elemType>[m+1];
            pright_y = new PointType<elemType>[n_points-m-1];
            
            for (int i=0; i <= m; i++)
                pleft_x[i]=p_x[i];
            for(int i=m+1; i < n_points; i++)
                pright_x[i-m-1]=p_x[i];
            
            int i_left=0, i_right=0;
            for(int i=0; i < n_points; i++)
            {
                if(p_y[i].x <= node->x)
                {
                    pleft_y[i_left]=p_y[i];
                    i_left++;
                }
                else
                {
                    pright_y[i_right]=p_y[i];
                    i_right++;
                }
            }
            
            delete [] p_x;
            delete [] p_y;
            
            
            node->llink=build(pleft_x, pleft_y, m+1, depth+1);
            node->rlink=build(pright_x, pright_y, n_points-m-1, depth+1);
            
        }
        else
        {
            node->x=p_y[m].x;
            node->y=p_y[m].y;
            
            
            PointType<elemType> *pAbove_x, *pBelow_x;
            PointType<elemType> *pAbove_y, *pBelow_y;
            
            pAbove_x  = new PointType<elemType>[n_points-m-1];
            pBelow_x = new PointType<elemType>[m+1];
            pAbove_y  = new PointType<elemType>[n_points-m-1];
            pBelow_y = new PointType<elemType>[m+1];
            
            for (int i=0; i <= m; i++)
                pBelow_y[i]=p_y[i];
            
            for(int i=m+1; i < n_points; i++)
                pAbove_y[i-m-1]=p_y[i];
            
            
            int i_Below=0, i_Above=0;
            for(int i=0; i < n_points; i++)
            {
                if(p_x[i].y <= node->y)
                {
                    pBelow_x[i_Below]=p_x[i];
                    i_Below++;
                }
                else
                {
                    pAbove_x[i_Above]=p_x[i];
                    i_Above++;
                }
            }
            
            delete [] p_x;
            delete [] p_y;
            
            
            
            node->llink=build(pBelow_x, pBelow_y, m+1, depth+1);
            node->rlink=build(pAbove_x, pAbove_y, n_points-m-1, depth+1);
            
        }
        
    }
    
    return node;
    
    
}


template <class elemType>
void KD_Tree<elemType>::buildKD_Tree(PointType<elemType> *p, int n_points)
{
    
    PointType<elemType> *p_x;
    PointType<elemType> *p_y;
    
    p_x= new PointType<elemType>[n_points];
    p_y= new PointType<elemType>[n_points];
    
    for (int i=0; i < n_points; i++)
    {
        p_x[i]= p[i];
        p_y[i]= p[i];
    }
    
    
    // cout << "sorting with respect to x-coordinate" << endl;
    sort(p_x, p_x + n_points , sortPointsX<elemType>);
    
    // for(int i=0; i < n_points; i++)
    //     cout << "(" << p_x[i].x  << "," << p_x[i].y << ")" << endl;
    
    x_max=p_x[n_points-1].x;
    x_min=p_x[0].x;
    
    //cout << "sorting with respect to y-coordinate" << endl;
    sort(p_y, p_y+ n_points, sortPointsY<elemType>);
    
    // for(int i=0; i < n_points; i++)
    //     cout << "(" << p_y[i].x  << "," << p_y[i].y << ")" << endl;
    
    y_max=p_y[n_points-1].y;
    y_min=p_y[0].y;
    
    root=build(p_x, p_y, n_points, 0);
    
    
}

template <class elemType>
void KD_Tree<elemType>::inorderTraversal() const
{
    inorder(root);
}

template <class elemType>
void KD_Tree<elemType>::inorder(treeNode<elemType> *p) const
{
    if (p!=NULL)
    {
        inorder(p->llink);
        cout << "(" << p->x << "," << p->y << ")" << endl;
        inorder(p->rlink);
    }
}

template <class elemType>
int KD_Tree<elemType>::ReportSubtree(treeNode<elemType> *p)
{
    int s = 0;
    if (p!=NULL)
    {
        s += ReportSubtree(p->llink);
        if (isLeaf(p)) {
            // cout << "(" << p->x << "," << p->y << ")" << endl;
            s += 1;
        }
        s += ReportSubtree(p->rlink);
    }
    return s;
}

template <class elemType>
void KD_Tree<elemType>::destroyTree()
{
    destroy(root);
}

template <class elemType>
void KD_Tree<elemType>::destroy(treeNode<elemType>* &p)
{
    if (p != NULL)
    {
        destroy(p->llink);
        destroy(p->rlink);
        delete p;
        p=NULL;
    }
}


template <class elemType>
bool KD_Tree<elemType>::isLeaf(treeNode<elemType>* p) const
{
    if( (p!= NULL) and p->llink == NULL and p->rlink == NULL)
        return(true);
    else
        return(false);
}





#endif