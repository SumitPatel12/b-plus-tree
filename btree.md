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
def delete_key(Key K, Pointer P):
    L = find(K)
    if (N == root && N has only one child):
        Make child of N the root and delete N
    else if (deleting from N leaves it with too few pointers or keys):
        sibling = previous or next child of parent(N)
        K2 = value b/w pointers N and sibling in parent(N)
        if (entries of N and sibling fit in a single node):
            if (N is a predecessor of sibling): swap the pointers of both
            if (N != leaf_node):
                append K2 and all keys and pointers of N to sibling
            else:
                append all Key, Pointer pairs in N to sibling
                # The merged node points to the right sibling of the deleted node.
                sibling.pointers[n] = L.pointers[n]
            delete_key(parent(N), K2, sibling)
            delete_node(N)
        else:
            if (sibling is a predecessor of N):
                if (N != leaf_node):
                    last_pointer_idx = index of the last_pointer of sibling
                    remove(sibling.keys[last_pointer_idx - 1], sibling.pointers[last_pointer_idx])
                    insert the pair (sibling.pointers[last_pointer_idx], K2) as the first pointer and value in N
                    replace K2 in the parent(N) by sibling.keys[last_pointer_idx - 1]
                else:
                    last_pointer_idx = index of the last_pointer of sibling
                    last_key = sibling.pointers[last_pointer_idx]
                    remove(sibling.keys[last_pointer_idx], last_key)
                    insert the pair (sibling.keys[last_pointer_idx], last_key) as the first pointer and value in N
                    replace K2 in the parent(N) by last_key
            else:
                # Symmetric to the then case.
                # I planned to write this, whole thing but man if it's tardy.
```

## References
Most of it is form the Database Systems Concepts book, and some from Modern B Tree Techniques.
