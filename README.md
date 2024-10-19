This C++ implementation of an AVL tree achieves improved performance for AVL node deletion via the algorithm proposed by Caxton C. Foster in his 1965 article, "A Study of AVL Trees" (Goodyear Aerospace Corporation internal document GER-12158), and also described in the following pre-print article.

https://arxiv.org/abs/2406.05162

The avlTree.cpp source-code file is an implementation of an AVL tree. The avlMap.cpp source-code file is an implementation of a key-to-value map based on an AVL tree. The words.txt file is a dictionary of words used to test avlMap.cpp.

The C++ code was created by translating Pascal code from Nicklaus Wirth's textbook, "Algorithms + Data Structures = Programs" and then modifying the del procedure to fix a bug and improve performance as described in the above pre-print article.
