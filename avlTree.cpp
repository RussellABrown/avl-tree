/*
 * Modifications Copyright (c) 1996, 2016, 2023, 2024 Russell A. Brown
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
 * AVL tree-building program modified from Pascal procedures
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

#ifndef AVL_TREE_CPP
#define AVL_TREE_CPP

#include <iostream>
#include <exception>
#include <sstream>
#include <vector>

/*
 * The avlTree class defines the root of the AVL tree and provides the
 * h, a, and r boolean variables to the avlNode class.
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
         * @param t (IN) a pointer to the avlTree instance
         * @param x (IN) the key to add to the tree
         * 
         * @return the root of the rebalanced sub-tree
         */
    public:
        avlNode* insert( avlTree* const t,  T const& x ) {
            
            avlNode* p = this;
            
            if ( x < p->key ) {                         /* search the left branch? */
                if ( p->left != nullptr ) {
                    p->left = left->insert( t, x );
                } else {
                    p->left = new avlNode( x, t->h );
                    t->a = true;
                }
                if ( t->h == true ) {                   /* left branch has grown higher */
                    switch ( p->bal ) {
                        case 1:                         /* balance restored */
                            p->bal = 0;
                            t->h = false;
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
                            t->h = false;
                            break;
                    }
                }
            } else if ( x > p->key ) {                  /* search the right branch? */
                if ( p->right != nullptr ) {
                    p->right = right->insert( t, x );
                } else {
                    p->right = new avlNode( x, t->h );
                    t->a = true;
                }
                if ( t->h == true ) {                   /* right branch has grown higher */
                    switch ( p->bal ) {
                        case -1:                        /* balance restored */
                            p->bal = 0;
                            t->h = false;
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
                            t->h = false;
                            break;
                    }
                }
            } else {  /* the key is already in tree, so increment "copies" and don't modify the tree */
                p->copies++;
                t->h = false;
                t->a = false;
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
         * @param t (IN) a pointer to the avlTree instance
         * 
         * @return the root of the rebalanced sub-tree
         */
    private:
        avlNode* balanceLeft( avlTree* const t ) {
            
            avlNode* p = this;
            
            switch ( p->bal ) {
                case -1:                    /* balance restored */
                    p->bal = 0;
                    break;
                case 0:                     /* tree has become more unbalanced */
                    p->bal = 1;
                    t->h = false;
                    break;
                case 1:                     /* tree must be rebalanced */
                    avlNode* p1 = p->right;
                    if ( p1->bal >= 0 ) {   /* single RR rotation */
                        p->right = p1->left;
                        p1->left = p;
                        if ( p1->bal == 0 ) {
                            p->bal = 1;
                            p1->bal = -1;
                            t->h = false;
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
         * @param t (IN) a pointer to the avlTree instance
         * 
         * @return the root of the rebalanced sub-tree
         */
    private:
        avlNode* balanceRight( avlTree* const t ) {
            
            avlNode* p = this;
            
            switch ( p->bal ) {
                case 1:                     /* balance restored */
                    p->bal = 0;
                    break;
                case 0:                     /* tree has become more unbalanced */
                    p->bal = -1;
                    t->h = false;
                    break;
                case -1:                    /* tree must be rebalanced */
                    avlNode* p1 = p->left;
                    if ( p1->bal <= 0 ) {   /* single LL rotation */
                        p->left = p1->right;
                        p1->right = p;
                        if ( p1->bal == 0 ) {
                            p->bal = -1;
                            p1->bal = 1;
                            t->h = false;
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
         * @param t (IN) a pointer to the avlTree instance
         * @param q (MODIFIED) the avlNode to be deleted
         * 
         * @return the root of the rebalanced sub-tree
         */
     private:
        avlNode* eraseLeft( avlTree* const t, avlNode*& q ) {
            
            avlNode* p = this;
            
            if ( p->left != nullptr ) {
                p->left = left->eraseLeft( t, q );
                if ( t->h == true ) p = balanceLeft( t );
            } else {
                q->key = p->key;                /* copy avlNode contents from p to q */
                q->copies = p->copies;
                q = p;                          /* redefine q as avlNode to be deleted */
                p = p->right;                   /* replace avlNode with right branch */
                t->h = true;
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
         * @param t (IN) a pointer to the avlTree instance
         * @param q (MODIFIED) the avlNode to be deleted
         * 
         * @return the root of the rebalanced sub-tree
         */
    private:
        avlNode* eraseRight( avlTree* const t, avlNode*& q ) {
            
            avlNode* p = this;
            
            if ( p->right != nullptr ) {
                p->right = p->right->eraseRight( t, q );
                if ( t->h == true ) p = balanceRight( t );
            } else {
                q->key = p->key;                /* copy avlNode contents from p to q */
                q->copies = p->copies;
                q = p;                          /* redefine q as avlNode to be deleted  */
                p = p->left;                    /* replace avlNode with left branch */
                t->h = true;
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
         * @param t (IN) a pointer to the avlTree instance
         * @param x (IN) the key to remove from the tree
         * 
         * @return the root of the rebalanced sub-tree
        */
     public:
        avlNode* erase( avlTree* const t, T const& x ) {
            
            avlNode* p = this;
            
            if ( x < p->key ) {                     /* search left branch? */
                if ( p->left != nullptr ) {
                    p->left = p->left->erase( t, x );
                    if ( t->h ) {
                        p = balanceLeft( t );
                    }
                } else {
                    t->h = false;                   /* key is not in the tree*/
                    t->r = false;
                }
            } else if ( x > p->key ) {              /* search right branch? */
                if ( p->right != nullptr ) {
                    p->right = p->right->erase( t, x );
                    if ( t->h ) {
                        p = balanceRight( t );
                    }
                } else {
                    t->h = false;                   /* key is not in the tree */
                    t->r = false;
                }
            } else if ( p->copies-- <= 1 ) {        /* x == key and not redundant, so... */
                avlNode* q = p;                     /* ...select this avlNode for removal */
                if ( p->right == nullptr ) {        /* if one branch is nullptr... */
                    p = p->left;
                    t->h = true;
                } else if ( p->left == nullptr ) {  /* ...replace with the other one */
                    p = p->right;
                    t->h = true;
                } else {
                    switch ( p->bal ) {             /* otherwise find an avlNode to remove */
                        case 0: case -1:            /* left or neither subtree is deeper */
                            p->left = p->left->eraseRight( t, q );
                            if ( t->h == true ) {
                                p = balanceLeft( t );
                            }
                            break;
                        case 1:                     /* right subtree is deeper */
                            p->right = p->right->eraseLeft( t, q );
                            if ( t->h == true ) {
                                p = balanceRight( t );
                            }
                            break;
                        default:
                            {
                                std::ostringstream buffer;
                                buffer << std::endl << (p->bal) << " is out of range" << std::endl;
                                throw std::runtime_error(buffer.str());
                            }
                    }
                }
                delete q;
                t->r = true;
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
                std::cout << "    ";
            }
            std::cout << key << "\n";
            if ( left ) {
                left->printTree( d+1 );
            }
        }
        
        /*
         * This method deletes every avlNode in the tree.  If the
         * tree has been completely deleted via prior calls to the
         * erase() method, the ~avlTree() destructor will not call
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
        void getKeys( std::vector<T>& v, size_t& i ) {

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
    bool h, a, r;   /* record modification of the tree */
  
public:
    avlTree() {
        root = nullptr;
        count = 0;
        h = a = r = false;
    }
    
public:
    ~avlTree() {
        if ( root != nullptr ) {
            root->clear();
        }
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

    /*
     * This method searches the tree for the existence of a key.
     *
     * Calling parameter:
     *
     * @param x (IN) the key to search for
     * 
     * @return true if the key was found; otherwise, false
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
        h = false, a = true;
        if ( root != nullptr ) {
            root = root->insert( this, x );
            if ( a == true ) {
                ++count;
            }
        } else {
            root = new avlNode( x, h );
            ++count;
        }
        return a;
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
        h = false, r = false;
        if ( root != nullptr ) {
            root = root->erase( this, x );
            if ( r == true ) {
                --count;
            }
        }
        return r;
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

    /* This method deletes every avlNode in the AVL tree. */
public:
    void clear() {
        if ( root != nullptr ) {
            root->clear();
        }
        root = nullptr;
        count = 0;
    }

    /*
     * This method walks the tree in order and stores each key in a vector.
     *
     * Calling parameter:
     * 
     * @param v (MODIFIED) vector of the keys
     */
public:
    void getKeys( std::vector<T>& v ) {
        if ( root != nullptr ) {
            size_t i = 0;
            root->getKeys( v, i );
        }
    }
};

#ifdef TEST_AVL_TREE

int main() {

    using std::cin;
    using std::cout;
    using std::endl;
    using std::ostringstream;
    using std::runtime_error;
    using std::vector;

   /* 22 keys, one of which (14) is duplicated */
    
   vector<int> const keys{ 8, 9, 11, 15, 19, 20, 21, 7, 3, 2, 1, 5, 6, 4, 13, 14, 10, 12, 14, 17, 16, 18 };

   /* Present and missing keys */

    int const presentKey = 13, duplicatekey = 14, missingKey = 0;
    
    char ch;
    avlTree<int> t;

    /* Add each key to the AVL tree. */

    for ( size_t i = 0; i < keys.size(); ++i ) {
        cout << endl << "press return to add " << keys[i] << endl;
        ch = cin.get();
        if ( t.insert(keys[i]) == false && keys[i] != duplicatekey ) {
            cout << "error: failure to insert key " << keys[i] << endl;
        }
        cout << "tree contains " << t.size() << " nodes" << endl << endl;
        t.printTree();
    }

    cout << endl << "*** balanced tree completed; ordered keys follow ***" << endl << endl;

    /*
     * Retrieve the keys sorted in ascending order and store them in a vector.
     * Pre-allocate the vector to avoid re-sizing it.
     */

    vector<int> sortedKeys( t.size() );
    t.getKeys( sortedKeys );
    for (size_t i = 0; i < sortedKeys.size(); ++i) {
        cout << sortedKeys[i] << " ";
    }
    cout << endl;

    /* Test the contains() function. */

    if ( t.contains( presentKey ) == false ) {
        cout << endl << "error: does not contain key " << presentKey << endl;
    }
    if ( t.contains( missingKey ) == true ) {
        cout << endl << "error: contains missing key " << missingKey << endl;
    }

    /* Test the erase() function for a missing key. */

    if ( t.erase( missingKey ) == true ) {
        cout << endl << "error: erased missing key " << missingKey << endl;
    }

    /* Delete each key from the AVL tree. */

    for ( size_t i = 0; i < keys.size(); ++i ) {
        cout << endl << "press return to remove " << keys[i] << endl;
        ch = cin.get();
        if ( t.erase(keys[i]) == false && keys[i] != duplicatekey ) {
            cout << "error: failure to erase key " << keys[i] << endl;
        }
        cout << "tree contains " << t.size() << " nodes" << endl << endl;
        t.printTree();
    }
    cout << "all done" << endl << endl;
    
    return 0;
}

#endif // TEST_AVL_TREE

#endif // AVL_TREE_CPP
