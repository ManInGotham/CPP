#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <stack>

using namespace std;


// Node in a tree
struct NODE {
	NODE *left;
	NODE *right;
	char text;
};

struct NODE_LEVEL {
	NODE *node;
	int level; // 0 is the tree root
};

// Client is in charge of deleting the ptr
NODE * createNode(char text) {
	auto node = new NODE;
	node->left = nullptr;
	node->right = nullptr;
	node->text = text;

	cout << "cr " << text << endl;
	return node;
}


void deleteNode(NODE* node) {
	if (node) {
		if (node->left) deleteNode(node->left);
		if (node->right) deleteNode(node->right);

		cout << "del " << node->text << endl;
		delete node;
		node = nullptr;
	}
}

int main()
{		
	stack<NODE_LEVEL> unprocessed;
	map<char, NODE*> index;
	int maxDepth = 0;
		
	stringstream ss;
	stringbuf *pbuf = ss.rdbuf();
	string test ("a,b,c,b,g,k,k,l,");
	pbuf->sputn(test.c_str(), test.length());
	cin.rdbuf(pbuf);

	NODE *tree;
	bool isRootSelected = false;

	char parent[1], left[1], right[1];
	while (!cin.eof()) {
		cin.getline(parent, 2, ',');		
		cin.getline(left, 2, ',');
		cin.getline(right, 2, ',');

		NODE *nodeP = index[*parent];
		NODE *nodeL, *nodeR;
		nodeL = nodeR = nullptr;


		if (!nodeP) {
			nodeP = createNode(*parent);
			index[*parent] = nodeP;

			if (!isRootSelected) {
				isRootSelected = true;
				tree = nodeP;
			}
		}

		if (*left != '\0') {
			nodeL = index[*left];
			if (!nodeL) {
				nodeL = createNode(*left);
				index[*left] = nodeL;
			}
		}
		nodeP->left = nodeL;

		if (*right != '\0') {
			nodeR = index[*right];
			if (!nodeR) {
				nodeR = createNode(*right);
				index[*right] = nodeR;
			}
		}
		nodeP->right = nodeR;
	}
	

	// Add the root
	unprocessed.push(NODE_LEVEL{ tree, 0 });

	while (!unprocessed.empty()) {
		NODE_LEVEL node_level = unprocessed.top();
		unprocessed.pop(); // removing from the stack

		int depth = node_level.level;
		NODE *node = node_level.node;

		while(node) {			
			if (node->right) {
				// Adding for processing later
				unprocessed.push(NODE_LEVEL{ node->right, depth+1 });
			}

			// moving along the left nodes
			if (node->left) 
				depth++;

			node = node->left;
		}

		if (depth > maxDepth)
			maxDepth = depth;
	}

	cout << maxDepth << endl;

	deleteNode(tree);
	tree = nullptr;

	return 0;
}