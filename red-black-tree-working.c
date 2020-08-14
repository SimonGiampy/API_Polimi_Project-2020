//
// Created by simon on 07/08/20.
// Pointers are like integrals: they require a lot of time and practice in order to be fully mastered
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

struct intervalTree {
	struct intervalTree *left;
	struct intervalTree *right;
	struct intervalTree *parent;
	int a, b; //intervals represent a range from a to b
	int skips; //calculates the number of deleted elements until the right endpoint b included
	//additional properties for knowing where the bigger overlapping interval is, that includes the smaller ones
	int highEndpoint;
	//identifies the corresponding a value of its overlapping bigger interval, which includes the smaller ones
	int lowEndpoint;
	char color; //can be 'R' or 'B' if the node is red or black
};
typedef struct intervalTree intervalTree; //size of struct is 48 bytes, where 3 are wasted
//LEGEND used for storing char inside an integer: red node: skips > 0, black node: skips < 0.

struct quickLook {
	intervalTree *piece;
	struct quickLook *next;
};
struct quickLookupsHead { //stack of type FILO (first in, last out) for keeping track of last accessed intervals
	struct quickLook *head; //pointer to the first element
	struct quickLook *end; //pointer to the last element
	int capacity; //can be set by default to 10
	int occupied;
};
struct quickLookupsHead *quickLookupsHead;



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
void redBlackTransplant(intervalTree *source, intervalTree *substitute);
intervalTree* getParent(intervalTree *node);
intervalTree* getSibling(intervalTree *node);
void redBlackTreeRemovalFixup2(intervalTree* node);
void redBlackTreeRemovalFixup1(intervalTree* node);
void deleteNode(intervalTree *node);
void showTreeStructure(intervalTree* node);
void adjustParametersAfterInsertion(intervalTree* element);
void adjustParametersAfterDeletion(intervalTree* node);

void assignColor(char color, intervalTree* dest);
void copyColor(intervalTree* source, intervalTree* dest);
bool isRed(intervalTree* node);
bool isBlack(intervalTree* node);
int abs(int value);
int lookup(int key);

void addIntervalForQuickLookups(intervalTree* node);
int quickLookup(int key);
void debugQuickLookups();

intervalTree *tree; //global variable that stores the entire tree structure, pointer to the root node


int main() { //this implementation uses global variable tree to access its values
	printf("size of intervalTree is %d bytes\n", (int) sizeof(intervalTree));

	//initialization of quicklookups structure
	quickLookupsHead = (struct quickLookupsHead*) malloc(sizeof(struct quickLookupsHead));
	quickLookupsHead->capacity = 4; //just for testing a limited amount of intervals
	quickLookupsHead->occupied = 0;

	/*
	insertInterval(5, 6);
	insertInterval(10, 12);
	insertInterval(45, 46);
	insertInterval(44, 48);
	insertInterval(43, 49);
	insertInterval(15, 18);
	insertInterval(14, 19);
	insertInterval(13, 20);
	insertInterval(23, 25);
	insertInterval(42, 52);
	insertInterval(53, 54);
	insertInterval(35, 38);
	insertInterval(62, 65);
	insertInterval(66, 67);
	insertInterval(1, 3);
	insertInterval(40, 55);
	insertInterval(56, 58);
	insertInterval(26, 29);
	insertInterval(21, 60);
	insertInterval(8, 9);
	insertInterval(81,84);
	insertInterval(78, 79);
*/

	insertInterval(13, 15);
	insertInterval(20, 23);
	insertInterval(27, 28);
	insertInterval(31, 33);
	insertInterval(73, 74);
	insertInterval(77, 78);
	insertInterval(80, 82);
	insertInterval(36, 39);
	insertInterval(43, 44);
	insertInterval(50, 50);
	insertInterval(55, 57);
	insertInterval(29, 34);
	insertInterval(63, 64);
	insertInterval(62, 67);
	insertInterval(60, 70);
	insertInterval(72, 85);

	printf("in-order Traversal:\n");
	printInorderTrasversal(tree);
	printf("\n");
	//showTreeStructure(tree);
	//TODO: make new function to adjust parameters after deletion of an element and check its correctness

	int key = 24;
	printf("lookup %d = %d\n", key, quickLookup(key));
	debugQuickLookups();
	key = 33;
	printf("lookup %d = %d\n", key, quickLookup(key));
	debugQuickLookups();
	key = 10;
	printf("lookup %d = %d\n", key, quickLookup(key));
	debugQuickLookups();
	key = 60;
	printf("lookup %d = %d\n", key, quickLookup(key));
	debugQuickLookups();
	key = 26;
	printf("lookup %d = %d\n", key, quickLookup(key));
	debugQuickLookups();
	key = 19;
	printf("lookup %d = %d\n", key, quickLookup(key));
	debugQuickLookups();
	key = 9;
	printf("lookup %d = %d\n", key, quickLookup(key));
	debugQuickLookups();

	return 0;
}

//calculates the exact row where a line = key is stored
int lookup(int key) {
	intervalTree *node = tree;
	intervalTree *match = NULL; //save in memory the node which skips value is the closest as possible to the key, but greater than the key

	while (node != NULL) {
		if (key > node->skips) { //goes to the right subtree
			if ((node->skips > key && match == NULL) || (node->skips > key && node->skips < match->skips)) {
				match = node; //match update
			}
			if (node->right == NULL) {
				if (match == NULL) { //the rightmost node in the tree has a different index calculation
					addIntervalForQuickLookups(node);
					printf("node chosen is <%d,%d,%d>\n", node->a, node->b, node->skips);
					return node->highEndpoint + key - node->skips;
				}
				break; //stops the while cycle since it found the correct node
			} else {
				node = node->right; //goes to the right subtree
			}

		} else if (key < node->skips) { //goes to the left subtree
			if ((match == NULL && node->skips > key) || (node->skips > key && node->skips < match->skips)) {
				match = node; //match update
			}
			if (node->left == NULL) {
				break; //stops the while cycle since it found the correct node
			} else {
				node = node->left;
			}
		} else if (key == node->skips) { //found exact key in one of the nodes
			//addIntervalForQuickLookups(node); //this particular case in not necessary since the key is exactly equal
			//and the probability of it being found again is very low
			printf("node chosen is <%d,%d,%d>\n", node->a, node->b, node->skips);
			return node->lowEndpoint - 1;
		}

	}

	if (match != NULL) { //should be match
		//insert debug flag here or delete the printf statement
		printf("node chosen is <%d,%d,%d>\n", match->a, match->b, match->skips);
		addIntervalForQuickLookups(match);
		return match->lowEndpoint - 1 - match->skips + key; //is it correct though?
	}
	return 0; //must be never called
}
//iterates through the linked list of intervals, and tries to find a match for the calculation of the index
int quickLookup(int key) {
	struct quickLook *current = quickLookupsHead->head; //current must be != NULL from the start
	intervalTree *comparison; //used for comparison with a certain interval in the list
	struct quickLook *prev = NULL; //prev must be automatically != NULL when counter > 1
	int counter = 0; //counts the number of nodes which have been examined

	while (current != NULL) {
		comparison = inOrderPreviousNode(current->piece); //needed to check if the interval is the correct one
		counter++;

		if (inOrderNextNode(current->piece) == NULL) {
			//this is the rightmost node
			if (key > current->piece->skips) { //rightmost node is a valid interval
				if (counter > 1 && prev != NULL) {
					//moves current node to the head to make access faster
					prev->next = current->next;
					current->next = quickLookupsHead->head;
					quickLookupsHead->head = current;
				}
				return current->piece->highEndpoint + key - current->piece->skips;
			}
		} else if (comparison == NULL) { //leftmost node found
			if (current->piece->skips >= key) {
				if (counter > 1 && prev != NULL) {
					//moves current node to the head to make access faster
					prev->next = current->next;
					current->next = quickLookupsHead->head;
					quickLookupsHead->head = current;
				}
				return current->piece->lowEndpoint - 1 - current->piece->skips + key;
			}
		} else if (current->piece->skips >= key && comparison->skips < key) { //comparison is != NULL
			//valid interval chosen, which is between 2 intervals
			if (counter > 1 && prev != NULL) {
				//moves current node to the head to make access faster
				prev->next = current->next;
				current->next = quickLookupsHead->head;
				quickLookupsHead->head = current;
			}
			return current->piece->lowEndpoint - 1 - current->piece->skips + key;
		}
		//continues the execution since a valid interval is not found
		prev = current;
		current = current->next;
	}

	return lookup(key); //called only if the necessary node is not stored in the quick lookup structure
}

//this is a quicker method for finding the correct interval for the calculation of the correct indexes
//adds an interval for a quick access, in case the interval is not present already
void addIntervalForQuickLookups(intervalTree* node) {
	struct quickLook *current = (struct quickLook*) malloc(sizeof(struct quickLook));
	current->piece = node;
	if (quickLookupsHead->head == NULL) { //first element inserted and created
		current->next = NULL;
		quickLookupsHead->occupied = 1;
		quickLookupsHead->head = current;
		quickLookupsHead->end = current;
	} else {
		current->next = quickLookupsHead->head;
		quickLookupsHead->head = current;
		if (quickLookupsHead->occupied == quickLookupsHead->capacity ) {
			//insert new node at the head and frees the last element in the linked list
			struct quickLook *tmp = current;
			short counter = 1;
			while (counter < quickLookupsHead->capacity) { //scrolls the list till the element before the last one
				tmp = tmp->next;
				counter++;
			}
			free(tmp->next); //deletes the last element when the occupied size is greater than the capacity
			tmp->next = NULL;
			quickLookupsHead->end = tmp; //adjust the end pointer, by scrolling the list
		} else if (quickLookupsHead->occupied < quickLookupsHead->capacity) {
			quickLookupsHead->occupied += 1; //decrease or increase
		}
	}
}

void debugQuickLookups() {
	struct quickLook *current = quickLookupsHead->head;
	printf("showing the list of quick lookups: ");
	while (current != NULL) {
		printf("<%d,%d,%d> - ", current->piece->a, current->piece->b, current->piece->skips);
		current = current->next;
	}
	printf("\n");
}

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

//this utility function fixes the structure of the tree so that after the insertion, continues to satisfy the rbt property
void redBlackTreeInsertionFixup(intervalTree* node) {
	if (getParent(node) == NULL) {
		assignColor('B', node);
	} else if (isBlack(getParent(node))) {
		return;
	} else if (getSibling(getParent(node)) != NULL && isRed(getSibling(getParent(node)))) {
		assignColor('B', node);
		assignColor('B', getSibling(getParent(node)));
		assignColor('R', getParent(getParent(node)));
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
			assignColor('B', p);
			assignColor('R', g);
			redBlackTreeRightRotate(g);
		} else if (node == p->right){
			assignColor('B', p);
			assignColor('R', g);
			redBlackTreeLeftRotate(g);
		}

	}
}

// Utility function to insert a new node in RedBlack tree, after a delete call
void insertInterval(int a, int b) {
	// Allocates memory for new node (48 bytes)
	intervalTree *newNode = (intervalTree*) malloc(sizeof(intervalTree));
	newNode->left = NULL;
	newNode->right = NULL;
	newNode->parent = NULL;
	newNode->a = a;
	newNode->b = b;
	newNode->skips = a - 1;
	newNode->highEndpoint = b;
	newNode->lowEndpoint = a;

	intervalTree *y = NULL;
	intervalTree *x = tree;

	//follows standard BST procedure to insert the node in the right place
	//bigger overlapping intervals are put before the smaller ones which are included by the bigger one
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

	assignColor('R', newNode);
	// call insertFixUp to fix reb-black tree's property if it is violated due to insertion.
	redBlackTreeInsertionFixup(newNode);
	// Find the new root
	intervalTree* newRoot = newNode;
	while (getParent(newRoot) != NULL) {
		newRoot = getParent(newRoot);
	}
	tree = newRoot;

	adjustParametersAfterInsertion(newNode);
}

//displays the tree structure with the form: node value, left subtree, right subtree, for every node in the tree
void showTreeStructure(intervalTree* node) { //used for debugging the tree structure and check if it's balanced
	if (node != NULL) {
		printf("node<%d,%d,%d>: ", node->a, node->b, node->skips);
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
	//format for the node's description is <a,b,skips,lowEndpoint,highEndpoint>
	printInorderTrasversal(node->left);
	printf("<%d,%d,%d,%d,%d> - ", node->a, node->b, node->skips, node->lowEndpoint, node->highEndpoint);
	printInorderTrasversal(node->right);
}
// A utility function to traverse Red-Black tree in pre-order fashion (parent --> left child --> right child)
void printPreorderTrasversal(intervalTree* node) {
	if (node == NULL) return;
	//format for the node's description is <a,b,skips,lowEndpoint,highEndpoint>
	printf("<%d,%d,%d,%d,%d> - ", node->a, node->b, node->skips, node->lowEndpoint, node->highEndpoint); //prints data of the node
	printPreorderTrasversal(node->left); // then recur on left subtree
	printPreorderTrasversal(node->right); // now recur on right subtree
}

//finds the successor node of the input node, following the crescent order
intervalTree* inOrderNextNode(intervalTree* node) { //finds minimum element in the right subtree
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
intervalTree* inOrderPreviousNode(intervalTree* node) { //finds maximum element in left subtree
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

bool isRed(intervalTree* node) { //returns false if the node is black, true if it's red
	if (node->color == 'R') return true; else return false;
	/*if (node->highEndpoint < 0) {
		return false; //black node
	} else if (node->highEndpoint > 0) {
		return true; //red node
	} else {
		return NULL; //never happens since the skips can't be == 0
	}*/
}
bool isBlack(intervalTree* node) { //returns false if the node is black, true if it's red
	if (node->color == 'B') return true; else return false;
	/*if (node->highEndpoint < 0) {
		return true; //black node
	} else if (node->highEndpoint > 0) {
		return false; //red node
	} else {
		return NULL; //never happens since the skips can't be == 0
	}*/
}
int abs(int value) { //return absolute integer value of an input value
	if (value < 0) {
		return -value;
	} else return value;
}
void copyColor(intervalTree* source, intervalTree* dest) {
	dest->color = source->color;
	/*
	if ((source->highEndpoint > 0 && dest->highEndpoint < 0) || (source->highEndpoint < 0 && dest->highEndpoint > 0)) {
		dest->highEndpoint = -dest->highEndpoint; //change color in the case the two nodes have different colors
	}
	 */
}
void assignColor(char color, intervalTree* dest) {
	dest->color = color;
	/*
	if (color == 'R') {
		if (dest->highEndpoint < 0) {
			dest->highEndpoint = - dest->highEndpoint;
		}
	} else if (color == 'B') {
		if (dest->highEndpoint > 0) {
			dest->highEndpoint = - dest->highEndpoint;
		}
	}*/
}

//modifies the parameters skips and highEndpoint, so the lookup function is faster to execute, since it's the one
//which is called many more times than the delete rows function
void adjustParametersAfterInsertion(intervalTree* element) {
	intervalTree *prev = inOrderPreviousNode(element);
	//this if statement checks for the number of lines actually present, before the interval considered
	if (prev != NULL && prev->highEndpoint < element->a) {
		element->skips = prev->skips + element->a - 1 - prev->highEndpoint;
	}

	intervalTree *current = inOrderNextNode(element);
	prev = element;
	int elementHigh = element->highEndpoint; //keeps track of the high endpoint of the bigger overlapping interval
	int elementLow = element->lowEndpoint; //keeps track of the low endpoint of the bigger overlapping interval

	while (current != NULL) {
		if (current->b < elementHigh && current->a > elementLow) {
			//iteration through the smaller intervals included in a bigger one
			//copies the endpoints to make the lookup function faster, while wasting a bit of memory
			current->highEndpoint = elementHigh;
			current->lowEndpoint = elementLow;
			current->skips = element->skips;
		} else {
			//calculates the new number of skips
			current->skips = prev->skips + current->a - 1 - prev->highEndpoint;
			//updates the new endpoint values based on the intervals that come after
			elementHigh = current->highEndpoint;
			elementLow = current->lowEndpoint;
		}
		prev = current;
		current = inOrderNextNode(current); //goes forward and updates the remaining nodes
	}
}

void adjustParametersAfterDeletion(intervalTree* node) {
	//node should be the node that is situated before the deleted node
	//so it cycles forward the in order list of elements and updates the values
}


//returns the struct pointer to the interval containing the same exact interval numbers
intervalTree* searchInterval(int a, int b) {
	intervalTree *x = tree;
	while (x != NULL && x->a != a && x->b != b) {
		if (a < x->a) {
			x = x->left;
		} else {
			x = x->right;
		}
	}
	return x; //returns null if the interval is not found
}

// replaces node source with the node substitute
void redBlackTransplant(intervalTree *source, intervalTree *substitute){
	substitute->parent = source->parent;
	if(source == source->parent->left){
		source->parent->left = substitute;
	} else {
		source->parent->right = substitute;
	}
}

//deletes a node, given as input the specific node pointer to delete
void deleteNode(intervalTree *node) {
	intervalTree *child = NULL;
	if (node->right == NULL && node->left != NULL) {
		child = node->left; //one child removal, base case
	} else if (node->left == NULL && node->right != NULL) {
		child = node->right; //one child removal, base case
	} else if (node->left == NULL && node->right == NULL) {
		//leaf removal (this is my addition)
		if (node->parent->left == node) {
			node->parent->left = NULL;
		} else {
			node->parent->right = NULL;
		}
		free(node);
		return;
	} else if (node->right != NULL && node->left != NULL) {
		//additional case: removal of a node with 2 children nodes
		intervalTree *y = inOrderNextNode(node);
		char yOriginalColor = y->color;

		intervalTree *x = y->right;
		if (x != NULL) {
			if (y->parent == node) {
				x->parent = y;
			} else {
				redBlackTransplant(y, y->right);
				y->right = node->right;
				y->right->parent = y;
			}
		}

		redBlackTransplant(node, y);
		y->left = node->left;
		y->left->parent = y;
		copyColor(node, y);
		if (yOriginalColor == 'B' && x != NULL){
			redBlackTreeRemovalFixup1(x);
		}
		free(node);
		return;
	}

	redBlackTransplant(node, child);
	if (isBlack(node)) {
		if (isRed(child)) {
			assignColor('B', child);
		} else {
			redBlackTreeRemovalFixup1(child); //fixes the tree structure after the deletion of the node
		}
	}
	free(node); //frees the memory for the selected node in a secure way, after fixing the tree structure
}

//these utilities serve for fixing the red-black tree structure and colors
void redBlackTreeRemovalFixup1(intervalTree* node) {
	if (node->parent != NULL) {
		intervalTree* stepBro = getSibling(node);

		if (isRed(stepBro)) {
			assignColor('R', node->parent);
			assignColor('B', stepBro);
			if (node == node->parent->left) {
				redBlackTreeLeftRotate(node->parent);
			} else {
				redBlackTreeRightRotate(node->parent);
			}
		}
		redBlackTreeRemovalFixup2(node);
	}
}
//second utility function to maintain the re-black tree properties still valid
void redBlackTreeRemovalFixup2(intervalTree* node) {
	intervalTree* stepSis = getSibling(node);

	if (isBlack(node->parent) && isBlack(stepSis) && isBlack(stepSis->left) && isBlack(stepSis->right)) {
		assignColor('R', stepSis);
		redBlackTreeRemovalFixup1(node->parent);
	} else {
		if (isRed(node->parent) && isBlack(stepSis) && isBlack(stepSis->left) && isBlack(stepSis->right)) {
			assignColor('R', stepSis);
			assignColor('B', node->parent);
		} else {
			// This if statement is trivial, due to case 2 (even though case 2 changed
			// the sibling to a sibling's child, the sibling's child can't be red, since
			// no red parent can have a red child).
			if (isBlack(stepSis)) {
				// The following statements just force the red to be on the left of the
				// left of the parent, or right of the right, so case six will rotate
				// correctly.
				if ((node == node->parent->left) && isBlack(stepSis->right) && isRed(stepSis->left)) {
					// This last test is trivial too due to cases 2-4.
					assignColor('R', stepSis);
					assignColor('B', stepSis->left);
					redBlackTreeRightRotate(stepSis);
				} else if ((node == node->parent->right) && isBlack(stepSis->left) && isRed(stepSis->right)) {
					// This last test is trivial too due to cases 2-4.
					assignColor('R', stepSis);
					assignColor('B', stepSis->right);
					redBlackTreeLeftRotate(stepSis);
				}
			}
			stepSis = getSibling(node);

			copyColor(node->parent, stepSis);
			assignColor('B', node->parent);

			if (node == node->parent->left) {
				assignColor('B', stepSis->right);
				redBlackTreeLeftRotate(node->parent);
			} else {
				assignColor('B', stepSis->left);
				redBlackTreeRightRotate(node->parent);
			}
		}
	}
}

//returns the sibling of the parent node in input, returns null if the parent is its parent only child
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

//utility function to retrieve the parent node, for the ease of use
intervalTree* getParent(intervalTree *node) {
	// Note that parent is set to null for the root node.
	if (node == NULL) {
		return NULL;
	} else {
		return node->parent;
	}
}
