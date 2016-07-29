#include <iostream>
#include <sstream>
#include <assert.h>
#include <vector>

using namespace std;

int * elements = nullptr;

int compute_diff_n_nplus1(int n)
{
	return elements[n + 1] - elements[n];
}

void Test_Task1()
{
	int numElements;
	cin >> numElements;

	elements = new int [numElements];
	for (int i = 0; i < numElements; i++)
	{
		cin >> elements[i];
	}

	for (int i = 0; i < numElements - 1; i++)
	{
		int diff = compute_diff_n_nplus1(i);
		int diff_next = compute_diff_n_nplus1(i + 1);

		if (diff != diff_next)
		{
			int d = 0;

			if (i > 0) 
				d = compute_diff_n_nplus1(i - 1);			
			else if ((i + 2 + 1) >= numElements) 
				d = diff; 
			else 
				d = compute_diff_n_nplus1(i + 2);
			
			int missingValue = d;

			if (d != diff)
				missingValue += elements[i];
			else 
				missingValue += elements[i + 1];

			cout << missingValue;
			break;
		}
	}
	
	delete elements;
	elements = nullptr;
}

void Run_Task1(stringbuf * input, int result)
{
	stringbuf b;
	streambuf * const cout_buf = cout.rdbuf(&b);
	streambuf * const cin_buf = cin.rdbuf(input);

	Test_Task1();

	assert(atoi(b.str().c_str()) == result);

	cout.rdbuf(cout_buf);
	cin.rdbuf(cin_buf);
}


struct node {
	struct node *left, *right;
	int val;
};
typedef struct node node;

node * addElement(int val)
{
	node * root = static_cast<node*>(malloc(sizeof(node)));
	root->left = root->right = nullptr;
	root->val = val;

	return root;
}

int diameterOfTree(node * treeNode, int depth)
{
	if (!treeNode || (!treeNode->left && !treeNode->right))
	{
		return depth;
	}	

	int depth_l, depth_r;
	depth_l = depth_r = 0;

	depth++;

	if (treeNode->left)
	{		
		depth_l = diameterOfTree(treeNode->left, depth);
	}

	if (treeNode->right)
	{
		depth_r = diameterOfTree(treeNode->right, depth);
	}

	if (depth_l > depth_r)
		return depth_l;
	else
		return depth_r;
}

void Test_Task2(node * root)
{
	int d_left, d_right;
	d_left = d_right = 0;

	if (root->left)
		d_left = diameterOfTree(root->left, 1);
	
	if (root->right)
		d_right = diameterOfTree(root->right, 1);

	cout << d_left + d_right + 1 << endl;
}

node * CreateTestTree1()
{
	node * root = addElement(40);
	root->left = addElement(30);
	root->left->left = addElement(22);
	root->left->right = addElement(38);

	root->right = addElement(65);
	root->right->right = addElement(78);

	return root;
}

node * CreateTestTree2()
{
	node * root = addElement(1);
	root->left = addElement(2);
	root->left->left = addElement(3);
	root->left->right = addElement(4);
	root->left->right->left = addElement(5);
	root->left->right->right = addElement(6);

	root->right = addElement(7);
	root->right->right = addElement(8);
	root->right->right->right = addElement(9);
	root->right->right->right->left = addElement(10);
	root->right->right->right->right = addElement(11);
	root->right->right->right->left->left = addElement(12);
	root->right->right->right->left->right = addElement(13);

	return root;
}

void DeleteTree(node * root)
{
	if (root)
	{
		if (root->left)
			DeleteTree(root->left);

		if (root->right)
			DeleteTree(root->right);
		
		delete root;
		root = nullptr;
	}
}

int getNumberOfPrimes(int N)
{
	int numPrimes = 1;
	vector<int> a(N);
	a[0] = 2;

	for (int i = 3; i <= N; i++)
	{
		for (size_t j = 0; j < a.size(); j++)
		{
			if (i % a[j] == 0)
			{			
				// i can be divided without remainder => not prime
				goto nextIteration;
			}	
		}

		//cout << "#" << i << endl;
		numPrimes++;
		a.push_back(i);

		nextIteration: continue;
	}

	return numPrimes;
}

void main()
{
	/* problem 3 */
	cout << getNumberOfPrimes(100) << endl;
	cout << getNumberOfPrimes(1000000) << endl;

	/* problem 2 */
	node* tree = CreateTestTree1();
	Test_Task2(tree);
	DeleteTree(tree);

	tree = CreateTestTree2();
	Test_Task2(tree);
	DeleteTree(tree);


	/* problem 1 */
	stringbuf a("5\r\n1 11 31 41 51");
	Run_Task1(&a, 21);

	stringbuf a1("5\r\n1 3 5 9 11");
	Run_Task1(&a1, 7);

	stringbuf a2("4\r\n-1 4 9 19");
	Run_Task1(&a2, 14);

	stringbuf a3("3\r\n-1 4 14");
	Run_Task1(&a3, 9);

	stringbuf a4("5\r\n-1 -4 -7 -13 -16");
	Run_Task1(&a4, -10);
}
