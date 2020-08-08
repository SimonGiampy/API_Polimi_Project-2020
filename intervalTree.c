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
	char color; //TODO: change color property with a signed integer sum (red if sum<0, black otherwise) to save space
};
typedef struct intervalTree intervalTree;


void printInorderTrasversal(intervalTree *node);
void printPreorderTrasversal(intervalTree* node);
intervalTree* leftmostNode(intervalTree* node);
intervalTree* rightmostNode(intervalTree* node);
intervalTree* inOrderNextNode(intervalTree* node);
intervalTree* inOrderPreviousNode(intervalTree* node);
void repairTree(intervalTree *z);
void leftRotate(intervalTree *x);
void rightRotate(intervalTree *y);
void insertInterval(int a, int b);
intervalTree* searchInterval(int a, int b);
void redBlackTransplant(intervalTree *u, intervalTree *v);

//bugged function of delete interval
//TODO: safe removal of these functions
void redBlackDeleteFixup(intervalTree *x);
void deleteInterval(intervalTree *z);

intervalTree* getParent(intervalTree *node);
intervalTree* getSibling(intervalTree *node);
void removalUtility2(intervalTree* node);
void removalUtility1(intervalTree* node);
void deleteNode(intervalTree *node);

intervalTree *tree; //global variable that store the entire tree structure

// Left Rotation procedure for maintaining the balance property for the trees
void leftRotate(intervalTree *x) {
	if (x == NULL || x->right == NULL)
		return ;
	//y stored pointer of right child of x
	intervalTree *y = x->right;
	x->right = y->left; //store y's left subtree's pointer as x's right child
	if (x->right != NULL) {
		x->right->parent = x; //update parent pointer of x's right
	}
	y->parent = x->parent; //update y's parent pointer
	if (x->parent == NULL) {
		(*tree) = *y; // if x's parent is null make y as root of tree
	} else if (x == x->parent->left) {
		x->parent->left = y; // store y at the place of x
	} else {
		x->parent->right = y;
	}

	y->left = x; // make x as left child of y
	x->parent = y; //update parent pointer of x
}

// Right Rotation (similar to leftRotate)
void rightRotate(intervalTree *y) {
	if (y == NULL || y->left != NULL)
		return ;
	intervalTree *x = y->left;
	y->left = x->right;

	if (x->right != NULL) {
		x->right->parent = y;
	}
	x->parent =y->parent;
	if (x->parent == NULL) {
		(*tree) = *x;
	} else if (y == y->parent->left) {
		y->parent->left = x;
	} else  {
		y->parent->right = x;
	}
	x->right = y;
	y->parent = x;
}

// Utility function to fixup the Red-Black tree after standard BST insertion
void repairTree(intervalTree *z) {
	// iterate until z is not the root and z's parent color is red
	while (z != tree && z != tree->left && z != tree->right && z->parent->color == 'R') {
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
				rightRotate(z->parent->parent);
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
					leftRotate(z->parent);
					rightRotate(z->parent->parent);
				} else if (z->parent == z->parent->parent->right && z == z->parent->right) {
					// Right-Right (RR) case, do following
					// (i)  Swap color of parent and grandparent
					// (ii) Left Rotate Grandparent
					char ch = z->parent->color ;
					z->parent->color = z->parent->parent->color;
					z->parent->parent->color = ch;
					leftRotate(z->parent->parent);
				} else if (z->parent == z->parent->parent->right && z == z->parent->left) {
					// Right-Left (RL) case, do following
					// (i)  Swap color of current node  and grandparent
					// (ii) Right Rotate Parent
					// (iii) Left Rotate Grand Parent
					char ch = z->color ;
					z->color = z->parent->parent->color;
					z->parent->parent->color = ch;
					rightRotate(z->parent);
					leftRotate(z->parent->parent);
				}
			}

		}
	}
	tree->color = 'B'; //keep root always black
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
	intervalTree** root = &tree;
	//TODO: update sum calculation mechanism

	if (*root == NULL) { //if root is null make newNode as root
		newNode->color = 'B';
		(*root) = newNode;
		newNode->sum = b - a + 1;
	} else {
		intervalTree *y = NULL;
		intervalTree *x = (*root);

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
		if (newNode->b < y->a || (newNode->a < y->a && newNode->b > y->b)) {
			y->left = newNode;
		} else {
			y->right = newNode;
		}

		newNode->color = 'R';
		// call insertFixUp to fix reb-black tree's property if it is violated due to insertion.
		repairTree(newNode);
	}
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

//function used for debugging nodes content
void printExtra(intervalTree *node) {
	intervalTree *lnode = leftmostNode(node);
	printf("leftmost node is: <%d,%d,%d> \n", lnode->a, lnode->b, lnode->sum);
	//lnode = inOrderNextNode(lnode);
	//printf("its next node is <%d,%d,%d> \n", lnode->a, lnode->b, lnode->sum);
	lnode = inOrderPreviousNode(lnode);
	if (lnode == NULL) {
		printf("null node\n");
	}
	printf("its prev node is <%d,%d,%d> \n", lnode->a, lnode->b, lnode->sum);
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


	insertInterval(5, 8);
	insertInterval(43, 50);
	insertInterval(10, 12);
	insertInterval(15, 18);
	insertInterval(13, 20);
	insertInterval(23, 25);
	insertInterval(21, 40);
	insertInterval(35, 38);
	insertInterval(14, 19);
	insertInterval(1, 3);
	insertInterval(26, 29);


	//printf("pre-order Traversal:\n");
	//printPreorderTrasversal(tree);
	//printf("\n");

	printf("in-order Traversal:\n");
	printInorderTrasversal(tree);
	printf("\n");

	//printExtra(tree);
	intervalTree *found;
	found = searchInterval(13, 20);
	if (found != NULL) {
		deleteNode(found);
	}
	found = searchInterval(43, 50);
	if (found != NULL) {
		deleteNode(found);
	}
	found = searchInterval(1, 3);
	printf("found node is: <%d,%d,%d> \n", found->a, found->b, found->sum);
	if (found != NULL) {
		deleteNode(found);
	}
	found = searchInterval(21, 40);
	printf("found node is: <%d,%d,%d> \n", found->a, found->b, found->sum);
	if (found != NULL) {
		deleteNode(found);
	}


	printf("in-order Traversal:\n");
	printInorderTrasversal(tree);
	printf("\n");

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
/*
 * Deletion is done by the same mechanism as BST deletion. If z has no child, z is
 * removed. If z has single child, z is replaced by its child. Else z is replaced by
 * its successor. If successor is not z's own child, successor is replaced by its
 * own child first. then z is replaced by the successor.
 *
 * A pointer y is used to keep track. In first two case y is z. 3rd case y is z's
 * successor. So in first two case y is removed. In 3rd case y is moved.
 *
 *Another pointer x is used to keep track of the node which replace y.
 *
 * As removing or moving y can harm red-black tree properties a variable
 * yOriginalColor is used to keep track of the original colour. If it's BLACK then
 * removing or moving y harm red-black tree properties. In that case an auxiliary
 * procedure redBlackDeleteFixup(x) is called to recover this.
 */

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
				leftRotate(node->parent);
			} else {
				rightRotate(node->parent);
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
					rightRotate(stepSis);
				} else if ((node == node->parent->right) && (stepSis->left->color == 'B') &&
						   (stepSis->right->color == 'R')) {
					// This last test is trivial too due to cases 2-4.
					stepSis->color = 'R';
					stepSis->right->color = 'B';
					leftRotate(stepSis);
				}
			}
			stepSis = getSibling(node);

			stepSis->color = node->parent->color;
			node->parent->color = 'B';

			if (node == node->parent->left) {
				stepSis->right->color = 'B';
				leftRotate(node->parent);
			} else {
				stepSis->left->color = 'B';
				rightRotate(node->parent);
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
/*
void deleteInterval(intervalTree *z){
	intervalTree *y, *x;
	int yOriginalColor;

	y = z;
	yOriginalColor = y->color;

	if (z->left == NULL) {
		x = z->right;
		redBlackTransplant(z, z->right);
	} else if (z->right == NULL) {
		x = z->left;
		redBlackTransplant(z, z->left);
	} else {
		y = leftmostNode(z->right);
		yOriginalColor = y->color;

		x = y->right;

		if (y->parent == z) {
			x->parent = y;
		} else {
			redBlackTransplant(y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}

		redBlackTransplant(z, y);
		y->left = z->left;
		y->left->parent = y;
		y->color = z->color;
	}

	if(yOriginalColor == 'B'){
		redBlackDeleteFixup(x);
	}
}
*/
/*
 * As y was black and removed x gains y's extra blackness.
 * Move the extra blackness of x until
 *		1. x becomes root. In that case just remove extra blackness
 *		2. x becomes a RED and BLACK node. in that case just make x BLACK
 *
 * First check if x is x's parents left or right child. Say x is left child
 *
 * There are 4 cases.
 *
 * Case 1: x's sibling w is red. transform case 1 into case 2 by recoloring
 * w and x's parent. Then left rotate x's parent.
 *
 * Case 2: x's sibling w is black, w's both children is black. Move x and w's
 * blackness to x's parent by coloring w to RED and x's parent to BLACK.
 * Make x's parent new x.Notice if case 2 come through case 1 x's parent becomes
 * RED and BLACK as it became RED in case 1. So loop will stop in next iteration.
 *
 * Case 3: w is black, w's left child is red and right child is black. Transform
 * case 3 into case 4 by recoloring w and w's left child, then right rotate w.
 *
 * Case 4: w is black, w's right child is red. recolor w with x's parent's color.
 * make x's parent BLACK, w's right child black. Now left rotate x's parent. Make x
 * point to root. So loop will be stopped in next iteration.
 *
 * If x is right child of it's parent do exact same thing swapping left<->right
 */

void redBlackDeleteFixup(intervalTree *x){
	intervalTree *w;

	while (x != tree && x->color == 'B') {

		if (x == x->parent->left){
			w = x->parent->right;

			if (w->color == 'R') {
				w->color = 'B';
				x->parent->color = 'R';
				leftRotate(x->parent);
				w = x->parent->right;
			}

			if (w->left->color == 'B' && w->right->color == 'B') {
				w->color = 'R';
				x->parent->color = 'B';
				x = x->parent;
			} else {

				if(w->right->color == 'B') {
					w->color = 'R';
					w->left->color = 'B';
					rightRotate(w);
					w = x->parent->right;
				}

				w->color = x->parent->color;
				x->parent->color = 'B';
				x->right->color = 'B';
				leftRotate(x->parent);
				x = tree;

			}

		} else {
			w = x->parent->left;

			if (w->color == 'R') {
				w->color = 'B';
				x->parent->color = 'B';
				rightRotate(x->parent);
				w = x->parent->left;
			}

			if (w->left->color == 'B' && w->right->color == 'B') {
				w->color = 'R';
				x->parent->color = 'B';
				x = x->parent;
			} else {

				if (w->left->color == 'B') {
					w->color = 'R';
					w->right->color = 'B';
					leftRotate(w);
					w = x->parent->left;
				}

				w->color = x->parent->color;
				x->parent->color = 'B';
				w->left->color = 'B';
				rightRotate(x->parent);
				x = tree;

			}
		}

	}

	x->color = 'B';
}

