#ifndef __IntBST_h__
#define __IntBST_h__

struct Node {
    int   data; //value
    int   count;//number of times data has been inserted
    Node *left;
	Node *right;
		
		};

class IntBST {
	public:
		//original functions from the old homework
         IntBST();
        ~IntBST();
         IntBST            (const IntBST &source);
         IntBST &operator= (const IntBST &source);
         
        bool contains  (int value) const;
        bool remove    (int value);
        int tree_height() const;
        int node_count () const;
        int count_total() const;
        int find_min   () const;
        int find_max   () const;        
        void insert    (int value);
        
        //functions that I added for SharkBatch
        void print	   ();
        bool is_empty();
        std::vector<int> return_vector();
        
	private:

	        // the root of the tree (starts as NULL)
        Node *root;
        
        //helper vector (this is a bit of a hack so I can output a vector of the nums
        std::vector <int>vec;
        void return_vector(Node *node);
        void  print		 		   (Node *node);

		// Helper functions from original homework
        Node *find_min      	   (Node *node) const;		
        Node *find_max      	   (Node *node) const;		
        Node *pre_order_copy	   (Node *node) const;		
        Node *find_parent   	   (Node *node, Node *child) const;	
        bool  contains     	       (Node *node, int value) const;
		bool  remove        	   (Node *node, Node *parent, int value);
        int   tree_height   	   (Node *node) const;
        int   node_count    	   (Node *node) const;
        int   count_total   	   (Node *node) const;
        void  insert               (Node *node, Node *parent, int value);
        void  post_order_delete    (Node *node);
        void  remove_node          (Node *node, Node *parent);
        void  remove_with_two_child(Node *node);
        void  link_parent          (Node *newNode, Node *parent, int value);

        
        
};

#endif // __IntBST_h__
	