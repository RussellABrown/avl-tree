/*
 * Modifications Copyright (c) 2007, 2016, 2023, 2024 Russell A. Brown
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
 * AVL map-building program modified from Pascal procedures
 * 4.63 (p. 220) and 4.64 (p. 223) of Nicklaus Wirth's textbook,
 * "Algorithms + Data Structures = Programs", with correction
 * of the bug in the del procedure and replacement of del
 * by the eraseRight and eraseLeft methods. The eraseRight
 * method performs the identical operations to del, whereas
 * the eraseLeft method performs the mirror-image operations
 * to eraseRight in an attempt to improve rebalancing efficiency
 * after deletion.
 * 
 * To build the test executable, compile via: g++ -std=c++11 -O3 -D TEST_AVL_MAP avlMap.cpp
 */

#ifndef AVL_MAP_CPP
#define AVL_MAP_CPP

#include <algorithm>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <exception>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <vector>

/*
 * The avlMap class defines the root of the AVL map and provides the
 * lli, lri, rli, rri, lle, lre, rle, and rre rotation counters
 * and the h, a, and r boolean variables to the avlNode class.
 */
template <typename K, typename V>
class avlMap {

    /* The avlNode class defines a node in the AVL map. */

    class avlNode {

    private:
        K key;          /* the key stored in this avlNode */
        V value;        /* the value stored in this avlNode */
        int bal;        /* the left/right balance that assumes values of -1, 0, or +1 */

    public:
        avlNode *left, *right;
        
        /*
         * Here is the constructor for the avlNode class.
         *
         * Calling parameters:
         * 
         * @param x (IN) the key to store in the avlNode
         * @param h (MODIFIED) specifies that the map height has changed
         */
    public:
        avlNode( K const& x, V const& y, bool& h ) {
            h = true;
            key = x;
            value = y;
            bal = 0;
            left = right = nullptr;
        }
        
        /*
         * This method searches the map for the existence of a key.
         *
         * Calling parameter:
         *
         * @param x (IN) the key to search for
         * 
         * @return true if the key was found; otherwise, false
         */
    public:
        bool contains( K const& x ) {

            if ( find(x) == nullptr ) {
                return false;
            } else {
                return true;
            }
        }

        /*
         * This method searches the map for the existence of a key
         * and returns a pointer to the associated value.
         *
         * The "this" pointer is copied to the "p" pointer that is
         * replaced by either the left or right child pointer as the
         * search iteratively descends through the map.
         * 
         * Calling parameter:
         *
         * @param x (IN) the key to search for
         * 
         * @return a pointer to the value if the key was found; otherwise, nullptr
         */
    public:
        V* find( K const& x ) {
            
            avlNode* p = this;
            
            while ( p != nullptr ) {                    /* iterate; don't use recursion */
                if ( x < p->key ) {
                    p = p->left;                        /* follow the left branch */
                } else if ( x > p->key ) {
                    p = p->right;                       /* follow the right branch */
                } else {
                    return &(p->value);                 /* found the key, so return pointer to value */
                }
            }
            return nullptr;                               /* didn't find the key, so return nullptr */
        }

        
        /*
         * This method searches the map recursively for the existence
         * of a key, and either inserts the (key, value) as a new avlNode
         * or updates the value. Then the map is rebalanced if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-map.
         *
         * Calling parameters:
         *
         * @param m (IN) a pointer to the avlMap instance
         * @param x (IN) the key to add to the map
         * @param y (IN) the value to add to the map
         */
    public:
        avlNode* insert( avlMap* const m, K const& x, V const& y ) {
            
            avlNode* p = this;
            
            if ( x < p->key ) {                         /* search the left branch? */
                if ( p->left != nullptr ) {
                    p->left = left->insert( m, x, y );
                } else {
                    p->left = new avlNode( x, y, m->h );
                    m->a = false;                       /* the present value is overwritten */
                }
                if ( m->h == true ) {                   /* left branch has grown higher */
                    switch ( p->bal ) {
                        case 1:                         /* balance restored */
                            p->bal = 0;
                            m->h = false;
                            break;
                        case 0:                         /* map has become more unbalanced */
                            p->bal = -1;
                            break;
                        case -1:		                /* map must be rebalanced */
                            avlNode* p1 = p->left;
                            if ( p1->bal == -1 ) {		/* single LL rotation */
                                m->lli++;
                                p->left = p1->right;
                                p1->right = p;
                                p->bal = 0;
                                p = p1;
                            } else {			        /* double LR rotation */
                                m->lri++;
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
                            m->h = false;
                            break;
                    }
                }
            } else if ( x > p->key ) {                  /* search the right branch? */
                if ( p->right != nullptr ) {
                    p->right = right->insert( m, x, y );
                } else {
                    p->right = new avlNode( x, y, m->h );
                    m->a = false;                       /* the present value is overwritten */
                }
                if ( m->h == true ) {                   /* right branch has grown higher */
                    switch ( p->bal ) {
                        case -1:                        /* balance restored */
                            p->bal = 0;
                            m->h = false;
                            break;
                        case 0:                         /* map has become more unbalanced */
                            p->bal = 1;
                            break;
                        case 1:                         /* map must be rebalanced */
                            avlNode* p1 = p->right;
                            if ( p1->bal == 1 ) {       /* single RR rotation */
                                m->rri++;
                                p->right = p1->left;
                                p1->left = p;
                                p->bal = 0;
                                p = p1;
                            } else {                    /* double RL rotation */
                                m->rli++;
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
                            m->h = false;
                            break;
                    }
                }
            } else {  /* the key is already in map, so update its value */
                p->value = y;
                m->h = false;
                m->a = true;
            }
            return p;  /* the root of the rebalanced sub-map */
        }
        
        /*
         * This method rebalances following deletion of a
         * left avlNode.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-map.
         * 
         * Calling parameters:
         * 
         * @param m (IN) a pointer to the avlMap instance
         * 
         * @return the root of the rebalanced sub-map
         */
    private:
        avlNode* balanceLeft( avlMap* const m ) {
            
            avlNode* p = this;
            
            switch ( p->bal ) {
                case -1:                    /* balance restored */
                    p->bal = 0;
                    break;
                case 0:                     /* map has become more unbalanced */
                    p->bal = 1;
                    m->h = false;
                    break;
                case 1:                     /* map must be rebalanced */
                    avlNode* p1 = p->right;
                    if ( p1->bal >= 0 ) {   /* single RR rotation */
                        m->rre++;
                        p->right = p1->left;
                        p1->left = p;
                        if ( p1->bal == 0 ) {
                            p->bal = 1;
                            p1->bal = -1;
                            m->h = false;
                        } else {
                            p->bal = 0;
                            p1->bal = 0;
                        }
                        p = p1;
                    } else {				  /* double RL rotation */
                        m->rle++;
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
            return p; /* the root of the rebalanced sub-map */
        }
        
        /*
         * This method rebalances following deletion of a
         * right avlNode.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-map.
         * 
         * Calling parameters:
         * 
         * @param m (IN) a pointer to the avlMap instance
         * 
         * @return the root of the rebalanced sub-map
         */
    private:
        avlNode* balanceRight( avlMap* const m ) {
            
            avlNode* p = this;
            
            switch ( p->bal ) {
                case 1:                     /* balance restored */
                    p->bal = 0;
                    break;
                case 0:                     /* map has become more unbalanced */
                    p->bal = -1;
                    m->h = false;
                    break;
                case -1:                    /* map must be rebalanced */
                    avlNode* p1 = p->left;
                    if ( p1->bal <= 0 ) {   /* single LL rotation */
                        m->lle++;
                        p->left = p1->right;
                        p1->right = p;
                        if ( p1->bal == 0 ) {
                            p->bal = -1;
                            p1->bal = 1;
                            m->h = false;
                        } else {
                            p->bal = 0;
                            p1->bal = 0;
                        }
                        p = p1;
                    } else {				  /* double LR rotation */
                        m->lre++;
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
            return p;  /* the root of the rebalanced sub-map */
        }
        
        /*
         * This method replaces the avlNode to be deleted with the
         * leftmost avlNode of the right sub-map. Then the map is
         * rebalanced if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-map.
         * 
         * Calling parameters:
         * 
         * @param m (IN) a pointer to the avlMap instance
         * @param q (MODIFIED) the avlNode to be deleted
         * 
         * @return the root of the rebalanced sub-map
         */
     private:
        avlNode* eraseLeft( avlMap* const m, avlNode*& q ) {
            
            avlNode* p = this;
            
            if ( p->left != nullptr ) {
                p->left = left->eraseLeft( m, q );
                if ( m->h == true ) p = balanceLeft( m );
            } else {
                q->key = p->key;                /* copy avlNode contents from p to q */
                q->value = p->value;
                q = p;                          /* redefine q as avlNode to be deleted */
                p = p->right;                   /* replace avlNode with right branch */
                m->h = true;
            }
            return p;  /* the root of the rebalanced sub-map */
        }
        
        /*
         * This method replaces the avlNode to be deleted with the
         * rightmost avlNode of the left sub-map. Then the map is
         * rebalanced if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-map.
         * 
         * Calling parameters:
         * 
         * @param m (IN) a pointer to the avlMap instance
         * @param q (MODIFIED) the avlNode to be deleted
         * 
         * @return the root of the rebalanced sub-map
         */
    private:
        avlNode* eraseRight( avlMap* const m, avlNode*& q ) {
            
            avlNode* p = this;
            
            if ( p->right != nullptr ) {
                p->right = p->right->eraseRight( m, q );
                if ( m->h == true ) p = balanceRight( m );
            } else {
                q->key = p->key;                /* copy avlNode contents from p to q */
                q->value = p->value;
                q = p;                          /* redefine q as avlNode to be deleted  */
                p = p->left;                    /* replace avlNode with left branch */
                m->h = true;
            }
            return p;  /* the root of the rebalanced sub-map */
        }
        
        /*
         * This method removes an avlNode from the map. Then
         * the map is rebalanced if necessary.
         * 
         * The "this" pointer is copied to the "p" pointer that is
         * possibly modified and is returned to represent the root of
         * the sub-map.
         * 
         * Calling parameters:
         * 
         * @param m (IN) a pointer to the avlMap instance
         * @param x (IN) the key to remove from the map
         * 
         * @return the root of the rebalanced sub-map
        */
     public:
        avlNode* erase( avlMap* const m, K const& x ) {
            
            avlNode* p = this;
            
            if ( x < p->key ) {                     /* search left branch? */
                if ( p->left != nullptr ) {
                    p->left = p->left->erase( m, x );
                    if ( m->h ) {
                        p = balanceLeft( m );
                    }
                } else {
                    m->h = false;                   /* key is not in the map*/
                    m->r = false;
                }
            } else if ( x > p->key ) {              /* search right branch? */
                if ( p->right != nullptr ) {
                    p->right = p->right->erase( m, x );
                    if ( m->h ) {
                        p = balanceRight( m );
                    }
                } else {
                    m->h = false;                   /* key is not in the map */
                    m->r = false;
                }
            } else {                                /* the search key equals the key for this node */
                avlNode* q = p;                     /* so select this avlNode for removal */
                if ( p->right == nullptr ) {        /* if one branch is nullptr... */
                    p = p->left;
                    m->h = true;
                } else if ( p->left == nullptr ) {  /* ...replace with the other one */
                    p = p->right;
                    m->h = true;
                } else {
                    switch ( p->bal ) {             /* otherwise find an avlNode to remove */
                        case 0: case -1:            /* left or neither submap is deeper */
                            p->left = p->left->eraseRight( m, q );
                            if ( m->h == true ) {
                                p = balanceLeft( m );
                            }
                            break;
                        case 1:                     /* right submap is deeper */
                            p->right = p->right->eraseLeft( m, q );
                            if ( m->h == true ) {
                                p = balanceRight( m );
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
                m->r = true;
            }
            return p;  /* the root of the rebalanced sub-map */
        }
        
        /*
         * This method prints the keys stored in the map, where the
         * key of the root of the map is at the left and the keys of
         * the leaf nodes are at the right.
         * 
         * Calling parameter:
         * 
         * @param d (MODIFIED) the depth in the map
         */
    public:
        void printMap( int d ) {
            if ( right != nullptr ) {
                right->printMap( d+1 );
            }
            for ( int i = 0; i < d; ++i ) {
                std::cout << "    ";
            }
            std::cout << key << "\n";
            if ( left ) {
                left->printMap( d+1 );
            }
        }
        
        /*
         * This method deletes every avlNode in the map.  If the
         * map has been completely deleted via prior calls to the
         * erase() method, the ~avlMap() destructor will not call
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
         * This method walks the map in order and stores each key in a vector.
         *
         * Calling parameters:
         * 
         * @param v (MODIFIED) vector of the keys
         * @param i (MODIFIED) index to the next unoccupied vector element
         */       
    public:
        void getKeys( std::vector<K>& v, size_t& i ) {

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
    avlNode* root;  /* the root of the map */
    size_t count;   /* the number of nodes in the map */
    bool h, a, r;   /* record modification of the map */

public:
    size_t lle, lre, rle, rre, lli, lri, rli, rri;  /* the rotation counters */
   
public:
    avlMap() {
        root = nullptr;
        lle = lre = rle = rre = lli = lri = rli = rri = count = 0;
        h = a = r = false;
    }
    
public:
    ~avlMap() {
        if ( root != nullptr ) {
            root->clear();
        }
    }

    /* This method returns the number of avlNodes in the map. */
public:
    size_t size() {
        return count;
    }

    /* This method returns true if there are no avlNodes in the map. */
public:
    bool empty() {
        return ( count == 0 );
    }

    /* This method searches the map for the existence of a key.
     *
     * Calling parameter:
     *
     * @param x (IN) the key to search for
     * 
     * @return true if the key was found; otherwise, false
     */
public:
    bool contains( K const& x ) {
        if (root != nullptr) {
            return root->contains( x );
        } else {
            return false;
        }
    }
    
    /* This method searches the map for the existence of a key
     * and returns the associated value.
     *
     * Calling parameter:
     *
     * @param x (IN) the key to search for
     * 
     * @return a pointer to the value if the key was found; otherwise, nullptr */
public:
    V* find( K const& x ) {
        if (root != nullptr) {
            return root->find( x );
        } else {
            return nullptr;
        }
    }
    
    /*
     * This method searches the map recursively for the existence
     * of a key, and either inserts the (key, value) as a new avlNode
     * or updates the value. Then the map is rebalanced if necessary.
     *
     * Calling parameter:
     *
     * @param x (IN) the key to add to the map
     * 
     * @return true if update, false if insertion
     */
public:
    bool insert( K const& x, V const& y ) {
        h = false, a = false;
        if ( root != nullptr ) {
            root = root->insert( this, x, y );
            if ( a == false ) {
                ++count;
            }
        } else {
            root = new avlNode( x, y, h );
            ++count;
        }
        return a;
    }

    /*
     * This method removes an avlNode from the map.
     * Then the map is rebalanced if necessary.
     * 
     * Calling parameter:
     * 
     * @param x (IN) the key to remove from the map
     * 
     * @return true if the key existed, false if not
     */
public:
    bool erase( K const& x ) {
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
     * This method prints the keys stored in the map, where the
     * key of the root of the map is at the left and the keys of
     * the leaf nodes are at the right.
     */
public:
    void printMap() {
        if ( root != nullptr ) {
            root->printMap( 0 );
        }
    }

    /* This method deletes every avlNode in the AVL map. */
public:
    void clear() {
        if ( root != nullptr ) {
            root->clear();
        }
        root = nullptr;
        count = 0;
    }

    /*
     * This method walks the map in order and stores each key in a vector.
     *
     * Calling parameter:
     * 
     * @param v (MODIFIED) vector of the keys
     */
public:
    void getKeys( std::vector<K>& v ) {
        if ( root != nullptr ) {
            size_t i = 0;
            root->getKeys( v, i );
        }
    }
};

#ifdef TEST_AVL_MAP

// A basic test
int main(int argc, char **argv) {
    
    using std::cout;
    using std::endl;
    using std::ostringstream;
    using std::runtime_error;
    using std::setprecision;
    using std::random_shuffle;
    using std::string;
    using std::vector;

   struct timespec startTime, endTime;
    size_t iterations = 100;

    // Read the words file into a dictionary. 
    vector<string> dictionary;
    char buf[512];
    FILE *f = fopen("words.txt", "r");
    while (fgets(buf, sizeof buf, f)) {
        size_t len = strlen(buf);
        buf[len-1] = '\0';
        dictionary.push_back(string(buf));
    }
    fclose(f);

    // Create a vector of unique unsigned integers as large as the number of words.
    vector<uint32_t> numbers;
    for (size_t i = 0; i < dictionary.size(); ++i) {
        numbers.push_back(i);
    }

    // Obtain statistics for an AVL tree that has a string key.
    avlMap<string, uint32_t> stringRoot;
    size_t stringMapSize;
    double createStringTime = 0, searchStringTime = 0, deleteStringTime = 0;
     for (size_t it = 0; it < iterations; ++it) {

         // Shuffle the dictionary and add each word to the AVL map.
        random_shuffle(dictionary.begin(), dictionary.end());
        clock_gettime(CLOCK_REALTIME, &startTime);
        for (size_t i = 0; i < dictionary.size(); ++i) {
            if ( stringRoot.insert( dictionary[i], i ) == true ) {
                ostringstream buffer;
                buffer << endl << "key " << dictionary[i] << " is already in string tree" << endl;
                throw runtime_error(buffer.str());
            }
        }

        clock_gettime(CLOCK_REALTIME, &endTime);
        createStringTime += (endTime.tv_sec - startTime.tv_sec) +
        1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

        // Verify that the correct number of nodes were added to the map.
        stringMapSize = stringRoot.size();
        if (stringMapSize != dictionary.size()) {
            ostringstream buffer;
            buffer << endl << "expected size for string tree = " << stringMapSize
                   << " differs from actual size = " << dictionary.size() << endl;
            throw runtime_error(buffer.str());
        }

        // Search the AVL map for each key and value.
        clock_gettime(CLOCK_REALTIME, &startTime);
        for (size_t i = 0; i < dictionary.size(); ++i) {
            if ( stringRoot.contains( dictionary[i] ) == false ) {
                ostringstream buffer;
                buffer << endl << "key " << dictionary[i] << " is not in string tree for contains" << endl;
                throw runtime_error(buffer.str());
            }
            uint32_t const* val = stringRoot.find( dictionary[i] );
            if (val == nullptr) {
                ostringstream buffer;
                buffer << endl << "key " << dictionary[i] << " is not in string tree for find" << endl;
                throw runtime_error(buffer.str());
            } else if (*val != i) {
                ostringstream buffer;
                buffer << endl << "wrong value = " << (*val) << " for string key "
                       << dictionary[i] << " expected value = " << i << endl;
                throw runtime_error(buffer.str());
            }
        }

        clock_gettime(CLOCK_REALTIME, &endTime);
        searchStringTime += (endTime.tv_sec - startTime.tv_sec) +
        1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

        // Shuffle the dictionary and delete each word from the AVL tree.
        random_shuffle(dictionary.begin(), dictionary.end());
        clock_gettime(CLOCK_REALTIME, &startTime);
        for (size_t i = 0; i < dictionary.size(); ++i) {
            if ( stringRoot.erase( dictionary[i] ) == false ) {
                ostringstream buffer;
                buffer << endl << "string key " << dictionary[i] << " is not in tree for erase" << endl;
                throw runtime_error(buffer.str());
            }
        }

        clock_gettime(CLOCK_REALTIME, &endTime);
        deleteStringTime += (endTime.tv_sec - startTime.tv_sec) +
        1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

        // Verify that the tree is empty
        if ( stringRoot.empty() == false ) {
            ostringstream buffer;
            buffer << endl << stringRoot.size() << " nodes remain in string tree following erasure" << endl;
            throw runtime_error(buffer.str());
        }
    }

    // Report the string tree statistics.
    cout << "number of words in string map = " << stringMapSize << endl;
    cout << "create string time = " << setprecision(4) << (createStringTime/(double)iterations) << " seconds" << endl;
    cout << "search string time = " << setprecision(4) << (searchStringTime/(double)iterations) << " seconds" << endl;
    cout << "delete string time = " << setprecision(4) << (deleteStringTime/(double)iterations) << " seconds" << endl;
    cout << "string insert LL = " << (stringRoot.lli/iterations) << "\tLR = " << (stringRoot.lri/iterations)
         << "\tRL = " << (stringRoot.rli/iterations) << "\tRR = " << (stringRoot.rri/iterations)
         << "\ttotal = " << ((stringRoot.lli+stringRoot.lri+stringRoot.rli+stringRoot.rri)/iterations) << endl;
    cout << "string erase  LL = " << (stringRoot.lle/iterations) << "\tLR = " << (stringRoot.lre/iterations)
         << "\tRL = " << (stringRoot.rle/iterations) << "\tRR = " << (stringRoot.rre/iterations)
         << "\ttotal = " << ((stringRoot.lle+stringRoot.lre+stringRoot.rle+stringRoot.rre)/iterations) << endl;

    // Obtain statisitics for an AVL map that has an integer key.
    avlMap<uint32_t, uint32_t> integerRoot;
    size_t integerMapSize;
    double createIntegerTime = 0, searchIntegerTime = 0, deleteIntegerTime = 0;
    for (size_t it = 0; it < iterations; ++it) {

        // Shuffle the integers and add each integer to the AVL tree.
        random_shuffle(numbers.begin(), numbers.end());
        clock_gettime(CLOCK_REALTIME, &startTime);
        for (size_t i = 0; i < numbers.size(); i++) {
            if ( integerRoot.insert( numbers[i], i ) == true) {
                ostringstream buffer;
                buffer << endl << "key " << dictionary[i] << " is already in integer tree" << endl;
                throw runtime_error(buffer.str());
            }
        }

        clock_gettime(CLOCK_REALTIME, &endTime);
        createIntegerTime += (endTime.tv_sec - startTime.tv_sec) +
        1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

        // Verify that the correct number of nodes were added to the tree
        integerMapSize = integerRoot.size();
        if (integerMapSize != numbers.size()) {
            ostringstream buffer;
            buffer << endl << "expected size for integer tree = " << integerMapSize
                   << " differs from actual size = " << dictionary.size() << endl;
            throw runtime_error(buffer.str());
        }

        // Search for each integer in the AVL tree.
        clock_gettime(CLOCK_REALTIME, &startTime);
        for (size_t i = 0; i < numbers.size(); i++) {
            if ( integerRoot.contains( numbers[i] ) == false ) {
                ostringstream buffer;
                buffer << endl << "key " << numbers[i] << " is not in integer tree for contains" << endl;
                throw runtime_error(buffer.str());
            }
            uint32_t const* val = integerRoot.find( numbers[i] );
            if (val == nullptr) {
                ostringstream buffer;
                buffer << endl << "key " << numbers[i] << " is not in integer tree for find" << endl;
                throw runtime_error(buffer.str());
            } else if (*val != i) {
                ostringstream buffer;
                buffer << endl << "wrong value = " << (*val) << " for integer key "
                       << dictionary[i] << " expected value = " << i << endl;
                throw runtime_error(buffer.str());
            }
        }

        clock_gettime(CLOCK_REALTIME, &endTime);
        searchIntegerTime += (endTime.tv_sec - startTime.tv_sec) +
        1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

        // Shuffle the integers and delete each integer from the AVL tree.
        random_shuffle(numbers.begin(), numbers.end());
        clock_gettime(CLOCK_REALTIME, &startTime);
        for (size_t i = 0; i < numbers.size(); i++) {
            if ( integerRoot.erase( numbers[i] ) == false ) {
                ostringstream buffer;
                buffer << endl << "integer key " << numbers[i] << " is not in tree for erase" << endl;
                throw runtime_error(buffer.str());
            }
        }

        clock_gettime(CLOCK_REALTIME, &endTime);
        deleteIntegerTime += (endTime.tv_sec - startTime.tv_sec) +
        1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

        // Verify that the tree is empty
        if ( integerRoot.empty() == false ) {
            ostringstream buffer;
            buffer << endl << integerRoot.size() << " nodes remain in integer tree following erasure" << endl;
            throw runtime_error(buffer.str());
        }
    }

    // Report the integer statistics.
    cout << "number of words in integer map = " << integerMapSize << endl;
    cout << "create integer time = " << setprecision(4) << (createIntegerTime/(double)iterations) << " seconds" << endl;
    cout << "search integer time = " << setprecision(4) << (searchIntegerTime/(double)iterations) << " seconds" << endl;
    cout << "delete integer time = " << setprecision(4) << (deleteIntegerTime/(double)iterations) << " seconds" << endl;
    cout << "integer insert LL = " << (integerRoot.lli/iterations) << "\tLR = " << (integerRoot.lri/iterations)
         << "\tRL = " << (integerRoot.rli/iterations) << "\tRR = " << (integerRoot.rri/iterations)
         << "\ttotal = " << ((integerRoot.lli+integerRoot.lri+integerRoot.rli+integerRoot.rri)/iterations) << endl;
    cout << "integer erase  LL = " << (integerRoot.lle/iterations) << "\tLR = " << (integerRoot.lre/iterations)
         << "\tRL = " << (integerRoot.rle/iterations) << "\tRR = " << (integerRoot.rre/iterations)
         << "\ttotal = " << ((integerRoot.lle+integerRoot.lre+integerRoot.rle+integerRoot.rre)/iterations) << endl;

    return 0;
}

#endif // TEST_AVL_MAP

#endif // AVL_MAP_CPP
