//
// Created by simon on 07/08/20.
//
#include <stdio.h>
#include <stdlib.h>

struct intervalTree {
	struct intervalTree *left;
	struct intervalTree *right;
	struct intervalTree *parent;
	int a, b;
	int sum;
	int data;
	int extra; //additional property with specific objective
	char color; //TODO: change color property with a signed integer sum (red if sum<0, black otherwise) to save space
};
typedef struct intervalTree intervalTree;


void printInorderTrasversal(intervalTree *node);
void printPreorderTrasversal(intervalTree* node);
intervalTree* leftmostNode(intervalTree* node);
intervalTree* rightmostNode(intervalTree* node);
intervalTree* inOrderNextNode(intervalTree* node);
intervalTree* inOrderPreviousNode(intervalTree* node);
void redBlackTreeLeftRotate(intervalTree *x);
void redBlackTreeRightRotate(intervalTree *y);
void insertInterval(int a, int b);
intervalTree* searchInterval(int a, int b);
void redBlackTransplant(intervalTree *u, intervalTree *v);

intervalTree* getParent(intervalTree *node);
intervalTree* getSibling(intervalTree *node);
void removalUtility2(intervalTree* node);
void removalUtility1(intervalTree* node);
void deleteNode(intervalTree *node);

void showTreeStructure(intervalTree* node);

intervalTree *tree; //global variable that store the entire tree structure

// Left Rotation procedure for maintaining the balance property for the trees
void redBlackTreeLeftRotate(intervalTree *x) {
	if (x == NULL || x->right == NULL)
		return ;
	intervalTree* y = x->right;
	intervalTree* p = getParent(x);
	// Since the leaves of a red-black tree are empty, they cannot become internal nodes.
	x->right = y->left;
	y->left = x;
	x->parent = y;
	// Handle other child/parent pointers.
	if (x->right != NULL) {
		x->right->parent = x;
	}

	// Initially n could be the root.
	if (p != NULL) {
		if (x == p->left) {
			p->left = y;
		} else if (x == p->right) {
			p->right = y;
		}
	}
	y->parent = p;
}

// Right Rotation (similar to leftRotate)
void redBlackTreeRightRotate(intervalTree *y) {
	if (y == NULL || y->left == NULL)
		return ;
	intervalTree *x = y->left; //x must be not null
	intervalTree *p = getParent(y);
	y->left = x->right;
	x->right = y;
	y->parent = x;

	if (y->left != NULL) {
		y->left->parent = y;
	}

	// Initially n could be the root.
	if (p != NULL) {
		if (y == p->left) {
			p->left = x;
		} else if (y == p->right) {
			p->right = x;
		}
	}
	x->parent = p;
}

void redBlackTreeInsertionFixup(intervalTree* node) {
	if (getParent(node) == NULL) {
		node->color = 'B';
	} else if (getParent(node)->color == 'B') {
		return;
	} else if (getSibling(getParent(node)) != NULL && getSibling(getParent(node))->color == 'R') {
		getParent(node)->color = 'B';
		(getSibling(getParent(node)))->color = 'B';
		(getParent(getParent(node)))->color = 'R';
		redBlackTreeInsertionFixup(getParent(getParent(node)));
	} else {
		intervalTree* p = getParent(node);
		intervalTree* g = getParent(getParent(node));

		if (node == p->right && p == g->left) {
			redBlackTreeLeftRotate(p);
			node = node->left;
		} else if (node == p->left && p == g->right) {
			redBlackTreeRightRotate(p);
			node = node->right;
		}

		p = getParent(node);
		g = getParent(getParent(node));

		if (node == p->left) {
			p->color = 'B';
			g->color = 'R';
			redBlackTreeRightRotate(g);
		} else if (node == p->right){
			p->color = 'B';
			g->color = 'R';
			redBlackTreeLeftRotate(g);
		}

	}
}

void insertRecursive(intervalTree *root, intervalTree * n) {  //unused in this file
	// Recursively descend the tree until a leaf is found.
	if (root != NULL) {
		//if (n->b < root->a || (n->a < root->a && n->b > root->b)) {
		if (n->data < root->data) {
			if (root->left != NULL) {
				insertRecursive(root->left, n);
				return;
			} else {
				root->left = n;
			}
		} else { // n->key >= root->key
			if (root->right != NULL) {
				insertRecursive(root->right, n);
				return;
			} else {
				root->right = n;
			}
		}
	}

	// Insert new Node n.
	n->parent = root;
	n->color = 'R';
	n->right = NULL;
	n->left = NULL;
}
// Utility function to insertInterval newly node in RedBlack tree
void insertInterval(int a, int b) {
	// Allocate memory for new node
	intervalTree *newNode = (intervalTree*) malloc(sizeof(intervalTree));
	newNode->left = NULL;
	newNode->right = NULL;
	newNode->parent = NULL;
	newNode->a = a;
	newNode->b = b;
	newNode->data = (a + b)/2;
	newNode->sum = b - a + 1;

	intervalTree *y = NULL;
	intervalTree *x = tree;

	//follows standard BST procedure to insert the node in the right place
	while (x != NULL) {
		y = x;
		if (newNode->b < x->a || (newNode->a < x->a && newNode->b > x->b)) {
			x = x->left;
		} else {
			x = x->right;
		}
	}

	newNode->parent = y;
	if (y == NULL) {
		tree = newNode;
	} else if (newNode->b < y->a || (newNode->a < y->a && newNode->b > y->b)) {
		y->left = newNode;
	} else {
		y->right = newNode;
	}

	newNode->color = 'R';
	// call insertFixUp to fix reb-black tree's property if it is violated due to insertion.
	redBlackTreeInsertionFixup(newNode);
	// Find the new root
	intervalTree* newRoot = newNode;
	while (getParent(newRoot) != NULL) {
		newRoot = getParent(newRoot);
	}
	tree = newRoot;
}

//displays the tree structure with the form: node value, left subtree, right subtree, for every node in the tree
void showTreeStructure(intervalTree* node) {
	if (node != NULL) {
		printf("node<%d,%d>: ", node->a, node->b);
		if (node->left != NULL) {
			printf("sx:<%d,%d> ", node->left->a, node->left->b);
		} else {
			printf("sx:NULL ");
		}
		if (node->right != NULL) {
			printf("dx:<%d,%d> ", node->right->a, node->right->b);
		} else {
			printf("dx:NULL ");
		}
		printf("\n");
	} else {
		return;
	}
	showTreeStructure(node->left);
	showTreeStructure(node->right);
}

// A utility function to traverse Red-Black tree in in-order fashion (from the minimum value to the max value)
void printInorderTrasversal(intervalTree *node) {
	if (node == NULL) return;

	printInorderTrasversal(node->left);
	printf("<%d,%d,%d> - ", node->a, node->b, node->sum);
	printInorderTrasversal(node->right);
}
// A utility function to traverse Red-Black tree in pre-order fashion (parent --> left child --> right child)
void printPreorderTrasversal(intervalTree* node) {
	if (node == NULL) return;

	printf("<%d,%d,%d> - ", node->a, node->b, node->sum); //prints data of the node
	printPreorderTrasversal(node->left); // then recur on left subtree
	printPreorderTrasversal(node->right); // now recur on right subtree
}

//finds the successor node of the input node, following the crescent order
intervalTree* inOrderNextNode(intervalTree* node) {
	if (node->right != NULL) {
		return leftmostNode(node->right);
	}
	intervalTree* p = node->parent;
	while (p != NULL && node == p->right) {
		node = p;
		p = p->parent;
	}
	return p;
}
//finds the antecedent node of the input node, following the crescent order
intervalTree* inOrderPreviousNode(intervalTree* node) {
	if (node->left != NULL) {
		return rightmostNode(node->left);
	}
	intervalTree *p = node->parent;
	while (p != NULL && node == p->left) {
		node = p;
		p = p->parent;
	}
	return p;
}

//given a non-empty binary search tree, returns the minimum data value found in that tree
intervalTree* leftmostNode(intervalTree* node) {
	intervalTree* current = node;
	// loop down to find the leftmost leaf
	while (current->left != NULL) {
		current = current->left;
	}
	return current;
}
//given a non-empty binary search tree, returns the maximum data value found in that tree
intervalTree* rightmostNode(intervalTree* node) {
	intervalTree* current = node;
	// loop down to find the rightmost leaf
	while (current->right != NULL) {
		current = current->right;
	}
	return current;
}


//TODO: implement adjust sums procedure from current inserted node, and going right til the right-most leaf

int main() { //this implementation uses global variable tree to access its values
	printf("size of intervalTree is %d bytes\n", (int) sizeof(intervalTree));


	insertInterval(5, 6);
	insertInterval(10, 12);
	insertInterval(45, 46);
	insertInterval(44, 48);
	insertInterval(43, 49);
	insertInterval(15, 18);
	insertInterval(13, 20);
	insertInterval(23, 25);
	insertInterval(42, 52);
	insertInterval(53, 54);
	insertInterval(35, 38);
	insertInterval(14, 19);
	insertInterval(62, 65);
	insertInterval(66, 67);
	insertInterval(1, 3);
	insertInterval(40, 55);
	insertInterval(56, 58);
	insertInterval(26, 29);
	insertInterval(21, 60);
	insertInterval(8, 9);
	insertInterval(70,80);
	insertInterval(68, 69);


	printf("in-order Traversal:\n");
	printInorderTrasversal(tree);
	printf("\n");

	//TODO: delete node is bugged and doesn't work

	showTreeStructure(tree);

	return 0;
}

//returns the struct pointer to the interval containing the same interval numbers
intervalTree* searchInterval(int a, int b) {
	intervalTree *x = tree;
	while (x != NULL && x->a != a && x->b != b) {
		//TODO: bug correction: make sure the correct interval is always found, otherwise return null
		if (a < x->a) {
			x = x->left;
		} else {
			x = x->right;
		}
	}
	return x;
}

// replace node u with node v
void redBlackTransplant(intervalTree *u, intervalTree *v){
	v->parent = u->parent;
	if(u->parent == NULL){
		tree = v;
	} else if(u == u->parent->left){
		u->parent->left = v;
	} else {
		u->parent->right = v;
	}
}

//delete doesn't work
void deleteNode(intervalTree *node) {
	intervalTree *child = NULL;
	if (node->right != NULL) {
		child = node->right;
	} else if (node->left != NULL) {
		child = node->left;
	} else { //leaf removal
		if (node->parent->left == NULL) {
			node->parent->right = NULL;
		} else {
			node->parent->left = NULL;
		}
		free(node);
		return;
	}

	redBlackTransplant(node, child);
	if (node->color == 'B') {
		if (child->color == 'R') {
			child->color = 'B';
		} else {
			removalUtility1(child);
		}
	}
	free(node); //frees the memory for the selected node in a secure way, after fixing the tree structure
}

//these utilities serve for fixing the red-black tree structure and colors
void removalUtility1(intervalTree* node) {
	if (node->parent != NULL) {
		intervalTree* stepBro = getSibling(node);

		if (stepBro->color == 'R') {
			node->parent->color = 'R';
			stepBro->color = 'B';
			if (node == node->parent->left) {
				redBlackTreeLeftRotate(node->parent);
			} else {
				redBlackTreeRightRotate(node->parent);
			}
		}
		removalUtility2(node);
	}
}

void removalUtility2(intervalTree* node) {
	intervalTree* stepSis = getSibling(node);

	if ((node->parent->color == 'B') && (stepSis->color == 'B') &&
		(stepSis->left->color == 'B') && (stepSis->right->color == 'B')) {
		stepSis->color = 'R';
		removalUtility1(node->parent);
	} else {
		//intervalTree* stepSis = getSibling(node);

		if ((node->parent->color == 'R') && (stepSis->color == 'B') &&
			(stepSis->left->color == 'B') && (stepSis->right->color == 'B')) {
			stepSis->color = 'R';
			node->parent->color = 'B';
		} else {
			//intervalTree* s = getSibling(node);

			// This if statement is trivial, due to case 2 (even though case 2 changed
			// the sibling to a sibling's child, the sibling's child can't be red, since
			// no red parent can have a red child).
			if (stepSis->color == 'B') {
				// The following statements just force the red to be on the left of the
				// left of the parent, or right of the right, so case six will rotate
				// correctly.
				if ((node == node->parent->left) && (stepSis->right->color == 'B') &&
					(stepSis->left->color == 'R')) {
					// This last test is trivial too due to cases 2-4.
					stepSis->color = 'R';
					stepSis->left->color = 'B';
					redBlackTreeRightRotate(stepSis);
				} else if ((node == node->parent->right) && (stepSis->left->color == 'B') &&
						   (stepSis->right->color == 'R')) {
					// This last test is trivial too due to cases 2-4.
					stepSis->color = 'R';
					stepSis->right->color = 'B';
					redBlackTreeLeftRotate(stepSis);
				}
			}
			stepSis = getSibling(node);

			stepSis->color = node->parent->color;
			node->parent->color = 'B';

			if (node == node->parent->left) {
				stepSis->right->color = 'B';
				redBlackTreeLeftRotate(node->parent);
			} else {
				stepSis->left->color = 'B';
				redBlackTreeRightRotate(node->parent);
			}
		}
	}
}

intervalTree* getSibling(intervalTree *node) {
	intervalTree* parent = getParent(node);
	// No parent means no sibling.
	if (parent == NULL) {
		return NULL;
	}
	if (node == parent->left) {
		return parent->right;
	} else {
		return parent->left;
	}
}

intervalTree* getParent(intervalTree *node) {
	// Note that parent is set to null for the root node.
	if (node == NULL) {
		return NULL;
	} else {
		return node->parent;
	}
}
