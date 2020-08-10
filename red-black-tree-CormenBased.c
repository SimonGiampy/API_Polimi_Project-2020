// Implementing Red-Black Tree in C++

#include <stdlib.h>
#include <stdio.h>

struct Node {
	int data;
	struct Node *parent;
	struct Node *left;
	struct Node *right;
	int color;  //red = 1, black = 0
};

typedef struct Node *NodePtr;

NodePtr root;
NodePtr TNULL;

void leftRotate(NodePtr x);
NodePtr minimum(NodePtr node);
void rightRotate(NodePtr x);

void initializeNULLNode(NodePtr node, NodePtr parent) {
	node->data = 0;
	node->parent = parent;
	node->left = NULL;
	node->right = NULL;
	node->color = 0;
}

NodePtr getParent(NodePtr n) {
	// Note that parent is set to null for the root node.
	return n == NULL ? NULL : n->parent;
}

NodePtr searchTreeHelper(NodePtr node, int key) {
	if (node == TNULL || key == node->data) {
		return node;
	}

	if (key < node->data) {
		return searchTreeHelper(node->left, key);
	}
	return searchTreeHelper(node->right, key);
}

// For balancing the tree after deletion
void deleteFix(NodePtr x) {
	NodePtr s;
	while (x != root && x->color == 0) {
		if (x == x->parent->left) {
			s = x->parent->right;
			if (s->color == 1) {
				s->color = 0;
				x->parent->color = 1;
				leftRotate(x->parent);
				s = x->parent->right;
			}

			if (s->left->color == 0 && s->right->color == 0) {
				s->color = 1;
				x = x->parent;
			} else {
				if (s->right->color == 0) {
					s->left->color = 0;
					s->color = 1;
					rightRotate(s);
					s = x->parent->right;
				}

				s->color = x->parent->color;
				x->parent->color = 0;
				s->right->color = 0;
				leftRotate(x->parent);
				x = root;
			}
		} else {
			s = x->parent->left;
			if (s->color == 1) {
				s->color = 0;
				x->parent->color = 1;
				rightRotate(x->parent);
				s = x->parent->left;
			}

			if (s->left->color == 0 && s->right->color == 0) {
				s->color = 1;
				x = x->parent;
			} else {
				if (s->left->color == 0) {
					s->right->color = 0;
					s->color = 1;
					leftRotate(s);
					s = x->parent->left;
				}

				s->color = x->parent->color;
				x->parent->color = 0;
				s->left->color = 0;
				rightRotate(x->parent);
				x = root;
			}
		}
	}
	x->color = 0;
}

void rbTransplant(NodePtr u, NodePtr v) {
	if (u->parent == NULL) {
		root = v;
	} else if (u == u->parent->left) {
		u->parent->left = v;
	} else {
		u->parent->right = v;
	}
	v->parent = u->parent;
}

void deleteNodeHelper(NodePtr node, int key) {
	NodePtr z = TNULL;
	NodePtr x, y;
	while (node != TNULL) {
		if (node->data == key) {
			z = node;
		}

		if (node->data <= key) {
			node = node->right;
		} else {
			node = node->left;
		}
	}

	if (z == TNULL) {
		return; //key not found
	}

	y = z;
	int y_original_color = y->color;
	if (z->left == TNULL) {
		x = z->right;
		rbTransplant(z, z->right);
	} else if (z->right == TNULL) {
		x = z->left;
		rbTransplant(z, z->left);
	} else {
		y = minimum(z->right);
		y_original_color = y->color;
		x = y->right;
		if (y->parent == z) {
			x->parent = y;
		} else {
			rbTransplant(y, y->right);
			y->right = z->right;
			y->right->parent = y;
		}

		rbTransplant(z, y);
		y->left = z->left;
		y->left->parent = y;
		y->color = z->color;
	}
	//delete z;
	free(z);
	if (y_original_color == 0) {
		deleteFix(x);
	}
}

// For balancing the tree after insertion
void insertFix(NodePtr z) {
	NodePtr u, par, grand;
	while (z != root && z != root->left && z != root->right && z->parent->color == 1) {
		par = getParent(z);
		grand = getParent(getParent(z));
		//extra cases
		if (z == par->right && par == grand->left) {
			leftRotate(par);
			z = z->left;
			return;
		} else if (z == par->left && par == grand->right) {
			rightRotate(par);
			z = z->right;
			return;
		}
		if (z == z->parent->left) {
			par->color = 0;
			grand->color = 1;
			rightRotate(grand);
			return;
		} else if (z == z->parent->right) {
			par->color = 0;
			grand->color = 1;
			leftRotate(grand);
			return;
		}

		if (z->parent == z->parent->parent->right) {
			u = z->parent->parent->left;
			if (u->color == 1) {
				u->color = 0;
				z->parent->color = 0;
				z->parent->parent->color = 1;
				z = z->parent->parent;
			} else {
				if (z == z->parent->left) {
					z = z->parent;
					rightRotate(z);
				}
				z->parent->color = 0;
				z->parent->parent->color = 1;
				leftRotate(z->parent->parent);
			}
		} else {
			u = z->parent->parent->right;

			if (u->color == 1) {
				u->color = 0;
				z->parent->color = 0;
				z->parent->parent->color = 1;
				z = z->parent->parent;
			} else {
				if (z == z->parent->right) {
					z = z->parent;
					leftRotate(z);
				}
				z->parent->color = 0;
				z->parent->parent->color = 1;
				rightRotate(z->parent->parent);
			}
		}
		if (z == root) {
			break;
		}
	}
	root->color = 0;
}


/*
RedBlackTree() {
	TNULL = new Node;
	TNULL->color = 0;
	TNULL->left = NULL;
	TNULL->right = NULL;
	root = TNULL;
}
*/


NodePtr searchTree(int k) {
	return searchTreeHelper(root, k);
}

NodePtr minimum(NodePtr node) {
	while (node->left != TNULL) {
		node = node->left;
	}
	return node;
}

NodePtr maximum(NodePtr node) {
	while (node->right != TNULL) {
		node = node->right;
	}
	return node;
}

NodePtr successor(NodePtr x) {
	if (x->right != TNULL) {
		return minimum(x->right);
	}

	NodePtr y = x->parent;
	while (y != TNULL && x == y->right) {
		x = y;
		y = y->parent;
	}
	return y;
}

NodePtr predecessor(NodePtr x) {
	if (x->left != TNULL) {
		return maximum(x->left);
	}

	NodePtr y = x->parent;
	while (y != TNULL && x == y->left) {
		x = y;
		y = y->parent;
	}

	return y;
}

void leftRotate(NodePtr x) {
	NodePtr y = x->right;
	x->right = y->left;
	if (y->left != TNULL) {
		y->left->parent = x;
	}
	y->parent = x->parent;
	if (x->parent == NULL) {
		root = y;
	} else if (x == x->parent->left) {
		x->parent->left = y;
	} else {
		x->parent->right = y;
	}
	y->left = x;
	x->parent = y;
}

void rightRotate(NodePtr x) {
	NodePtr y = x->left;
	x->left = y->right;
	if (y->right != TNULL) {
		y->right->parent = x;
	}
	y->parent = x->parent;
	if (x->parent == NULL) {
		root = y;
	} else if (x == x->parent->right) {
		x->parent->right = y;
	} else {
		x->parent->left = y;
	}
	y->right = x;
	x->parent = y;
}

// Inserting a node
void insert(int key) {
	NodePtr node = (NodePtr) malloc(sizeof(struct Node));
	node->parent = NULL;
	node->data = key;
	node->left = TNULL;
	node->right = TNULL;
	node->color = 1;

	NodePtr y = NULL;
	NodePtr x = root;

	while (x != TNULL) {
		y = x;
		if (node->data < x->data) {
			x = x->left;
		} else {
			x = x->right;
		}
	}

	node->parent = y;
	if (y == NULL) {
		root = node;
	} else if (node->data < y->data) {
		y->left = node;
	} else {
		y->right = node;
	}
	/*
	if (node->parent == NULL) {
		node->color = 0;
		return;
	}

	if (node->parent->parent == NULL) {
		return;
	}
	 */

	insertFix(node);
}

NodePtr getRoot() {
	return root;
}

void deleteNode(int data) {
	deleteNodeHelper(root, data);
}

//displays the tree structure with the form: node value, left subtree, right subtree, for every node in the tree
void showTreeStructure(NodePtr node) {
	if (node != NULL) {
		printf("node<%d>: ", node->data);
		if (node->left != NULL) {
			printf("sx:<%d> ", node->left->data);
		} else {
			printf("sx:NULL ");
		}
		if (node->right != NULL) {
			printf("dx:<%d> ", node->right->data);
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
void printInorderTrasversal(NodePtr node) {
	if (node == NULL) return;

	printInorderTrasversal(node->left);
	printf("<%d> - ", node->data);
	printInorderTrasversal(node->right);
}

int main() {
	/*
	int numbers[20];
	for (int i = 0; i < 20; ++i) {
		numbers[i] = (int) random() % (1000 + 1);
		printf("%d ", numbers[i]);
	}
	printf("\n");
	//random insertion of 10 elements from 0 to 100
	for (int i = 0; i < 20; ++i) {
		insert(numbers[i]);
	}
	 */
	insert(67);
	insert(56);
	insert(34);
	insert(36);
	insert(40);
	insert(50);

	printf("inorder trasversal: ");
	printInorderTrasversal(root);
	printf("\n");
	showTreeStructure(root);

}