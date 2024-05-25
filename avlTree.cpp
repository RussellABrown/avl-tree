/*
 * Copyright (c) 1996, 2016, 2023, 2024, Russell A. Brown
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
 * AVL tree-building program adapted from Pascal procedures
 * 4.63 (p. 220) and 4.64 (p. 223) of Nicklaus Wirth's textbook,
 * "Algorithms + Data Structures = Programs" with correction
 * of the bug in the del procedure and bifurcation of that
 * procedure into the eraseLeft and eraseRight methods.  The
 * eraseRight method performs the identical operations to del,
 * whereas the eraseLeft method performs the mirror-image
 * operations to eraseRight in an attempt to improve rebalancing
 * efficiency after deletion.
 *
 * To build the test executable, compile via: g++ -std=c++11 -O3 -D TEST_AVL_TREE avlTree.cpp
 */

#include <iostream>
#include <list>
#include <vector>

using namespace std;

/*
 * The avlTree class defines the root of the AVL tree as well as the avlNode class.
 */
template <typename T>
class avlTree {

    /* The avlNode class defines a node in the AVL tree. */

    class avlNode {

    private:
        T key;          /* the key stored in this avlNode */
        size_t copies;  /* the number of attempts to insert the key into the tree */
        int bal;        /* the left/right balance that assumes values of -1, 0, or +1 */
        avlNode *left, *right;
        
        /*
         * Here is the constructor for the avlNode class.
         *
         * Calling parameters:
         * 
         * @param x (IN) the key to store in the avlNode
         * @param h (MODIFIED) specifies that the tree height has changed
         */
    public:
        avlNode( T const& x, bool& h ) {
            h = true;
            key = x;
            copies = 1;
            bal = 0;
            left = right = nullptr;
        }
        
        /* This method searches the tree for the existence of a key.
         *
         * The "this" pointer is copied to the "p" pointer that is
         * replaced by either the left or right child pointer as the
         * search iteratively descends through the tree.
         * 
         * Calling parameter:
         *
         * @param x (IN) the key to search for
         * 
         * @return true if the key was found; otherwise, false
         */
    public:
        bool contains( T const& x) {
            
            avlNode* p = this;
            
            while ( p != nullptr ) {                    /* iterate; don't use recursion */
                if ( x < p->key ) {
                    p = p->left;                        /* follow the left branch */
                } else if ( x > p->key ) {
                    p = p->right;                       /* follow the right branch */
                } else {
                    return true;                        /* found the key, so return true */
                }
            }
            return false;                               /* didn't find the key, so return false */
        }
        
        /*
         * This method searches the tree recursively for the existence
         * of a key, and either adds the key as a new avlNode or
         * increments the "copies" counter. Then the tree is rebalanced
         * if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-tree.
         *
         * Calling parameters:
         *
         * @param x (IN) the key to add to the tree
         * @param h (MODIFIED) if true, the height of the tree has changed
         * @param a (MODIFIED) if true, the key was added as a new avlNode
         * 
         * @return the root of the rebalanced sub-tree
         */
    public:
        avlNode* insert( T const& x, bool& h, bool& a ) {
            
            avlNode* p = this;
            
            if ( x < p->key ) {                         /* search the left branch? */
                if ( p->left != nullptr ) {
                    p->left = left->insert( x, h, a );
                } else {
                    p->left = new avlNode( x, h );
                    a = true;
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
                            avlNode* p1 = p->left;
                            if ( p1->bal == -1 ) {		/* single LL rotation */
                                p->left = p1->right;
                                p1->right = p;
                                p->bal = 0;
                                p = p1;
                            } else {			        /* double LR rotation */
                                avlNode* p2 = p1->right;
                                p1->right = p2->left;
                                p2->left = p1;
                                p->left = p2->right;
                                p2->right = p;
                                if ( p2->bal == -1 ) {
                                    p->bal = 1;
                                } else {
                                    p->bal = 0;
                                }
                                if ( p2->bal == 1 ) {
                                    p1->bal = -1;
                                } else {
                                    p1->bal = 0;
                                }
                                p = p2;
                            }
                            p->bal = 0;
                            h = false;
                            break;
                    }
                }
            } else if ( x > p->key ) {                  /* search the right branch? */
                if ( p->right != nullptr ) {
                    p->right = right->insert( x, h, a );
                } else {
                    p->right = new avlNode( x, h );
                    a = true;
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
                            avlNode* p1 = p->right;
                            if ( p1->bal == 1 ) {       /* single RR rotation */
                                p->right = p1->left;
                                p1->left = p;
                                p->bal = 0;
                                p = p1;
                            } else {                    /* double RL rotation */
                                avlNode* p2 = p1->left;
                                p1->left = p2->right;
                                p2->right = p1;
                                p->right = p2->left;
                                p2->left = p;
                                if ( p2->bal == 1 ) {
                                    p->bal = -1;
                                } else {
                                    p->bal = 0;
                                }
                                if ( p2->bal == -1 ) {
                                    p1->bal = 1;
                                } else {
                                    p1->bal = 0;
                                }
                                p = p2;
                            }
                            p->bal = 0;
                            h = false;
                            break;
                    }
                }
            } else {  /* the key is already in tree, so increment "copies" and don't modify the tree */
                p->copies++;
                h = false;
                a = false;
            }
            return p;  /* the root of the rebalanced sub-tree */
        }
        
        /*
         * This method rebalances following deletion of a
         * left avlNode.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-tree.
         * 
         * Calling parameter:
         * 
         * @param h (MODIFIED) if true, the height of the tree has changed
         * 
         * @return the root of the rebalanced sub-tree
         */
    private:
        avlNode* balanceLeft( bool& h ) {
            
            avlNode* p = this;
            
            switch ( p->bal ) {
                case -1:                    /* balance restored */
                    p->bal = 0;
                    break;
                case 0:                     /* tree has become more unbalanced */
                    p->bal = 1;
                    h = false;
                    break;
                case 1:                     /* tree must be rebalanced */
                    avlNode* p1 = p->right;
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
                        avlNode* p2 = p1->left;
                        p1->left = p2->right;
                        p2->right = p1;
                        p->right = p2->left;
                        p2->left = p;
                        if ( p2->bal == 1 ) {
                            p->bal = -1;
                        } else {
                            p->bal = 0;
                        }
                        if ( p2->bal == -1 ) {
                            p1->bal = 1;
                        } else {
                            p1->bal = 0;
                        }
                        p = p2;
                        p->bal = 0;
                    }
                    break;
            }
            return p; /* the root of the rebalanced sub-tree */
        }
        
        /*
         * This method rebalances following deletion of a
         * right avlNode.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-tree.
         * 
         * Calling parameter:
         * 
         * @param h (MODIFIED) if true, the height of the tree has changed
         * 
         * @return the root of the rebalanced sub-tree
         */
    private:
        avlNode* balanceRight( bool& h ) {
            
            avlNode* p = this;
            
            switch ( p->bal ) {
                case 1:                     /* balance restored */
                    p->bal = 0;
                    break;
                case 0:                     /* tree has become more unbalanced */
                    p->bal = -1;
                    h = false;
                    break;
                case -1:                    /* tree must be rebalanced */
                    avlNode* p1 = p->left;
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
                        avlNode* p2 = p1->right;
                        p1->right = p2->left;
                        p2->left = p1;
                        p->left = p2->right;
                        p2->right = p;
                        if ( p2->bal == -1 ) {
                            p->bal = 1;
                        } else {
                            p->bal = 0;
                        }
                        if ( p2->bal == 1 ) {
                            p1->bal = -1;
                        } else {
                            p1->bal = 0;
                        }
                        p = p2;
                        p->bal = 0;
                    }
                    break;
            }
            return p;  /* the root of the rebalanced sub-tree */
        }
        
        /*
         * This method replaces the avlNode to be deleted with the
         * leftmost avlNode of the right sub-tree. Then the tree is
         * rebalanced if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-tree.
         * 
         * Calling parameters:
         * 
         * @param q (MODIFIED) the avlNode to be deleted
         * @param h (MODIFIED) if true, the height of the tree has changed
         * 
         * @return the root of the rebalanced sub-tree
         */
     private:
        avlNode* eraseLeft( avlNode*& q, bool& h ) {
            
            avlNode* p = this;
            
            if ( p->left != nullptr ) {
                p->left = left->eraseLeft( q, h );
                if ( h == true ) p = balanceLeft( h );
            } else {
                q->key = p->key;                /* copy avlNode contents from p to q */
                q->copies = p->copies;
                q = p;                          /* redefine q as avlNode to be deleted */
                p = p->right;                   /* replace avlNode with right branch */
                h = true;
            }
            return p;  /* the root of the rebalanced sub-tree */
        }
        
        /*
         * This method replaces the avlNode to be deleted with the
         * rightmost avlNode of the left sub-tree. Then the tree is
         * rebalanced if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-tree.
         * 
         * Calling parameters:
         * 
         * @param q (MODIFIED) the avlNode to be deleted
         * @param h (MODIFIED) if true, the height of the tree has changed
         * 
         * @return the root of the rebalanced sub-tree
         */
    private:
        avlNode* eraseRight( avlNode*& q, bool& h ) {
            
            avlNode* p = this;
            
            if ( p->right != nullptr ) {
                p->right = p->right->eraseRight( q, h );
                if ( h == true ) p = balanceRight( h );
            } else {
                q->key = p->key;                /* copy avlNode contents from p to q */
                q->copies = p->copies;
                q = p;                          /* redefine q as avlNode to be deleted  */
                p = p->left;                    /* replace avlNode with left branch */
                h = true;
            }
            return p;  /* the root of the rebalanced sub-tree */
        }
        
        /*
         * This method removes an avlNode from the tree. Then
         * the tree is rebalanced if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-tree.
         * 
         * Calling parameters:
         * 
         * @param x (IN) the key to remove from the tree
         * @param h (MODIFIED) if true, the height of the tree has changed
         * @param r (MODIFIED) if true, the key was removed from the tree
         * 
         * @return the root of the rebalanced sub-tree
        */
     public:
        avlNode* erase( T const& x, bool& h, bool& r ) {
            
            avlNode* p = this;
            
            if ( x < p->key ) {                     /* search left branch? */
                if ( p->left != nullptr ) {
                    p->left = p->left->erase( x, h, r );
                    if ( h ) {
                        p = balanceLeft( h );
                    }
                } else {
                    h = false;                      /* key is not in the tree*/
                    r = false;
                }
            } else if ( x > p->key ) {              /* search right branch? */
                if ( p->right != nullptr ) {
                    p->right = p->right->erase( x, h, r );
                    if ( h ) {
                        p = balanceRight( h );
                    }
                } else {
                    h = false;                      /* key is not in the tree */
                    r = false;
                }
            } else if ( p->copies-- <= 1 ) {        /* x == key and not redundant, so... */
                avlNode* q = p;                     /* ...select this avlNode for removal */
                if ( p->right == nullptr ) {        /* if one branch is nullptr... */
                    p = p->left;
                    h = true;
                } else if ( p->left == nullptr ) {  /* ...replace with the other one */
                    p = p->right;
                    h = true;
                } else {
                    switch ( p->bal ) {             /* otherwise find an avlNode to remove */
                        case 0: case -1:            /* left or neither subtree is deeper */
                            p->left = p->left->eraseRight( q, h );
                            if ( h == true ) {
                                p = balanceLeft( h );
                            }
                            break;
                        case 1:                     /* right subtree is deeper */
                            p->right = p->right->eraseLeft( q, h );
                            if ( h == true ) {
                                p = balanceRight( h );
                            }
                            break;
                    }
                }
                delete q;
                r = true;
            }
            return p;  /* the root of the rebalanced sub-tree */
        }
        
        /*
         * This method prints the keys stored in the tree, where the
         * key of the root of the tree is at the left and the keys of
         * the leaf nodes are at the right.
         * 
         * Calling parameter:
         * 
         * @param d (MODIFIED) the depth in the tree
         */
    public:
        void printTree( int d ) {
            if ( right != nullptr ) {
                right->printTree( d+1 );
            }
            for ( int i = 0; i < d; ++i ) {
                cout << "    ";
            }
            cout << key << "\n";
            if ( left ) {
                left->printTree( d+1 );
            }
        }
        
        /*
         * This method deletes every avlNode in the tree.  If the
         * tree has been completely deleted via prior calls to the
         * erase() method, the ~tree() destructor will not call
         * this method.
         */
    public:
        void clear() {
            
            if ( left != nullptr ) {
                left->clear();
                left = nullptr;
            }
            if ( right != nullptr ){
                right->clear();
                right = nullptr;
            }
            delete this;
        }
        
        /*
         * This method walks the tree in order and stores each key in a vector.
         *
         * Calling parameters:
         * 
         * @param v (MODIFIED) vector of the keys
         * @param i (MODIFIED) index to the next unoccupied vector element
         */       
    public:
        void getKeys( vector<T>& v, size_t& i ) {

            if ( left != nullptr ) {
                left->getKeys( v, i );
            }
            v[i++] = this->key;
            if ( right != nullptr ){
                right->getKeys( v, i );
            }
        }
    };

private:
    avlNode* root;  /* the root of the tree */
    size_t count;   /* the number of nodes in the tree */
   
public:
    avlTree() {
        root = nullptr;
        count = 0;
    }
    
public:
    ~avlTree() {
        if ( root != nullptr ) {
            root->clear();
        }
        root = nullptr;
        count = 0;
    }

    /* This method returns the number of avlNodes in the tree. */
public:
    size_t size() {
        return count;
    }

    /* This method returns true if there are no avlNodes in the tree. */
public:
    bool empty() {
        return ( count == 0 );
    }

    /* This method searches the tree for the existence of a key.
     *
     * Calling parameter:
     *
     * x - the key to search for
     * 
     * return - true if the key was found; otherwise, false
     */
public:
    bool contains( T const& x ) {
        if (root != nullptr) {
            return root->contains( x );
        } else {
            return false;
        }
    }
    
    /*
     * This method searches the tree for the existence of
     * a key, and either adds the key as a new avlNode or
     * increments an existing avlNode's "copies" counter.
     * Then the tree is rebalanced if necessary.
     *
     * Calling parameter:
     *
     * @param x (IN) the key to add to the tree
     * 
     * @return true if the key was added as a new avlNode; otherwise, false
     */
public:
    bool insert( T const& x ) {
        bool h = false, a = false;
        if ( root != nullptr ) {
            root = root->insert( x, h, a );
            if ( a == true ) {
                ++count;
                return true;
            } else {
                return false;
            }
        } else {
            root = new avlNode( x, h );
            ++count;
            return true;
        }
    }

    /*
     * This method removes an avlNode from the tree.
     * Then the tree is rebalanced if necessary.
     * 
     * Calling parameter:
     * 
     * @param x (IN) the key to remove from the tree
     * 
     * @return true if the key was removed from the tree; otherwise, false
     */
public:
    bool erase( T const& x ) {
        bool h = false, r = false;
        if ( root != nullptr ) {
            root = root->erase( x, h, r );
            if ( r == true ) {
                --count;
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }

    /*
     * This method prints the keys stored in the tree, where the
     * key of the root of the tree is at the left and the keys of
     * the leaf nodes are at the right.
     */
public:
    void printTree() {
        if ( root != nullptr ) {
            root->printTree( 0 );
        }
    }

    /*
     * This method walks the tree in order and stores each key in a vector.
     *
     * Calling parameter:
     * 
     * @param v (MODIFIED) vector of the keys
     */
public:
    void getKeys( vector<T>& v ) {
        if ( root != nullptr ) {
            size_t i = 0;
            root->getKeys( v, i );
        }
    }
};

#ifdef TEST_AVL_TREE

int main() {

    /* 22 keys, one of which (14) is duplicated */
    
   vector<int> const keys{ 8, 9, 11, 15, 19, 20, 21, 7, 3, 2, 1, 5, 6, 4, 13, 14, 10, 12, 14, 17, 16, 18 };

   /* Present and missing keys */

    int const presentKey = 13, missingKey = 0;
    
    char ch;
    avlTree<int>* t = new avlTree<int>();

    /* Add each key to the AVL tree. */

    for ( size_t i = 0; i < keys.size(); ++i ) {
        cout << endl << "press return to add " << keys[i] << endl;
        ch = cin.get();
        t->insert( keys[i] );
        cout << "tree contains " << t->size() << " nodes" << endl << endl;
        t->printTree();
    }
    cout << endl << "*** balanced tree completed; ordered keys follow ***" << endl << endl;

    /*
     * Retrieve the keys sorted in ascending order and store them in a vector.
     * Pre-allocate the vector to avoid re-sizing it.
     */

    vector<int> sortedKeys( t->size() );
    t->getKeys( sortedKeys );
    for (size_t i = 0; i < sortedKeys.size(); ++i) {
        cout << sortedKeys[i] << " ";
    }
    cout << endl;

    /* Test the contains() function. */

    if ( t->contains( presentKey ) == false ) {
        cout << endl << "error: failed to find key " << presentKey << endl;
    }
    if ( t->contains( missingKey ) == true ) {
        cout << endl << "error: found key " << missingKey << endl;
    }

    /* Test the erase() function for a missing key. */

    if ( t->erase( missingKey ) == true ) {
        cout << endl << "error: removed key " << missingKey << endl;
    }

    /* Delete each key from the AVL tree. */

    for ( size_t i = 0; i < keys.size(); ++i ) {
        cout << endl << "press return to remove " << keys[i] << endl;
        ch = cin.get();
        t->erase( keys[i] );
        cout << "tree contains " << t->size() << " nodes" << endl << endl;
        t->printTree();
    }
    cout << "all done" << endl << endl;
    
    return 0;
}

#endif
