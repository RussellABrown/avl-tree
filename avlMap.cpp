/*
 * Copyright (c) 2007, 2016, 2023, Russell A. Brown
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
 * of the bug in the del() procedure and replacement of del()
 * by the removeRight() and removeLeft() methods. The removeRight()
 * method performs the identical operations to del(), whereas
 * the removeRight() method performs the mirror-image operations
 * in an attempt to improve rebalancing efficiency after deletion.
 */

#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <set>
#include <string>
#include <stdexcept>
#include <vector>

using namespace std;

/*
 * The mach_gettime function is an alternate to clock_gettime(CLOCK_REALTIME, &time) for Mach.
 * See http://stackoverflow.com/questions/5167269/clock-gettime-alternative-in-mac-os-x
 */
#ifdef MACH
#include <mach/mach_time.h>

#define MACH_NANO (+1.0E-9)
#define MACH_GIGA UINT64_C(1000000000)

static double mach_timebase = 0.0;
static uint64_t mach_timestart = 0;

struct timespec mach_gettime(void) {
    // be more careful in a multithreaded environement
    if (!mach_timestart) {
        mach_timebase_info_data_t tb = { 0 };
        mach_timebase_info(&tb);
        mach_timebase = tb.numer;
        mach_timebase /= tb.denom;
        mach_timestart = mach_absolute_time();
    }
    struct timespec t;
    double diff = (mach_absolute_time() - mach_timestart) * mach_timebase;
    t.tv_sec = diff * MACH_NANO;
    t.tv_nsec = diff - (t.tv_sec * MACH_GIGA);
    return t;
}
#endif

// an AVL node
template <typename K, typename V>
class node {
private:
    K key;
    V value;
    int bal;
    node *left, *right;
    
public:
    node(K const& x, V const& y) {
        key = x;
        value = y;
        bal = 0;
        left = right = nullptr;
    }
    
    /*
     * This method searches the tree for the existence of a key,
     * and either inserts a new node or updates a value.  The
     * tree is then rebalanced recursively backtracking up the tree.
     *
     * @param p (MODIFIED) - pointer to an AVL node
     * @param x (IN) - the key
     * @param y (IN) - the value
     * @param h (MODIFIED) - indicates whether the height has changed
     *
     * @return true if update, false if insertion
     */
public:
    static bool addNode( node<K, V>*& p, K const& x, V const& y, bool& h ) {

        bool result;
        node<K, V> *p1, *p2;
        
        if ( p == nullptr ) {                   // Node is empty so insert a node
            p = new node<K, V>(x, y);
            h = true;                           // and indicate that the height has changed.
            result = false;
        } else if ( x < p->key ) {              // Search the left branch?
            result = addNode( p->left, x, y, h );
            if ( h ) {                          // Left branch has grown higher.
                switch ( p->bal ) {
                    case 1:                     // Balance has been restored.
                        p->bal = 0;
                        h = false;
                        break;
                    case 0:                     // Tree has become less balanced.
                        p->bal = -1;
                        break;
                    case -1:                    // Tree must be rebalanced.
                        p1 = p->left;
                        if ( p1->bal == -1 ) {  // Single LL rotation
                            p->left = p1->right;
                            p1->right = p;
                            p->bal = 0;
                            p = p1;
                        } else {                // Double LR rotation
                            p2 = p1->right;
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
                    default:
                        throw logic_error("left branch p->bal out of range");
                }
            }
        } else if ( x > p->key ) {              // Search the right branch?
            result = addNode( p->right, x, y, h );
            if ( h ) {                          // Right branch has grown higher.
                switch ( p->bal ) {
                   case -1:                     // Balance has been restored.
                        p->bal = 0;
                        h = false;
                        break;
                    case 0:                     // Tree has become less balanced.
                        p->bal = 1;
                        break;
                    case 1:                     // Tree must be rebalanced.
                        p1 = p->right;
                        if ( p1->bal == 1 ) {   // Single RR rotation
                            p->right = p1->left;
                            p1->left = p;
                            p->bal = 0;
                            p = p1;
                        } else {                // Double RL rotation
                            p2 = p1->left;
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
                    default:
                        throw logic_error("right branch p->bal out of range");
                }
            }
        } else {            // The key x is already in tree, so update its value.
            p->value = y;
            h = false;
            result = true;
        }
        return result;
    }
    
    /*
     * This method rebalances following relocation of the rightmost node
     * of the left subtree.
     *
     * @param p (MODIFIED) - pointer to an AVL node
     * @param h (MODIFIED) - indicates whether the height has changed
     */
private:
    static void balanceLeft( node<K, V>*& p, bool& h ) {
        
        node<K, V> *p1, *p2;
        int b1, b2;
        
        switch ( p->bal ) {
            case -1:                    // Balance has been restored.
                p->bal = 0;
                break;
            case 0:                     // Tree has become less balanced.
                p->bal = 1;
                h = false;
                break;
            case 1:                     // Tree must be rebalanced.
                p1 = p->right;
                b1 = p1->bal;
                if ( b1 >= 0 ) {        // Single RR rotation
                    p->right = p1->left;
                    p1->left = p;
                    if ( b1 == 0 ) {
                        p->bal = 1;
                        p1->bal = -1;
                        h = false;
                    } else {
                        p->bal = 0;
                        p1->bal = 0;
                    }
                    p = p1;
                } else {                // Double RL rotation
                    p2 = p1->left;
                    b2 = p2->bal;
                    p1->left = p2->right;
                    p2->right = p1;
                    p->right = p2->left;
                    p2->left = p;
                    if ( b2 == 1 ) {
                        p->bal = -1;
                    } else {
                        p->bal = 0;
                    }
                    if ( b2 == -1 ) {
                        p1->bal = 1;
                    } else {
                        p1->bal = 0;
                    }
                    p = p2;
                    p2->bal = 0;
                }
                break;
            default:
                throw logic_error("p->bal out of range");
        }
    }
    
    /*
     * This method rebalances following relocation of the leftmost node
     * of the right subtree.
     *
     * @param p (MODIFIED) - pointer to an AVL node
     * @param h (MODIFIED) - indicates whether the height has changed
     */
private:
    static void balanceRight( node<K, V>*& p, bool& h ) {
        
        node<K, V> *p1, *p2;
        int b1, b2;
        
        switch ( p->bal ) {
            case 1:                     // Balance has been restored
                p->bal = 0;
                break;
            case 0:                     // Tree has become less balanced.
                p->bal = -1;
                h = false;
                break;
            case -1:                    // Tree must be rebalanced.
                p1 = p->left;
                b1 = p1->bal;
                if ( b1 <= 0 ) {        // Single LL rotation
                    p->left = p1->right;
                    p1->right = p;
                    if ( b1 == 0 ) {
                        p->bal = -1;
                        p1->bal = 1;
                        h = false;
                    } else {
                        p->bal = 0;
                        p1->bal = 0;
                    }
                    p = p1;
                } else {                // Double LR rotation
                    p2 = p1->right;
                    b2 = p2->bal;
                    p1->right = p2->left;
                    p2->left = p1;
                    p->left = p2->right;
                    p2->right = p;
                    if ( b2 == -1 ) {
                        p->bal = 1;
                    } else {
                        p->bal = 0;
                    }
                    if ( b2 == 1 ) {
                        p1->bal = -1;
                    } else {
                        p1->bal = 0;
                    }
                    p = p2;
                    p2->bal = 0;
                }
                break;
            default:
                throw logic_error("p->bal out of range");
        }
    }
    
    /*
     * This method replaces the node to be deleted with
     * the leftmost node of the right subtree.
     *
     * @param p (MODIFIED) - pointer to an AVL node
     * @param p (MODIFIED) - pointer to another AVL node
     * @param h (MODIFIED) - indicates whether the height has changed
     */
private:
    static void removeLeft( node<K, V>*& p, node<K, V>*& q, bool& h) {
        
        if ( p->left != nullptr ) {     // Find the leftmost node of the right subtree
            removeLeft( p->left, q, h );
            if ( h ) {
                balanceLeft( p, h );    // and rebalance following relocation of that node.
            }
        } else {
            q->key = p->key;            // Copy node p contents to node q,
            q->value = p->value;
            q = p;                      // redefine q as node to be deleted p,
            p = p->right;               // and replace node p with right branch.
            h = true;
        }
    }
    
    /*
     * This method replaces the node to be deleted with
     * the rightmost node of the left subtree.
     *
     * @param p (MODIFIED) - pointer to an AVL node
     * @param p (MODIFIED) - pointer to another AVL node
     * @param h (MODIFIED) - indicates whether the height has changed
     */
private:
    static void removeRight( node<K, V>*& p, node<K, V>*& q, bool& h ) {
        
        if ( p->right != nullptr ) {    // Find the rightmost node of the left subtree
            removeRight( p->right, q, h );
            if ( h ) {
                balanceRight( p, h );   // and rebalance following relocation of that node.
            }
        } else {
            q->key = p->key;	        // Copy node p contents to node q,
            q->value = p->value;
            q = p;                      // redefine q as node to be deleted p,
            p = p->left;	        // and replace node p with left branch.
            h = true;
        }
    }
    
    /*
     * This method removes a node from the tree, then
     * rebalances recursively by backtracking up the tree
     *
     * @param p (MODIFIED) - pointer to an AVL node
     * @param x (IN) - the key
     * @param h (MODIFIED) - indicates whether the height has changed
     *
     * @return true if node existed; otherwise, false;
     */
public:
    static bool removeNode( node<K, V>*& p, K const& x, bool& h ) {

        bool result;
        node<K, V>* q;
        
        if ( p == nullptr ) {                   // Node is empty, so
            h = false;                          // indicate that the height has not changed
            result = false;                     // and return false.
        } else if ( x < p->key ) {              // Search the left branch?
            result = removeNode( p->left, x, h );
            if ( h ) {
                balanceLeft( p, h );
            }
        } else if ( x > p->key ) {              // Search the right branch?
            result = removeNode( p->right, x, h );
            if ( h ) {
                balanceRight( p, h );
            }
        } else {                                // The search key x equals the key for this node,
            q = p;                              // so select this node for removal
            if ( q->right == nullptr ) {        // and if one branch is nullptr
                p = q->left;                    // replace it with the opposite one;
                h = true;
            } else if ( q->left == nullptr ) {  // or if the other branch is nullptr
                p = q->right;                   // replace it with the opposite one;
                h = true;
            } else {
                switch ( q->bal ) {             // otherwise find a node to use for replacement:
                    case 0: case -1:            // the left or neither subtree is deeper, so use
                        removeRight( q->left, q, h );  // the rightmost node of left subtree and
                        if ( h ) {
                            balanceLeft( p, h );       // rebalance following relocation of that node;
                        }
                        break;
                    case 1:                     // the right subtree is deeper, so use
                      removeLeft( q->right, q, h );    // the leftmost node of right subtree and
                        if ( h ) {
                            balanceRight( p, h );      // rebalance following relocation of that node.
                        }
                        break;
                    default:
                        throw logic_error("q->bal out of range");
                }
            }
            delete q;
            result = true;
        }
        return result;
    }
    
    /*
     * This method searches the AVL tree for the existence of a key.
     *
     * @param p (IN) - pointer to an AVL node
     * @param x (IN) - the key
     *
     * @return a pointer to the node if key is found; otherwise nullptr
     */
public:
    static V* findNode( node<K, V>* p, K const& x) {

        while (p != nullptr) {            // Iterate; don't use recursion.
            if ( x < p->key ) {           // Search the left branch?
                p = p->left;
            } else if ( x > p->key ) {    // Search right branch?
                p = p->right;
            } else {                      // The key x is in tree, so return a pointer.
                return &p->value;
            }
        }
        return nullptr;                   // The key x is not in tree, so return nullptr.
    }
    
    /*
     * This method counts every node in the AVL tree.
     *
     * @param p (IN) - pointer to an AVL node
     *
     * @return the number of nodes in the AVL tree
     */
public:
    static size_t countNodes( node<K, V>* p ) {

        size_t count = 0;

        if (p == nullptr) {
            return count;
        }
        if ( p->right != nullptr ) {
            count += countNodes(p->right);
        }
        if ( p->left != nullptr ) {
            count += countNodes(p->left);
        }
        count++;     // Count the node itself.

        return count;
    }

    /*
     * This method counts every byte in the AVL tree.
     *
     * @param p (IN) - pointer to an AVL node
     *
     * @return the number of nodes in the AVL tree
     */
public:
    static size_t countBytesForStringNodes( node<K, V>* p ) {

        size_t count = 0;

        if (p == nullptr) {
            return count;
        }
        if ( p->right != nullptr ) {
            count += countBytesForStringNodes(p->right);
        }
        if ( p->left != nullptr ) {
            count += countBytesForStringNodes(p->left);
        }
        count += sizeof(node) + p->key.size();

        return count;
    }

    /*
     * This method deletes every node in the AVL tree.  If all
     * nodes ins the tree have been deleted via calls to remove,
     * this method will do nothing.
     *
     * @param p (IN) - pointer to an AVL node
     */
public:
    static void deleteTree( node<K, V>* p ) {
        
        if (p == nullptr) {
            return;
        }
        if ( p->right != nullptr ) {
            deleteTree(p->right);
            p->right = nullptr;
        }
        if ( p->left != nullptr ) {
            deleteTree(p->left);
            p->left = nullptr;
        }
        delete p;
    }
};

// A basic test:
// compile for linux via 'g++ -lm -lrt -O3 avlMap.cc'
// compile for mach via 'g++ -lm -O3 -DMACH avlMap.cc'
int main(int argc, char **argv) {
    
    struct timespec startTime, endTime;
    int size = 1000000;

    for (uint32_t i = 1; i < argc; ++i) {
        if ( 0 == strcmp(argv[i], "-s") || 0 == strcmp(argv[i], "--size") ) {
            size = atol(argv[++i]);
            continue;
        }
        if ( 0 == strcmp(argv[i], "-h") || 0 == strcmp(argv[i], "--help") ) {
            fprintf(stderr, "\nCommand-line options are:\n\n");
            fprintf(stderr, "    -s or --size number_of_integers (default 1000000)\n\n");
            exit(1);
        }
        fprintf(stderr, "\nCommand-line options are:\n\n");
        fprintf(stderr, "    -s or --size number_of_integers (default 1000000)\n\n");
        exit(1);
    }
    
    // Create some unique unsigned integers
    std::set<uint32_t> numberSet;
    srand(1);
    for (uint32_t i = 0; i < size; i++) {
        uint32_t number =  (uint32_t)floor( (double)UINT_MAX * (double)rand() / (double)RAND_MAX );
        numberSet.insert(number);
    }

    // Add the integers to a vector
    vector<uint32_t> numbers(numberSet.begin(), numberSet.end());

    // Read the words file into a dictionary    
    vector<string> dictionary;
    char buf[512];
    FILE *f = fopen("words.txt", "r");
    while (fgets(buf, sizeof buf, f)) {
        size_t len = strlen(buf);
        buf[len-1] = '\0';
        dictionary.push_back(string(buf));
    }
    fclose(f);

    // Shuffle the words
    std::random_shuffle(dictionary.begin(), dictionary.end());

    // Create an AVL tree that has a string key
    node<string, uint32_t>* stringRoot = nullptr;
    bool h = false;

#ifdef MACH
    startTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &startTime);
#endif

    for (uint32_t i = 0; i < dictionary.size(); i++) {
        if (node<string, uint32_t>::addNode( stringRoot, dictionary[i], i, h )) {
            fprintf(stderr, "key %s already in string tree\n", dictionary[i].c_str());
        }
    }

#ifdef MACH
    endTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &endTime);
#endif
    double createTime = (endTime.tv_sec - startTime.tv_sec) +
	1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

    // Verify that the correct number of nodes were added to the tree
    size_t treeSize = node<string, uint32_t>::countNodes(stringRoot);
    if (treeSize != dictionary.size()) {
        fprintf(stderr, "expected size for string tree %lu differs from actual size %lu\n",
              treeSize, dictionary.size());
    }
    fprintf(stderr, "number of words = %lu  bytes = %lu  bytes/word = %.2f\n",
            treeSize, node<string, uint32_t>::countBytesForStringNodes(stringRoot),
            (double)node<string, uint32_t>::countBytesForStringNodes(stringRoot)/(double)treeSize);
    fprintf(stderr, "create time = %.4f seconds\n", createTime);

    // Search for each word in the AVL tree
#ifdef MACH
    startTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &startTime);
#endif

    for (uint32_t i = 0; i < dictionary.size(); i++) {
        uint32_t const* val = node<string, uint32_t>::findNode( stringRoot, dictionary[i] );
        if (val == nullptr) {
            fprintf(stderr, "key %s not in string tree for search\n", dictionary[i].c_str());
        } else if (*val != i) {
            fprintf(stderr, "wrong value %d for string key %s\n", *val, dictionary[i].c_str());
        }
    }

#ifdef MACH
    endTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &endTime);
#endif
    double searchTime = (endTime.tv_sec - startTime.tv_sec) +
	1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));
    fprintf(stderr, "search time = %.4f seconds\n", searchTime);

    // Delete each word from the AVL tree
#ifdef MACH
    startTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &startTime);
#endif

    for (uint32_t i = 0; i < dictionary.size(); i++) {
        if (!node<string, uint32_t>::removeNode( stringRoot, dictionary[i], h )) {
            fprintf(stderr, "string key %s not in tree for deletion\n", dictionary[i].c_str());
        }
    }
#ifdef MACH
    endTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &endTime);
#endif
    double deleteTime = (endTime.tv_sec - startTime.tv_sec) +
	1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));
    fprintf(stderr, "delete time = %.4f seconds\n", deleteTime);

    // Verify that the tree is empty
    treeSize = node<string, uint32_t>::countNodes(stringRoot);
    if (treeSize != 0) {
        fprintf(stderr, "%lu nodes remain in string tree following deletion\n", treeSize);
    }

    // Create an AVL tree that has an integer key
    node<uint32_t, uint32_t>* integerRoot = nullptr;
    h = false;

#ifdef MACH
    startTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &startTime);
#endif

    for (size_t i = 0; i < numbers.size(); i++) {
        if (node<uint32_t, uint32_t>::addNode( integerRoot, numbers[i], i, h )) {
          fprintf(stderr, "key %d already in tree\n", numbers[i]);
        }
    }

#ifdef MACH
    endTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &endTime);
#endif
    createTime = (endTime.tv_sec - startTime.tv_sec) +
	1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));

    // Verify that the correct number of nodes were added to the tree
    treeSize = node<uint32_t, uint32_t>::countNodes(integerRoot);
    if (treeSize != numbers.size()) {
        fprintf(stderr, "expected size for integer tree %lu differs from actual size %lu\n",
              treeSize, numbers.size());
    }
    fprintf(stderr, "number of integers = %lu  bytes = %lu  bytes/integer = %.2f\n",
            treeSize, treeSize * sizeof(node<uint32_t, uint32_t>),
            (double)sizeof(node<uint32_t, uint32_t>));
    fprintf(stderr, "create time = %.4f seconds\n", createTime);

    // Search for each word in the AVL tree
#ifdef MACH
    startTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &startTime);
#endif

    for (size_t i = 0; i < numbers.size(); i++) {
        uint32_t const* val = node<uint32_t, uint32_t>::findNode( integerRoot, numbers[i] );
        if (val == nullptr) {
          fprintf(stderr, "key %d not in integer tree for search\n", numbers[i]);
        } else if (*val != i) {
            fprintf(stderr, "wrong value %d for key %d\n", *val, numbers[i]);
        }
    }

#ifdef MACH
    endTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &endTime);
#endif
    searchTime = (endTime.tv_sec - startTime.tv_sec) +
	1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));
    fprintf(stderr, "search time = %.4f seconds\n", searchTime);

    // Delete each word from the AVL tree
#ifdef MACH
    startTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &startTime);
#endif

    for (size_t i = 0; i < numbers.size(); i++) {
        if (!node<uint32_t, uint32_t>::removeNode( integerRoot, numbers[i], h )) {
          fprintf(stderr, "integer key %d not in tree for deletion\n", numbers[i]);
        }
    }

#ifdef MACH
    endTime = mach_gettime();
#else
    clock_gettime(CLOCK_REALTIME, &endTime);
#endif
    deleteTime = (endTime.tv_sec - startTime.tv_sec) +
	1.0e-9 * ((double)(endTime.tv_nsec - startTime.tv_nsec));
    fprintf(stderr, "delete time = %.4f seconds\n", deleteTime);

    // Verify that the tree is empty
    treeSize = node<uint32_t, uint32_t>::countNodes(integerRoot);
    if (treeSize != 0) {
        fprintf(stderr, "%lu nodes remain in string tree following deletion\n", treeSize);
    }

    return 0;
}
