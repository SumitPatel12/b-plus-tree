B+ Tree is a blanced n-ary tree. Every path from the root to any of the leaf noeds is of the same length. In these kind of trees every non-leaf node except the root node will have between ceil(n/2) and n children - n-1 being the maximun number of keys a node can accommodate - the roote node can have between 2 and n children.

## Structure of a B+ Tree
A node in a B+ Tree can have upto *n* pointers pointing to *n* children and that node can contain upto *n-1* keys.
![B+ Tree Node Structure](<B+ Tree Node Structure.png>)

Generally for the leaf nodes the pointers would either point to the page on disk or it'd directly contain the data that the key would be pointing to.
Each node is allowed to have between **ceil((n-1)/2) and n-1 keys**, n being the maximum children/pointers a node can have. **NOTE: The value of N is constance for a B+ Tree.**

Invariants:
- For two keys K<sub>i</sub> and K<sub>j</sub>, and i < j then K<sub>i</sub> < K<sub>j</sub>.
  - There is no equals contraint for a reason, you can have duplicates if you want but that's not realy suggested as it can lead to duplicate keys in the internal nodes making insertions and deletions more complicated.
- For two leaf-nodes L<sub>i</sub> and L<sub>j</sub>, and i < j, then every search key value v<sub>i</sub> in L<sub>i</sub> is less than every search key value v<sub>j</sub> in L<sub>j</sub>.
- The above goes for branch nodes as well.
- Unlike leaf nodes that contain data or pointers to the required data, non-leaf nodes point to other tree nodes. They act like a sparse index on top of the leaf nodes.
- **A leaf node must always have at least ceil((n - 1) / 2) values; n being the maximum number of pointers a node can accommodate.**
- **A non-leaf node may hold upto n pointers and always hold at least ceil(n/2) pointers.** This is important to note, the invariant is on pointers not the number of keys.
- For a non-leaf node with m pointers (where m <= n):
  - For 2 <= i <= m - 1, the pointer P<sub>i</sub> points to the subtree that contains key values less than K<sub>i</sub> and greater than equal to K<sub>i-1</sub>.
  - P<sub>1</sub> points to the subtree that contains values less than K<sub>1</sub>.
  - P<sub>m</sub> points to the subtree that contains values greater than K<sub>m-1</sub>.
- The root node can have fewer than ceil((n-1)/2) pointers. It however must at least have two pointers or none at all i.e. it will have at least two children or the root node will be the only node in the tree at that time. It cannot have one pointer.


## Querying a B+ Tree
A function that wants to query anything from the B+ Tree would generally start from the root node and travel downwards until it finds what its looking for in one of the leaf nodes, or end the query if the requested data was not present in any of the leaf nodes.

How the query works, say it's a point query for finding the value v:
- The function starts at the root node, and searches for a key such that K<sub>i</sub> >= v. 
  - If such a key is found: 
    - And K = v, then we move to the node pointed to by P<sub>i+1</sub>, this is because the pointers to the left of the key point to a subtree that has values strictly less than the succeeding key.
    - And K > v, then we move to the node pointed to by P<sub>i</sub>
  - If such a key was not found that means the value is greater than the largest key present in the root node, so we move to the node pointed to by P<sub>m</sub>, i.e. the rightmost child of the root.
- The above process repeats until you reach a leaf node. Then depending on if the leaf node contains the value v, we return the index/data of the node or return a failure indicating that the value was not found in the tree.

The pseudocode would look something like this
```py
def find(v):
    # Assuming no duplicate keys exist of course.
    let cur_node = root_node
    while (cur_node != leaf_node)
      let key_index = smallest key such that K<sub>i</sub> >= v
      if key_index exists:
        if keys[key_index] == v:
          cur_node = child[key_index + 1]
        else
          cur_node = child[key_index]
      else:
        # Key does not exist so we go the last node
        cur_node = child[last_pointer_index]
        
    # Now we've reached a leaf node
    if (for some i, key[i] == v):
      return value[i] or pointer[i]
    else:
      return null
```

For range queries, looking like v ∈ [lb, ub]; lb = lower_bound and ub = upper_bound:
- The serach will look similar to point query. The search will be for lb.
- The additional process will be that we don't terminate the search if lb was not found. We traverse all keys in the leaf node we reached and it's right side siblings till we reach a key such that the value of that key is greater than to the ub. We collect all the pointers in between this traversal.

The pseudocode for this is just adding some scanning at the end of the previous one. I'll not write that down.:shrug:


## Updating B+ Trees
### Insertion
We'd use the find functionality to get to the leaf node where the value to be inserted should be present. Then we insert the value in the node, preserving the ordering.
If the node was full we'd need to split the node in two to accommodate the new value.

The alogrithm dictates that when a node splits one key is added to the parent, and if that parent was full, we'd have to split the parent as well. This process can cascade all the way up to the root node (the root node can also split, when that happens, the depth of the tree is increased).

When a non-leaf node splits
- Both the keys as well as child pointers are split between nodes.
- If the max key capacity was n, then ceil(n/2) keys remain with the left-most node, 1 key is moved to the parent and the remaining keys move to the newly created node. The pointers associated with the node that moved to the parent still remain on the same level, they are just distributed among the nodes.

Pseudocode:
```py
def insert(key K):
    if (tree_empty):
        create leaf node with key
        set created node as root
    else
        L = find leaf node where K should be inserted
        
    if (L.Keys.length < n - 1):
        insert_key_in_leaf(L, K, P)
    else:
        L2 = new_node()
        ## Assuming a linked list like structure for leaf nodes the last pointer points to their right sibling.
        L2.pointer[n - 1] = L1.pointer[n - 1]
        L1.pointer[n - 1] = L2
        Keep the first ceil(n / 2) keys in L
        Move the remaining keys (starting at index ceil(n / 2) + 1) to L2
        # K1 is the smallest key in the newly formed node.
        # Since the parent needs to know the left and right child of the newly formed node we pass L and L2 along to the function
        insert_key_in_parent(L, K1, L2)
        
def insert_key_in_leaf(Node L, Key K, Pointer P):
    if K < K1:
        Insert K at position 1 and shift all other keys by one.
    else:
        Insert P, K into L after the key Ki such that Ki <= K
        
def insert_key_in_parent(Node N, Key K, Node N2):
    if (N == root):
        R = new_node();
        R has only one key and two pointers: pointer to N, Key K, pointer to N2
        set_root(RA)
    else:
        if (p.pointers.len < n):
            P = parent_of(N)
            # Insert into the parent a key just after N
            insert_key_to_node(index of N, K, N2);
        else:
            # Since it's overflowing we'll need to copy the contents of P along with K, N2 to some temporary space.
            T = copy_node_with_capacity(P, n + 1)
            Insert (K, N2) just after N in T
            # Order of keys could've changed after insertions so delete all entries from P and move entreis from T
            Remove all entries from P
            Move T.P1...T.P(ceil((n + 1) / 2)) to P
            # The key between T.P(ceil((n+1)/2))
            K_new = T.Key[ceil((n + 1) / 2)]
            P2 = new_node()
            Move 
            insert_key_in_parent(P, K_new, P2)
```

### Deletion
Well ofcouse like all other cases, we first locate the leaf-node that contains the desired value. If it doesn't contain the value, we just return deleted or value does not exist, preference for return value is left to the user.

Now if we did find the value we delete that value from the leaf-node and then consider merging if necessary. Merging should occur if after deletion the leaf-node is left with **less than ceil((n-1)/2) keys.** Where n is the maximum pointers a node can hold. **Merges occur with the left sibling node.** After merge is done we must delete the pointer in parent that pointed to the now deleted node. After deletion the parent may underflow as well, so we need to check if it needs to be merged.

One more thing to note is that after deletion we might not always be able to merge, it might be possible that the combined total of pointers of the nodes (node to be merged and the sibling with whom it is be to be merged) would exceed the maximum allowed limit for the node, in which case we'd need to redistribute the pointers between the node and it's siblings.

Another case to consider is that when merging nodes who are direct children of the root node. We have an invariant that says: `Root node is either the only node in the tree or it has at least two children`. If we merge the only two children of the root node we compromise that invariant, so that case needs to be handled as well.

```py
def delete_key(Key K):
    """Delete key K from the B+ tree"""
    L = find_leaf(K)
    
    if K not in L:
        return "Key not found"
    
    # Remove the key from the leaf node
    remove K from L
    
    # Special case: if L is the root and is now empty
    if L == root and L.numKeys == 0:
        delete L
        root = None
        return "Success"
    
    # Check if L needs rebalancing (has fewer than minimum keys)
    if needs_rebalancing(L):
        handle_underflow(L)
    
    return "Success"


def needs_rebalancing(Node N):
    """Check if node N has too few entries after deletion"""
    # Root can have fewer entries
    if N == root:
        if N.isLeaf():
            return False  # Leaf root can be sparse
        else:
            # Internal root needs at least 1 key (2 children)
            return N.numKeys == 0
    
    # Non-root nodes must maintain minimum occupancy
    if N.isLeaf():
        # Leaf must have at least ceil((n-1)/2) keys
        return N.numKeys < ceil((n - 1) / 2)
    else:
        # Internal node must have at least ceil(n/2) pointers
        # Which means at least ceil(n/2) - 1 keys
        return (N.numKeys + 1) < ceil(n / 2)


def handle_underflow(Node N):
    """Handle underflow by redistributing or merging"""
    P = parent(N)
    
    # Special case: root with only one child
    if N == root and not N.isLeaf() and N.numKeys == 0:
        # Root has only one child, make it the new root
        root = N.pointers[0]
        delete N
        return
    
    # Get a sibling (prefer left sibling for consistency)
    left_sibling = get_left_sibling(N, P)
    right_sibling = get_right_sibling(N, P)
    
    # Determine which sibling to use and get separator key
    if left_sibling is not None:
        sibling = left_sibling
        sibling_is_left = True
        K_sep = separator key in P between left_sibling and N
    else:
        sibling = right_sibling
        sibling_is_left = False
        K_sep = separator key in P between N and right_sibling
    
    # Check if merge is possible (combined entries fit in one node)
    if can_merge(N, sibling):
        merge_nodes(N, sibling, K_sep, sibling_is_left, P)
    else:
        # Redistribute entries between N and sibling
        redistribute(N, sibling, K_sep, sibling_is_left, P)


def can_merge(Node N, Node sibling):
    """Check if N and sibling can fit in a single node"""
    if N.isLeaf():
        # For leaf nodes, just check if combined keys fit
        return N.numKeys + sibling.numKeys <= (n - 1)
    else:
        # For internal nodes, need space for separator key from parent
        return N.numKeys + sibling.numKeys + 1 <= (n - 1)


def merge_nodes(Node N, Node sibling, Key K_sep, bool sibling_is_left, Node P):
    """Merge N and sibling into a single node"""
    # Normalize: always merge right node into left node
    if sibling_is_left:
        left_node = sibling
        right_node = N
    else:
        left_node = N
        right_node = sibling
    
    if left_node.isLeaf():
        # Merge leaf nodes
        # Copy all entries from right_node to left_node
        append all (key, data) pairs from right_node to left_node
        # Update sibling pointer: left_node now points to right_node's right sibling
        left_node.right_sibling = right_node.right_sibling
    else:
        # Merge internal nodes
        # Pull down separator key from parent
        append K_sep to left_node
        # Copy all keys and pointers from right_node to left_node
        append all keys from right_node to left_node
        append all pointers from right_node to left_node
    
    # Delete the separator key and pointer to right_node from parent
    delete_entry(P, K_sep, right_node)
    delete right_node
    
    # Parent might now underflow, check recursively
    if needs_rebalancing(P):
        handle_underflow(P)


def redistribute(Node N, Node sibling, Key K_sep, bool sibling_is_left, Node P):
    """Redistribute entries between N and sibling to fix underflow"""
    
    if sibling_is_left:
        # Borrow from left sibling
        if N.isLeaf():
            # Take the last entry from left sibling
            last_idx = sibling.numKeys - 1
            borrowed_key = sibling.keys[last_idx]
            borrowed_data = sibling.dataPointers[last_idx]
            remove last entry from sibling
            
            # Insert borrowed entry as first entry in N
            insert (borrowed_key, borrowed_data) as first entry in N
            
            # Update separator in parent to N's new first key
            replace K_sep in P with N.keys[0]
        else:
            # Internal node: borrow last pointer and key from left sibling
            last_key_idx = sibling.numKeys - 1
            borrowed_key = sibling.keys[last_key_idx]
            borrowed_pointer = sibling.pointers[sibling.numKeys]  # last pointer
            remove last key and pointer from sibling
            
            # Bring down K_sep from parent as first key in N
            # Insert borrowed pointer as first pointer in N
            insert K_sep as first key in N
            insert borrowed_pointer as first pointer in N
            
            # Push up borrowed_key to parent as new separator
            replace K_sep in P with borrowed_key
    else:
        # Borrow from right sibling
        if N.isLeaf():
            # Take the first entry from right sibling
            borrowed_key = sibling.keys[0]
            borrowed_data = sibling.dataPointers[0]
            remove first entry from sibling
            
            # Insert borrowed entry as last entry in N
            insert (borrowed_key, borrowed_data) as last entry in N
            
            # Update separator in parent to sibling's new first key
            replace K_sep in P with sibling.keys[0]
        else:
            # Internal node: borrow first pointer and key from right sibling
            borrowed_key = sibling.keys[0]
            borrowed_pointer = sibling.pointers[0]
            remove first key and pointer from sibling
            
            # Bring down K_sep from parent as last key in N
            # Insert borrowed pointer as last pointer in N
            insert K_sep as last key in N
            insert borrowed_pointer as last pointer in N
            
            # Push up borrowed_key to parent as new separator
            replace K_sep in P with borrowed_key


def delete_entry(Node P, Key K, Node child_to_delete):
    """Delete key K and pointer to child_to_delete from parent P"""
    remove K from P
    remove pointer to child_to_delete from P
```

## References
Most of it is form the Database Systems Concepts book, and some from Modern B Tree Techniques.
