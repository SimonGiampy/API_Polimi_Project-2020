//
// Created by simon on 07/08/20.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


struct intervalTree {
	struct intervalTree *left;
	struct intervalTree *right;
	struct intervalTree *parent;
	int a, b;
	int sum;
	int data;
	char color;
};
typedef struct intervalTree intervalTree;


void printInorderTrasversal(intervalTree *node);
intervalTree* leftmostNode(intervalTree* node);
intervalTree* inOrderNextNode(intervalTree* root, intervalTree* node);
void repairTree(intervalTree **root, intervalTree *z);
void leftRotate(intervalTree **root, intervalTree *x);
void rightRotate(intervalTree **root,intervalTree *y);
void insertInterval(intervalTree **root, int a, int b);

//intervalTree *tree; //global variable that store the entire tree structure

// Left Rotation
void leftRotate(intervalTree **root, intervalTree *x) {
	if (x == NULL || !x->right)
		return ;
	//y stored pointer of right child of x
	intervalTree *y = x->right;

	//store y's left subtree's pointer as x's right child
	x->right = y->left;

	//update parent pointer of x's right
	if (x->right != NULL) {
		x->right->parent = x;
	}
	//update y's parent pointer
	y->parent = x->parent;

	// if x's parent is null make y as root of tree
	if (x->parent == NULL) {
		(*root) = y;
		// store y at the place of x
	} else if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}

	// make x as left child of y
	y->left = x;

	//update parent pointer of x
	x->parent = y;
}

// Right Rotation (Similar to leftRotate)
void rightRotate(intervalTree **root, intervalTree *y) {
	if (y == NULL || !y->left)
		return ;
	intervalTree *x = y->left;
	y->left = x->right;

	if (x->right != NULL) {
		x->right->parent = y;
	}
	x->parent =y->parent;
	if (x->parent == NULL) {
		(*root) = x;
	} else if (y == y->parent->left) {
		y->parent->left = x;
	} else  {
		y->parent->right = x;
	}
	x->right = y;
	y->parent = x;
}

// Utility function to fixup the Red-Black tree after standard BST insertion
void repairTree(intervalTree **root, intervalTree *z) {
	// iterate until z is not the root and z's parent color is red
	while (z != *root && z != (*root)->left && z != (*root)->right && z->parent->color == 'R') {
		intervalTree *y;

		// Find uncle and store uncle in y
		if (z->parent != NULL && z->parent->parent != NULL && z->parent == z->parent->parent->left) {
			y = z->parent->parent->right;
		} else {
			y = z->parent->parent->left;
		}
		// If uncle is RED, do following
		// (i)  Change color of parent and uncle as BLACK
		// (ii) Change color of grandparent as RED
		// (iii) Move z to grandparent
		if (y == NULL) {
			z = z->parent->parent;
		} else if (y->color == 'R') {
			y->color = 'B';
			z->parent->color = 'B';
			z->parent->parent->color = 'R';
			z = z->parent->parent;
		} else {
			// Uncle is BLACK, there are four cases (LL, LR, RL and RR)
			// Left-Left (LL) case, do following
			// (i)  Swap color of parent and grandparent
			// (ii) Right Rotate Grandparent
			if (z->parent == z->parent->parent->left && z == z->parent->left) {
				char ch = z->parent->color ;
				z->parent->color = z->parent->parent->color;
				z->parent->parent->color = ch;
				rightRotate(root,z->parent->parent);
			}

			if (z->parent != NULL && z->parent->parent != NULL ) {
				if (z->parent == z->parent->parent->left && z == z->parent->right) {
					// Left-Right (LR) case, do following
					// (i)  Swap color of current node  and grandparent
					// (ii) Left Rotate Parent
					// (iii) Right Rotate Grand Parent
					char ch = z->color;
					z->color = z->parent->parent->color;
					z->parent->parent->color = ch;
					leftRotate(root, z->parent);
					rightRotate(root, z->parent->parent);
				} else if (z->parent == z->parent->parent->right && z == z->parent->right) {
					// Right-Right (RR) case, do following
					// (i)  Swap color of parent and grandparent
					// (ii) Left Rotate Grandparent
					char ch = z->parent->color ;
					z->parent->color = z->parent->parent->color;
					z->parent->parent->color = ch;
					leftRotate(root, z->parent->parent);
				} else if (z->parent == z->parent->parent->right && z == z->parent->left) {
					// Right-Left (RL) case, do following
					// (i)  Swap color of current node  and grandparent
					// (ii) Right Rotate Parent
					// (iii) Left Rotate Grand Parent
					char ch = z->color ;
					z->color = z->parent->parent->color;
					z->parent->parent->color = ch;
					rightRotate(root,z->parent);
					leftRotate(root, z->parent->parent);
				}
			}

		}
	}
	(*root)->color = 'B'; //keep root always black
}

// Utility function to insertInterval newly node in RedBlack tree
void insertInterval(intervalTree **root, int a, int b) {
	int data = (a + b) / 2 ; //approx by defect
	// Allocate memory for new node
	intervalTree *newNode = (intervalTree*) malloc(sizeof(intervalTree));
	newNode->data = data;
	newNode->left = NULL;
	newNode->right = NULL;
	newNode->parent = NULL;
	newNode->a = a;
	newNode->b = b;
	//intervalTree** root = &tree;

	//if root is null make newNode as root
	if (*root == NULL) {
		newNode->color = 'B';
		(*root) = newNode;
	} else {
		intervalTree *y = NULL;
		intervalTree *x = (*root);

		//follows standard BST procedure to insert the node in the right place
		while (x != NULL) {
			y = x;
			if (newNode->data < x->data) {
				x = x->left;
			} else if (newNode->data >= x->data) {
				x = x->right;
			}
		}

		newNode->parent = y;
		if (newNode->data > y->data) {
			y->right = newNode;
		} else {
			y->left = newNode;
		}
		newNode->color = 'R';

		// call insertFixUp to fix reb-black tree's property if it is violated due to insertion.
		repairTree(root, newNode);
	}
}

// A utility function to traverse Red-Black tree in in-order fashion
void printInorderTrasversal(intervalTree *root) {
	if (root == NULL) return;

	printInorderTrasversal(root->left);
	printf("%d ", root->data);

	printInorderTrasversal(root->right);
}

void printPreorderTrasversal(intervalTree* node)
{
	if (node == NULL) return;

	// first print data of node
	printf("%d ", node->data);
	// then recur on left subtree
	printPreorderTrasversal(node->left);
	// now recur on right subtree
	printPreorderTrasversal(node->right);
}


intervalTree* inOrderNextNode(intervalTree* root, intervalTree* node) {
	// step 1 of the above algorithm
	if (node->right != NULL)
		return leftmostNode(node->right);

	// step 2 of the above algorithm
	intervalTree* p = node->parent;
	while (p != NULL && node == p->right) {
		node = p;
		p = p->parent;
	}
	return p;
}

// Given a non-empty binary search tree, return the minimum data value found in that tree.
intervalTree* leftmostNode(intervalTree* node) {
	intervalTree* current = node;

	// loop down to find the leftmost leaf
	while (current->left != NULL) {
		current = current->left;
	}
	return current;
}


int main() { //this implementation uses the address of the tree to pass as parameter for insertion
	srandom(time(0));
	intervalTree *root = NULL;
	//random insertion of 10 elements from 0 to 100
	int numbers[100];
	for (int i = 0; i < 100; ++i) {
		numbers[i] = (int) random() % (1000 + 1);
	}

	clock_t t0 = clock();
	//random insertion of 10 elements from 0 to 100
	for (int i = 0; i < 100; ++i) {
		insertInterval(&root, numbers[i], numbers[i] + 15);
	}
	/*
	insertInterval(5, 8);
	insertInterval(10, 12);
	insertInterval(15, 18);
	*/
	printf("size of struct node is %d\n", (int) sizeof(intervalTree));

	clock_t t1 = clock();
	printf("in-order Traversal Is :\n");
	printInorderTrasversal(root);
	printf("\n");
	float time_taken = (float)(t1 - t0) / CLOCKS_PER_SEC * 1000;
	printf("insertion took %fms\n", time_taken);

	return 0;
}

