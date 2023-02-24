/*
 * Copyright (c) 1996, 2016, 2023, Russell A. Brown
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software without
 *    specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 * OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * AVL tree building program adapted from Pascal procedures
 * 4.63 (p. 220) and 4.64 (p. 223) of Nicklaus Wirth's textbook,
 * "Algorithms + Data Structures = Programs", with correction
 * of the bug in the del procedure, which has bifurcated into
 * the removeLeft and removeRight methods.
 */

#include <iostream>
#include <stdexcept>

using namespace std;

/* The class node defines the nodes in the tree. */
template <typename T>
class node {
private:
    T key;
    int count, bal;
    node<T> *left, *right;
    
    /*
     * Here is the constructor for the class node.
     * The argument h is passed by reference and modified.
     */
public:
    node( T const& x, bool& h ) {
        h = true;				/* tree height has changed */
        key = x;
        count = 1;
        bal = 0;
        left = right = nullptr;
    }
    
    /*
     * This method searches the tree for the existence of a key,
     * and either inserts a new node or increments a counter.  The
     * tree is then rebalanced recursively backtracking up the tree.
     * The variable h is passed by reference and modified. The
     * "this" pointer is copied to the "p" pointer which is
     * modified and returned.
     */
public:
    node<T>* addNode( T const& x, bool& h ) {
        
        node<T>* p = this;
        
        if ( x < p->key ) {                         /* search the left branch? */
            if ( p->left != nullptr ) {
                p->left = left->addNode( x, h );
            } else {
                p->left = new node<T>( x, h );
            }
            if ( h == true ) {                      /* left branch has grown higher */
                switch ( p->bal ) {
                    case 1:                         /* balance restored */
                        p->bal = 0;
                        h = false;
                        break;
                    case 0:                         /* tree has become more unbalanced */
                        p->bal = -1;
                        break;
                    case -1:		                /* tree must be rebalanced */
                        node<T>* p1 = p->left;
                        if ( p1->bal == -1 ) {		/* single LL rotation */
                            p->left = p1->right;
                            p1->right = p;
                            p->bal = 0;
                            p = p1;
                        } else {			        /* double LR rotation */
                            node<T>* p2 = p1->right;
                            p1->right = p2->left;
                            p2->left = p1;
                            p->left = p2->right;
                            p2->right = p;
                            if ( p2->bal == -1 ) p->bal = 1;
                            else p->bal = 0;
                            if ( p2->bal == 1 ) p1->bal = -1;
                            else p1->bal = 0;
                            p = p2;
                        }
                        p->bal = 0;
                        h = false;
                        break;
                }
            }
        } else if ( x > p->key ) {                  /* search the right branch? */
            if ( p->right != nullptr ) {
                p->right = right->addNode( x, h );
            } else {
                p->right = new node<T>( x, h );
            }
            if ( h == true ) {                      /* right branch has grown higher */
                switch ( p->bal ) {
                    case -1:                        /* balance restored */
                        p->bal = 0;
                        h = false;
                        break;
                    case 0:                         /* tree has become more unbalanced */
                        p->bal = 1;
                        break;
                    case 1:                         /* tree must be rebalanced */
                        node<T>* p1 = p->right;
                        if ( p1->bal == 1 ) {       /* single RR rotation */
                            p->right = p1->left;
                            p1->left = p;
                            p->bal = 0;
                            p = p1;
                        } else {                    /* double RL rotation */
                            node<T>* p2 = p1->left;
                            p1->left = p2->right;
                            p2->right = p1;
                            p->right = p2->left;
                            p2->left = p;
                            if ( p2->bal == 1 ) p->bal = -1;
                            else p->bal = 0;
                            if ( p2->bal == -1 ) p1->bal = 1;
                            else p1->bal = 0;
                            p = p2;
                        }
                        p->bal = 0;
                        h = false;
                        break;
                }
            }
        } else {            /* x is already in tree, so increment its redundancy */
            p->count++;
            h = false;
        }
        return p;
    }
    
    /*
     * This method rebalances following deletion of a
     * left node.  The argument h is passed by reference and
     * modified.  The "this" pointer is copied to the "p"
     * pointer which is modified and returned.
     */
private:
    node<T>* balanceLeft( bool& h ) {
        
        node<T>* p = this;
        
        switch ( p->bal ) {
            case -1:                    /* balance restored */
                p->bal = 0;
                break;
            case 0:                     /* tree has become more unbalanced */
                p->bal = 1;
                h = false;
                break;
            case 1:                     /* tree must be rebalanced */
                node<T>* p1 = p->right;
                if ( p1->bal >= 0 ) {   /* single RR rotation */
                    p->right = p1->left;
                    p1->left = p;
                    if ( p1->bal == 0 ) {
                        p->bal = 1;
                        p1->bal = -1;
                        h = false;
                    } else {
                        p->bal = 0;
                        p1->bal = 0;
                    }
                    p = p1;
                } else {				  /* double RL rotation */
                    node<T>* p2 = p1->left;
                    p1->left = p2->right;
                    p2->right = p1;
                    p->right = p2->left;
                    p2->left = p;
                    if ( p2->bal == 1 ) p->bal = -1;
                    else p->bal = 0;
                    if ( p2->bal == -1 ) p1->bal = 1;
                    else p1->bal = 0;
                    p = p2;
                    p->bal = 0;
                }
                break;
        }
        return p;
    }
    
    /*
     * This method rebalances following deletion of a
     * right node.  The argument h is passed by reference and
     * modified.  The "this" pointer is copied to the "p"
     * pointer which is modified and returned.
     */
private:
    node<T>* balanceRight( bool& h ) {
        
        node<T>* p = this;
        
        switch ( p->bal ) {
            case 1:                     /* balance restored */
                p->bal = 0;
                break;
            case 0:                     /* tree has become more unbalanced */
                p->bal = -1;
                h = false;
                break;
            case -1:                    /* tree must be rebalanced */
                node<T>* p1 = p->left;
                if ( p1->bal <= 0 ) {   /* single LL rotation */
                    p->left = p1->right;
                    p1->right = p;
                    if ( p1->bal == 0 ) {
                        p->bal = -1;
                        p1->bal = 1;
                        h = false;
                    } else {
                        p->bal = 0;
                        p1->bal = 0;
                    }
                    p = p1;
                } else {				  /* double LR rotation */
                    node<T>* p2 = p1->right;
                    p1->right = p2->left;
                    p2->left = p1;
                    p->left = p2->right;
                    p2->right = p;
                    if ( p2->bal == -1 ) p->bal = 1;
                    else p->bal = 0;
                    if ( p2->bal == 1 ) p1->bal = -1;
                    else p1->bal = 0;
                    p = p2;
                    p->bal = 0;
                }
                break;
        }
        return p;
    }
    
    /*
     * This method replaces the node to be deleted with the
     * leftmost node of the right subtree.  The arguments q and h
     * are passed by reference and modified.  The "this" pointer
     * is coped to the "p" pointer which is modified and returned.
     */
private:
    node<T>* removeLeft( node<T>*& q, bool& h ) {
        
        node<T>* p = this;
        
        if ( p->left != nullptr ) {
            p->left = left->removeLeft( q, h );
            if ( h == true ) p = balanceLeft( h );
        } else {
            q->key = p->key;                /* copy node contents to q */
            q->count = p->count;
            q = p;                          /* redefine q as node to be deleted */
            p = p->right;                   /* replace node with right branch */
            h = true;
        }
        return p;
    }
    
    /*
     * This method replaces the node to be deleted with the
     * rightmost node of the left subtree.  The arguments q and h
     * are passed by reference and modified.  The "this" pointer
     * is copied to the "p" pointer which is modified and returned.
     */
private:
    node<T>* removeRight( node<T>*& q, bool& h ) {
        
        node<T>* p = this;
        
        if ( p->right != nullptr ) {
            p->right = p->right->removeRight( q, h );
            if ( h == true ) p = balanceRight( h );
        } else {
            q->key = p->key;                /* copy node contents to q */
            q->count = p->count;
            q = p;                          /* redefine q as node to be deleted  */
            p = p->left;                    /* replace node with left branch */
            h = true;
        }
        return p;
    }
    
    /*
     * This method removes a node from the tree, then
     * rebalances recursively by backtracking up the tree
     * The argument h is passed by reference and modified. The
     * "this" pointer is coped to the "p" pointer which
     * is modified and returned.
     */
public:
    node<T>* removeNode( T const& x, bool& h ) {
        
        node<T>* p = this;
        
        if ( x < p->key ) {                     /* search left branch? */
            if ( p->left != nullptr ) {
                p->left = p->left->removeNode( x, h );
                if ( h ) {
                    p = balanceLeft( h );
                }
            } else {
                cout << "key " << x << " is not in tree\n";
                h = false;
            }
        } else if ( x > p->key ) {                /* search right branch? */
            if ( p->right != nullptr ) {
                p->right = p->right->removeNode( x, h );
                if ( h ) {
                    p = balanceRight( h );
                }
            } else {
                cout << "key " << x << " is not in tree\n";
                h = false;
            }
        } else if ( p->count-- <= 1 ) {         /* x == key and not redundant, so... */
            node<T>* q = p;                     /* ...select this node for removal */
            if ( p->right == nullptr ) {        /* if one branch is nullptr... */
                p = p->left;
                h = true;
            } else if ( p->left == nullptr ) {  /* ...replace with the other one */
                p = p->right;
                h = true;
            } else {
                switch ( p->bal ) {             /* otherwise find a node to remove */
                    case 0: case -1:            /* left or neither subtree is deeper */
                        p->left = p->left->removeRight( q, h );
                        if ( h == true ) {
                            p = balanceLeft( h );
                        }
                        break;
                    case 1:                     /* right subtree is deeper */
                        p->right = p->right->removeLeft( q, h );
                        if ( h == true ) {
                            p = balanceRight( h );
                        }
                        break;
                }
            }
            delete q;
        }
        return p;
    }
    
    /* This method prints each node of the tree. */
    
    void printTree( int l ) {
        if ( right != nullptr ) {
            right->printTree( l+1 );
        }
        for ( int i = 0; i < l; i++ ) {
            cout << "    ";
        }
        cout << key << "\n";
        if ( left ) {
            left->printTree( l+1 );
        }
    }
    
    /*
     * This method deletes every node in the tree.  If the
     * tree has been completely deleted via calls to remove,
     * this function won't be called.
     */
    
    void deleteTree() {
        
        if ( right != nullptr ){
            right->deleteTree();
            right = nullptr;
        }
        if ( left != nullptr ) {
            left->deleteTree();
            left = nullptr;
        }
        delete this;
    }
};

/*
 * The class tree defines the root of the tree, and is a
 * container class for the nodes of the tree.
 */
template <typename T>
class tree {
private:
    node<T>* root;
    
public:
    
    tree() {
        root = nullptr;
    }
    
    ~tree() {
        if ( root != nullptr ) {
            root->deleteTree();
        }
        root = nullptr;
    }
    
    void addNode( T const& x, bool& h ) {
        if ( root != nullptr ) {
            root = root->addNode( x, h );
        } else {
            root = new node<T>( x, h);
        }
    }
    
    void removeNode( T const& x, bool& h ) {
        if ( root != nullptr ) {
            root = root->removeNode( x, h );
        }
    }
    
    void printTree() {
        if ( root != nullptr ) {
            root->printTree( 0 );
        }
    }
    
};

int main() {
    
    const int size = 22;
    
    int array[size] = { 8, 9, 11, 15, 19, 20, 21, 7,
		      3, 2, 1, 5, 6, 4, 13, 14, 10,
		      12, 14, 17, 16, 18 };
    
    char ch;
    bool h = false;
    tree<int>* t = new tree<int>();
    t->printTree();
    cout << endl;
    for ( int i = 0; i < size; i++ ) {
        cout << "press return to add " << array[i] << endl;
        ch = cin.get();
        t->addNode( array[i], h );
        t->printTree();
        cout << endl;
    }
    cout << "*** balanced tree completed ***" << endl << endl;
    for ( int i = 0; i < size; i++ ) {
        cout << "press return to remove " << array[i] << endl;
        ch = cin.get();
        t->removeNode( array[i], h );
        t->printTree();
        cout << endl;
    }
    cout << "all done" << endl << endl;
    
    return 0;
}
