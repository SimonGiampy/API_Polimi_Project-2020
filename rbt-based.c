#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>
//online compiler password: VMoY6CY1

//maximum number of elements in the hash table
#define DEBUG_ACTIVE 0 //debugging flag

struct stringList { //single linked list for handling changes in the text
	char* string;
	struct stringList* next;
};
typedef struct stringList stringList;

struct collection {
	stringList *stack;
	unsigned short currentPosition;
	//TODO: an idea could be to add a string pointer, so the access to the current string for printing is immediate (costs 8 bytes)
	bool deletedFlag;
};
typedef struct collection collection;

struct command { //every input is of the form (ind1,ind2)action
	int start;
	int end;
	char command;
};

struct deletedHistory { //contains the list of deleted indexes
	struct deletedHistory *next;
	int number;
	int *list;
};
typedef struct deletedHistory delHistory;

//function explicit declarations
void insertHashTable(int i, char* line);
void input(FILE *fp);
int hash(int n);
void change( int start, int end, char** text);
void delete(int start, int end);
void debugPrintFullHashLine(int index, stringList* pointer);
void debugPrintFullTable(bool showFullLineContents);
void debugPrintTombstones(delHistory *tombs);
struct command* getCommand(char* input);
void quit();

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

struct quickLook { //linked list for the quick access intervals
	intervalTree *piece;
	struct quickLook *next;
};
struct quickLookupsHead { //stack of type FILO (first in, last out) for keeping track of last accessed intervals
	struct quickLook *head; //pointer to the first element
	struct quickLook *end; //pointer to the last element, TODO: this is probably not needed, since it is never accessed
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
void adjustParametersAfterDeletion(intervalTree* node, int low, int high);

void assignColor(char color, intervalTree* dest);
void copyColor(intervalTree* source, intervalTree* dest);
bool isRed(intervalTree* node);
bool isBlack(intervalTree* node);
int lookup(int key);
void addIntervalForQuickLookups(intervalTree* node);
int quickLookup(int key);
void debugQuickLookups();

intervalTree *tree; //global variable that stores the entire tree structure, pointer to the root node

//global variable that defines the hash table (array of pointers to stringList objects, rows start from 1)
collection **hashTable;
int hashCapacity; //number of rows which have been allocated
int lastRow; //number of the last line that contains some text
delHistory *tombstones = NULL;

int main() {
	//initialization of the list of deleted lines

	tombstones = (delHistory *) malloc(sizeof(delHistory));
	tombstones->number = 0;
	tombstones->next = NULL;
	tombstones->list = NULL;

	//initialization of quicklookups structure
	quickLookupsHead = (struct quickLookupsHead*) malloc(sizeof(struct quickLookupsHead));
	quickLookupsHead->capacity = 15; //just for testing a limited amount of intervals
	quickLookupsHead->occupied = 0;

	hashCapacity = 0; //starting point for the hash table dimension
	hashTable = malloc(1 * sizeof(collection));
	hashTable[0] = NULL;
	lastRow = 0;

	if (DEBUG_ACTIVE) {
		//char fileName[] = "/home/simon/CS Project/Time_for_a_change_1_input.txt";
		char fileName[] = "/home/simon/Downloads/test-cases-project/level4/test1000.txt"; //only for testing and debugging
		//char fileName[] = "/home/simon/CS Project/TextEditor/inputTest.txt";
		FILE *fp = fopen(fileName, "r"); //reads from a file, used for debugging
		input(fp); //fp must be stdin when submitting the code on the platform
	} else {
		input(stdin);
	}

	return 0;
}

int hash(int n) { //hash function with linear hashing
	return quickLookup(n);
}

void change(int start, int end, char** text) { //changes the text
	//function declaration without malloc is: char (*text)[1024]
	int range = end - start + 1;
	int i;

	//increment size of hash table with a fast approximation, not every time it is needed
	if (hash(end) > hashCapacity) {
		hashTable = realloc(hashTable, (hashCapacity + range * 10) * sizeof(collection));
		for (int j = hashCapacity; j <= hashCapacity + range; j++) {
			hashTable[j] = NULL;
		}
		hashCapacity += range * 10;
	}
	if (end > lastRow) {
		lastRow = end;
	}

	int currentPos = hash(start);
	for (i = 0; i < range; i++) { //every row is inserted in the hash table
		insertHashTable(currentPos, text[i]);
		start++;
		currentPos = hash(start);
	}

}

void insertHashTable(int index, char *line) { //inserts single string in the hash table
	stringList *str = (stringList*) malloc(sizeof(stringList));
	//currently it only handles head-insertions
	str->string = line;

	if (hashTable[index] == NULL) {
		collection *coll = (collection*) malloc(sizeof(collection));
		coll->deletedFlag = false;
		coll->currentPosition = 1;
		coll->stack = str;
		str->next = NULL;
		hashTable[index] = coll;
	} else {
		str->next = hashTable[index]->stack;
		hashTable[index]->stack = str;
	}

}

void delete(int start, int end) {
	//TODO: make sure the interval is derived correctly
	if (start == 0 && end == 0) {
		//a delete call that doesn't do anything
		delHistory *newTomb = (delHistory*) malloc(sizeof(struct deletedHistory));
		newTomb->next = tombstones; //insert at the head of the linked list
		newTomb->list = (int *) malloc(sizeof(int));
		newTomb->list = 0;
		newTomb->number = 0;
		tombstones = newTomb;
		return;
	}
	if (end > lastRow) { //deals with useless delete calls on non-existent indexes in the hash table
		end = lastRow; //but doesn't take into account the rows in the middle of the selected ones
	}

	if (start == 0) {
		start = 1;
	} else if (start > lastRow) {
		//delete call doesn't do anything
		delHistory *newTomb = (delHistory*) malloc(sizeof(struct deletedHistory));
		newTomb->next = tombstones; //insert at the head of the linked list
		newTomb->list = (int *) malloc(sizeof(int));
		newTomb->list = 0;
		newTomb->number = 0;
		tombstones = newTomb;
		return;
	}

	int range = end - start + 1;

	delHistory *newTomb = (delHistory*) malloc(sizeof(struct deletedHistory));
	newTomb->next = tombstones; //insert at the head of the linked list
	newTomb->list = (int *) malloc(range * sizeof(int)); //allocates more than the necessary

	int count = 0; //counter for tombstones
	int forward = start; //counter for finding the correct lines
	int i = hash(start);

	while (hashTable[i] != NULL && count < range) { //fix here for the new mechanism
		newTomb->list[count] = i; //memorization of deleted lines from the hash table
		count++;
		hashTable[i]->deletedFlag = true;
		forward++; //next element to delete
		i = hash(forward); //hashing every element to find the correct ones
	}
	//could be added a realloc to shrink the array and reduce the number of elements to the ones strictly necessary
	//but I don't do this since it may take some time to be executed numerous times
	newTomb->number = count; //added line for keeping track of number of deleted elements
	tombstones = newTomb; //changes the list of tombstones by accessing at its address

	if (DEBUG_ACTIVE) {
		debugPrintTombstones(tombstones);
	}

	int hashStart = tombstones->list[0], hashEnd = tombstones->list[count - 1];
	insertInterval(hashStart,hashEnd);
	if (DEBUG_ACTIVE) printf("added interval <%d,%d> \n", hashStart, hashEnd);
}

void debugPrintTombstones(delHistory *tombs) {
	//prints the sequence list of deleted lines in the hash table
	//one element for every delete call, and every element is a list of indexes
	int c = 0;
	while (tombs != NULL) {
		c++;
		printf("deleted history %d: ", c);
		for (int i = 0; i < tombs->number; i++) {
			printf("%d ", tombs->list[i]);
		}
		printf("\n");
		tombs = tombs->next;
	}
}

void input(FILE *fp) { //fp is the file pointer passed from main
	if (fp == NULL) return;
	char buffer[1024 + 1]; //1024 max characters, +1 for \n, +1 for \0
	char* correctRead;

	while (!feof(fp)) { //until the input file is not finished reading (end of file)
		correctRead = fgets(buffer, 1024 + 1, fp);
		assert(correctRead != NULL);

		struct command* command = getCommand(buffer); //translates the line with the input (ind1,ind2)command in a struct containing the parameters
		int start = command->start;
		int end = command->end;
		int num = end - start + 1;
		char action = command->command;
		free(command);

		if (action == 'c') {
			char *text[num]; //allocates an array of num strings, one for each line
			for (int i = 0; i < num; i++) {
				correctRead = fgets(buffer, 1024 + 1, fp);
				assert(correctRead != NULL);
				text[i] = (char*) malloc(sizeof(char) * strlen(buffer) + 1);
				strcpy(text[i], buffer);
			}
			correctRead = fgets(buffer, 3, fp); //reads the terminal sequence of the input text ".\n\0" -> 3 chars
			assert(correctRead != NULL);
			//assert(strcmp(buffer, ".\n") == 0); //exits in case there isn't a full stop, this should never be called
			if (DEBUG_ACTIVE) {
				printf("-----------------change call %d,%d \n", start, end);
			}
			change(start, end, text);

			if (DEBUG_ACTIVE) {
				debugPrintFullTable(false);
			}

		} else if (action == 'p') {
			//print lines, used only for the online compiler
			if (DEBUG_ACTIVE) {
				printf("print sequence from %d to %d started:\n", start, end);
			}
			stringList *str;
			for (int i = start; i <= end; i++) {
				if (hashTable[hash(i)] != NULL) {
					str = hashTable[hash(i)]->stack;
					//goes forward until it finds the current string
					//can be optimized by storing the current selected string in the collection struct table
					for (int j = 1; j < hashTable[hash(i)]->currentPosition; j++) {
						str = str->next;
					}
					fputs(str->string, stdout); //only first string in each list
				} else {
					fputs(".\n", stdout); //no string present in the selected line
				}
			}

		} else if (action == 'd') {
			//delete lines, import from dynamic indexes
			if (DEBUG_ACTIVE) {
				printf("-----------------delete call %d,%d\n", start, end);
			}

			delete(start, end);
			if (DEBUG_ACTIVE) {
				debugPrintFullTable(false);
			}

		} else if (action == 'u' && end == 0) {
			//TODO: implement the function for the fusion of undos and redos in a single command
			//undo
		} else if (action == 'r' && end == 0) {
			//redo
		}

	}
	fclose(fp);
}

struct command* getCommand(char* input) { //translates the input string in a struct containing start, end and the action
	if (strcmp(input, "q\n") == 0 || strcmp(input, "q") == 0) { //equal strings
		quit();
	}
	//TODO: implement faster read using the function fread, which only stops at EOF, with the help of strtok
	struct command *commando = (struct command*) malloc(sizeof(struct command));
	//new better error-free version
	char* endPtr;
	int start = (int) strtol(input, &endPtr, 10);
	int end = 0;
	if (*endPtr == ',') {
		end = (int) strtol(endPtr + 1, &endPtr, 10);
	}
	char action = *endPtr;

	commando->start = start;
	commando->end = end;
	commando->command = action;
	return commando; //returns the struct containing the information of the command in input
}

void quit() { //input = 'q'
	exit(0);
}

void debugPrintFullHashLine(int index, stringList* pointer) { //prints all the contents in a single line of a hash table
	if (pointer->next == NULL) {
		printf("%s --> null\n", pointer->string);
	} else {
		printf("%s --> ", pointer->string);
		debugPrintFullHashLine(index, pointer->next);
	}
}


void debugPrintFullTable(bool showFullLineContents) { //prints all the lines in the hash table from line 1 to max
	int i = 1;
	if (showFullLineContents == false) { //prints only the first string for every row
		while (hashTable[i] != NULL) {
			if (hashTable[i]->deletedFlag == true) {
				printf("DELETED\n");
			} else {
				printf("%s", hashTable[i]->stack->string);
			}
			i++;
		}
	} else { //prints every string for every row
		while (hashTable[i] != NULL) {
			printf("%d: ", i);
			if (hashTable[i]->deletedFlag == true) {
				printf("DELETED\n");
			} else {
				debugPrintFullHashLine(i, hashTable[i]->stack); //full hash line debug
			}
			i++;
		}
	}
}



//calculates the exact row where a line = key is stored
int lookup(int key) {
	intervalTree *node = tree;

	if (tree == NULL) { //when no deletions occurred the tree is empty
		return key;
	}

	intervalTree *match = NULL; //save in memory the node which skips value is the closest as possible to the key, but greater than the key

	while (node != NULL) {
		if (key > node->skips) { //goes to the right subtree
			if ((node->skips > key && match == NULL) || (node->skips > key && node->skips < match->skips)) {
				match = node; //match update
			}
			if (node->right == NULL) {
				if (match == NULL) { //the rightmost node in the tree has a different index calculation
					addIntervalForQuickLookups(node);
					//printf("node chosen is <%d,%d,%d>\n", node->a, node->b, node->skips);
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
			//and the probability of it being found again is very low
			//printf("node chosen is <%d,%d,%d>\n", node->a, node->b, node->skips);
			intervalTree *prev = node;
			while (node != NULL && node->skips == key) {
				prev = node;
				node = inOrderPreviousNode(node);
			}
			return prev->lowEndpoint - 1;
		}

	}

	if (match != NULL) { //should be match
		//printf("node chosen is <%d,%d,%d>\n", match->a, match->b, match->skips);
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

	while (current != NULL && counter < quickLookupsHead->capacity) {
		//TODO: fsanitize flag says there is an error here that causes the segmentation fault in time for a change
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
			while (counter < quickLookupsHead->capacity) { //scrolls the list until it reaches the element before the last one
				tmp = tmp->next;
				counter++;
			}
			free(tmp->next); //deletes the last element when the occupied size is greater than the capacity
			tmp->next = NULL; //last element has a null next pointer
			quickLookupsHead->end = tmp; //adjust the end pointer, by scrolling the list
		} else if (quickLookupsHead->occupied < quickLookupsHead->capacity) {
			quickLookupsHead->occupied += 1; //increase if the stack isn't full yet
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
	if (node == NULL) return NULL;
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
	if (node == NULL) return NULL;
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
}
bool isBlack(intervalTree* node) { //returns false if the node is black, true if it's red
	if (node->color == 'B') return true; else return false;
}
void copyColor(intervalTree* source, intervalTree* dest) {
	dest->color = source->color;
}
void assignColor(char color, intervalTree* dest) {
	dest->color = color;
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

void adjustParametersAfterDeletion(intervalTree* node, int low, int high) {
	//the selected node is situated just before the deleted node
	int number; //= number of deleted elements

	intervalTree *current = node;
	int elementHigh = current->b; //keeps track of the high endpoint of the bigger overlapping interval
	int elementLow = current->a; //keeps track of the low endpoint of the bigger overlapping interval

	if (high == current->highEndpoint && low == current->lowEndpoint) {
		//there are sub-intervals
		number = node->a - low;
		while (current != NULL && current->b < high && current->a > low) {
			if (current->a > elementLow && current->b < elementHigh) { //smaller sub-intervals
				current->lowEndpoint = elementLow;
				current->highEndpoint = elementHigh;
			} else { //new big sub-interval
				if (current != node) {
					number += current->a - elementHigh - 1; //new offset between two smaller intervals
				}
				current->lowEndpoint = current->a;
				current->highEndpoint = current->b;
				elementLow = current->a;
				elementHigh = current->b;
			}
			current->skips += number;
			current = inOrderNextNode(current);
		}
		if (current != NULL) { //other nodes to examine
			number += high - elementHigh; //after last element add more index
		}

	} else {
		//skips updated of a length = high - low + 1 for every interval in the tree
		number = high - low + 1;
	}

	while (current != NULL) { //updates the values which come after the last overlapped interval
		current->skips += number;
		current = inOrderNextNode(current);
	}
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
	intervalTree *savedNode = inOrderNextNode(node); //this is needed to correct the parameters after a delete
	int deletedNodeLowEnd = node->lowEndpoint, deletedNodeHighEnd = node->highEndpoint;
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
		adjustParametersAfterDeletion(savedNode, deletedNodeLowEnd, deletedNodeHighEnd); //fixes the structure with its parameters
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
		adjustParametersAfterDeletion(savedNode, deletedNodeLowEnd, deletedNodeHighEnd); //fixes the structure with its parameters
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
	adjustParametersAfterDeletion(savedNode, deletedNodeLowEnd, deletedNodeHighEnd); //fixes the structure with its parameters
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

