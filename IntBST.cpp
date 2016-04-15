/*
 * IntBST.cpp - Tufts Comp15 Hw4
 * Dillon Bostwick Mar 13, 2016
 *
 * See IntBST.h for more info
 */

#include <iostream>
#include <climits>
#include <fstream>
#include <sstream>
#include <vector>

#include "IntBST.h"

using namespace std;

//////////////////////////////////////////////////////////////////////////
////////////////METHODS THAT I ADDED FOR SHARKBATCH///////////////////////

vector<int> IntBST::return_vector() {
	vec.clear();
	return_vector(root);
	return vec;
}

void IntBST::return_vector(Node *node) {
	if (node == NULL)
		return;
	
	return_vector(node->left);
	return_vector(node->right);
	
	vec.push_back(node->data);
}

bool IntBST::is_empty() {
	return root == NULL;
}

void IntBST::print() {
	if (root == NULL) {cout << "N/A" << endl;}
	print(root);
}

void IntBST::print(Node *node) {
	if (node == NULL)
		return;
	
	print(node->left);
	print(node->right);
	
	cout << node->data << endl;
}


//////////////////////////////////////////////////////////////////////////

//constructor sets root to NULL. At this point,
//the tree is empty; height==-1; nodes==0.
IntBST::IntBST()
{
    root = NULL;
}

// destructor walks tree in post-order traversal to delete
IntBST::~IntBST()
{
	post_order_delete(root);
}

// Post-order traversal delete implemented recursively. Pass
// the root of the tree you wish to free from memory.
void IntBST::post_order_delete(Node *node)
{
	if (node != NULL) {
		post_order_delete(node->left);
		post_order_delete(node->right);
		
		delete node;
		node = NULL;
	}
}

// Copy constructor - uses pre-order traverse to copy recursively
IntBST::IntBST(const IntBST &source)
{
	root = pre_order_copy(source.root);
}

// Assignment overload - uses post-order traverse to delete
// the current tree, then uses pre-order traverse to copy
// from the source tree.
IntBST &IntBST::operator= (const IntBST &source)
{
	if (this != &source) { //equivalency check
		this->post_order_delete(this->root);
		this->root = pre_order_copy(source.root);
	}
	
	return *this;
}

// Recursive pre-order traversal copy. Pass the root of the
// tree you wish to copy, and it will return a pointer to
// the new root
Node *IntBST::pre_order_copy(Node *node) const
{
	if (node == NULL) return NULL;

	//create a new node and fill it up.
	Node *newNode  = new Node();
	newNode->data  = node->data;
	newNode->count = node->count;
	//recursively fill left and right of the new node
	newNode->left  = pre_order_copy(node->left);
	newNode->right = pre_order_copy(node->right);
	
	return newNode;
}

// find_min() returns the minimum value in the tree
// returns INT_MIN if the tree is empty
int IntBST::find_min() const
{
	if (root == NULL) //check if list is empty
		return INT_MAX;
		
    return find_min(root)->data;
}

//overload of find_min() is recursive helper function. Node
//is traversed to it's left until it can't walk to the left
//any further.
Node *IntBST::find_min(Node *node) const
{
    if (node->left == NULL)
		return node;
	
	return find_min(node->left);
}

// find_max() returns the maximum value in the tree
// returns INT_MAX if tree is empty
int IntBST::find_max() const
{
	//check if empty list
	if (root == NULL)
		return INT_MAX;

    return find_max(root)->data;
}

//similar to find_min helper function. Recursive overload of
//find_max() takes advantage of invariant aspect of BST by
//walking down the right until it reaches NULL.
Node *IntBST::find_max(Node *node) const
{
    if (node->right == NULL)
		return node;
	
	return find_max(node->right);
}

// contains() returns true if the value is in the tree,
// false if it is not in the tree
bool IntBST::contains(int value) const
{
	return contains(root, value);
}

// Recursive helper function overload of contains(). Traverses
// only in the direction of the value it is trying to find,
// To achieve O(log n)
bool IntBST::contains(Node *node, int value) const
{
	if (node == NULL)
		return false;
	if (value == node->data) //found node
		return true;
	
	//traverse left or right
	if (value < node->data)
		return contains(node->left, value);
	else
		return contains(node->right, value);
}

// Inserts an int value into the Binary Search Tree.
// If the value is already in the tree, the node's count
// is incremented by 1. This function will insert on it's own
// if it knows the tree is empty, but otherwise it calls the
// overloaded function as a recursive helper
void IntBST::insert(int value)
{
	//check if tree is empty, then sets root to first node
	if (root == NULL) {
		root        = new Node();
		root->data  = value;
		root->count = 1;
		root->left  = NULL;
		root->right = NULL;
	} else {
	//otherwise use recursive helper
		insert(root, NULL, value);
	}
}

//Recursive helper for insert(int). Must pass the current node
//that is being traversed, as well as the parent of the current
//node, and of course, the original value to be inserted must
//be added as well. See function comments for details
void IntBST::insert(Node *node, Node *parent, int value)
{
    if (node == NULL) {
		//create new node
		node          = new Node;
		node->data    = value;
		node->count   = 1;
		node->left    = NULL;
		node->right   = NULL;
		//relink parent back to the node
		if (value < parent->data)
			parent->left = node;
		else
			parent->right = node;
	} else if (value == node->data) {
		//value already exists; just increment count.
		node->count++;
	} else if (value < node->data) {
		//recursive traverse left if val is less than.
		insert(node->left, node, value);
	} else if (value > node->data) {
		//recursive traverse right if val is greater than
		insert(node->right, node, value);
	} else {
		throw std::exception(); //this shouldn't happen
	}
}

// remove() does a full removal from the tree (NOT lazy removal)
// If a node's count is greater than one, the count is
// decremented, and the node is not removed.
// Nodes with a count of 0 follow this algorithm: if the node has no
// children, simply remove it. If the node has one child,
// "bypass" the node to the child from the parent. If the node
// has two children, first find the minimum node of the
// right child, replace the node's data with the value and
// count of right's minimum, and then recursively delete
// right's minimum.
bool IntBST::remove(int value)
{
	return remove(root, NULL, value);
}

// This helper function primarily searches the tree to see if
// the node exists. removeNode takes care of the actual removal
// NB the algorithm looks similar to that of contains().
bool IntBST::remove(Node *node, Node *parent, int value)
{
	if (node == NULL)
		return false;
	//check if node has been found
	if (value == node->data) {
		remove_node(node, parent);
		return true;
	}
	//traverse either left or right
	if (value < node->data)
		return remove(node->left, node, value);
	else
		return remove(node->right, node, value);
}

// remove node and relink if necessary, using original logic
// from above. Passed a node to be removed and the node's parent
// Assumes parent==NULL means the node is the root, and sets the
// root to NULL. Must pass node to be removed and the parent of
// the node to be removed.
void IntBST::remove_node(Node *node, Node *parent) {
	Node *temp;
	int value = node->data; //just easier than having to pass
	
	//more than one count - just decrement
	if (node->count > 1) {
		node->count--;
	//check if node is leaf - simple delete
	} else if (node->left == NULL && node->right == NULL) {
		delete node;
		//replace with NULL
		link_parent(NULL, parent, value);
	//check if node only has one child - delete & re-link
	} else if (node->left == NULL || node->right == NULL) {
		//find which side needs to get relinked
		//set temp to the node that needs to get relinked
		if (node->left != NULL)
			temp = node->left;
		else
			temp = node->right;
		//now delete and relink the child to node's parent
		delete node;
		link_parent(temp, parent, value);
	//node has 2 children
	} else {
		remove_with_two_child(node);
	}
}

// Removes a node in the tree that has two children. Only a
// pointer to the node itself must be passed. The node gets
// the minimum of it's right side, then the min is removed
// in order to ensure invariant property of the BST. Additional
// details regarding this algorithm in comment of remove(int)
// function.
void IntBST::remove_with_two_child(Node *node) {
	Node *rightMin;
	//set temp to the right min
	rightMin = find_min(node->right);
	//Now complete the linkage
	if (node->right == rightMin) {
		//node is parent of temp. Linkage is easy!
		node->right = rightMin->right;
	} else {
		//otherwise need to use find_parent to find the parent
		//then use linkParent
		link_parent(rightMin->right, find_parent(node->right,
				    rightMin), node->data);
	}
	//replace node data with temp then delete temp
	node->data = rightMin->data;
	delete rightMin;
}

// Given a parent, a newNode is added as a child to the parent
// based off whether or not the value passed is greater or less
// than the parent node. Used for linking a parent to NULL or
// to a child in the case that a node is deleted which is either
// a leaf (in which case newNode is passed as NULL) or it only
// has one child (in which case newNode is passed as the chil
// to be linked.
void IntBST::link_parent(Node *newNode, Node *parent, int value) {
	//check if parent is null, i.e. the root was removed
	if (parent == NULL)
		root = newNode;
	//otherwise, either parent's left or right gets newNode
	else if (value < parent->data)
		parent->left = newNode;
	else if (value > parent->data)
		parent->right = newNode;
	else
		throw exception(); //this shouldn't happen
}

// Returns the maximum distance from the root to it's farthest
// leaf. NB: An empty tree has a height of -1; a tree with
// just a root has a height of 0.
int IntBST::tree_height() const
{
	if (root == NULL) //check empty tree
		return -1;
		
	return tree_height(root);
}

// recursive helper overload of tree_height with node that is
// being traversed as the paramater. Similar to node
// counter but tail call uses max. NB exponential asymptotic
// complexity...!
int IntBST::tree_height(Node *node) const
{
	if (node == NULL || (node->left  == NULL &&
						 node->right == NULL))
		return 0;

	return 1 + max(tree_height(node->left),
				   tree_height(node->right));
}

// Returns the total number of nodes (NOT including duplicates)
// i.e., if a tree contains 3, 4, 4, the number of nodes would
// only be two, because the fours are contained in one node.
int IntBST::node_count() const
{
	return node_count(root);
}

// Recursive helper for node_count gets passed node that is
// being counted in traversal. NB Exponential asymptotic complexity
int IntBST::node_count(Node *node) const
{
    if (node == NULL)
		return 0;

	return 1 + node_count(node->left) + 
			   node_count(node->right);
}

// Returns the sum of all the node values (including duplicates)
int IntBST::count_total() const
{
	return count_total(root);
}

// Recursive helper for count_total gets passed node that is
// being added. Tail call creates exponential asymptotic
// complexity. Simple multiplication used to calculate total.
int IntBST::count_total(Node *node) const
{
    if (node == NULL)
		return 0;
	
	return (node->data * node->count) + 
		    count_total(node->left) + count_total(node->right);
}



// Find_parent (written for you) is necessary for removing
// the right_min of a node when removing a node that has
// two children
Node *IntBST::find_parent(Node *node, Node *child) const
{
	if (node == NULL) return NULL;

	// if either the left or right is equal to the child,
	// we have found the parent
	if (node->left==child or node->right == child) {
			return node;
	}

	// Use the binary search tree invariant to walk the tree
	if (child->data > node->data) {
		return find_parent(node->right, child);
	} else {
		return find_parent(node->left, child);
	}
}
