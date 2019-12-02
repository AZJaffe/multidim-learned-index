'''
Each row in a csv file is an item
Every BTreeNode stores a number of item objects
'''
class Item():

    def __init__(self, key, value):
        self.k = key
        self.v = value
    
    # get printable item
    def __str__(self):
        return "Key: " + str(self.k) + " Value: " + str(self.v)

    # implement comparators
    def __eq__(self, other):
        return self.k == other.k
    
    def __gt__(self, other):
        return self.k > other.k
    
    def __ge__(self, other):
        return self.k >= other.k
    
    def __lt__(self, other):
        return self.k < other.k
    
    def __le__(self, other):
        return self.k <= other.k


'''
parameters:
    degree: number of keys to half-fill the node
    numOfKeys: number of items in a node currently
    isLeaf: if a node is leaf node
    items: items in the node
    children: children of the node
    index: index of the node
'''
class BTreeNode:

    def __init__(self, degree=2, numOfKeys=0, isLeaf=True, items=None, children=None, index=None):
        self.degree = degree
        self.numOfKeys = numOfKeys
        self.isLeaf = isLeaf
        if items is not None:
            self.items = items
        else:
            self.items = [None] * (degree * 2 - 1)
        if children is not None:
            self.children = children
        else:
            self.children = [None] * degree * 2
        self.index = None
        return
    
    def set_index(self, index):
        self.index = index
    
    def get_index(self):
        return self.index
    
    # itemCount use to measure the cost of the search
    def search(self, bTree, item, itemCount):
        pos = 0
        itemCount += 1
        while pos < self.numOfKeys and item > self.items[pos]:
            pos += 1
            itemCount += 1
        # find the key
        if pos < self.numOfKeys and item == self.items[pos]:
            return {
                "found": True,
                "nodeIndex": self.index,
                "position": pos,
                "itemCount": itemCount
            }
        # Leaf node -> fail to find the key
        if self.isLeaf:
            return {
                "found": False,
                "nodeIndex": self.index,
                "position": -1,
                "itemCount": itemCount
            }
        # Nonleaf node -> search children
        else:
            return bTree._get_node(self.children[pos]).search(bTree, item, itemCount)

'''
parameters:
    degree: number of keys to half-fill the node
    rootIndex: index of the rootNode
    rootNode: root node of the tree
    freeIndex: an incremental counter pointing to next available index
    nodes: a map from index to node
'''
class BTree:

    def __init__(self, degree=2):
        self.nodes ={}
        self.degree = degree
        self.rootIndex = 1
        self.freeIndex = 2
        self.rootNode = BTreeNode(self.degree)
        self.rootNode.set_index(self.rootIndex)
        self._add_node(self.rootIndex, self.rootNode)        
    
    #################### Helper Functions #################
    def _set_root_node(self, rootNode):
        self.rootNode = rootNode
        self.rootIndex = self.rootNode.get_index()
    
    def _get_node(self, index):
        return self.nodes[index]
    
    def _add_node(self, index, node):
        self.nodes[index] = node

    def _get_free_index(self):
        self.freeIndex += 1
        return self.freeIndex - 1
    
    def _get_free_node(self):
        freeNode = BTreeNode(self.degree)
        freeIndex = self._get_free_index()
        freeNode.set_index(freeIndex)
        self._add_node(freeIndex, freeNode)
        return freeNode
    
    #################### Tree Operations #################
    def build(self, keys, values):
        assert(len(keys) == len(values))
        for i in range(len(keys)):
            self.insert(Item(keys[i], values[i]))
    
    def search(self, item):
        return self.rootNode.search(self, item, 0)

    def split_child(self, pNode, i, cNode):
        freeNode = self._get_free_node()
        freeNode.isLeaf = cNode.isLeaf
        freeNode.numOfKeys = self.degree - 1
        # split and copy second half of child node
        for j in range(0, self.degree-1):
            freeNode.items[j] = cNode.items[j + self.degree]
        if not cNode.isLeaf:
            for j in range(0, self.degree):
                freeNode.children[j] = cNode.children[j + self.degree]
        cNode.numOfKeys = self.degree - 1
        # split and shift second half of parent node
        j = pNode.numOfKeys + 1
        while j > i + 1:
            pNode.children[j+1] = pNode.children[j]
            j -= 1
        pNode.children[j] = freeNode.get_index()
        j = pNode.numOfKeys
        while j > i:
            pNode.items[j + 1] = pNode.items[j]
            j -= 1
        pNode.items[i] = cNode.items[self.degree - 1]
        pNode.numOfKeys += 1
    
    def insert(self, item):
        searchResult = self.search(item)
        if searchResult['found']:
            print(item.k)
            print("Duplicated Item! Should never happen")
            assert(False)
        r = self.rootNode
        if r.numOfKeys == 2 * self.degree - 1:
            freeNode = self._get_free_node()
            self._set_root_node(freeNode)
            freeNode.isLeaf = False
            freeNode.numOfKeys = 0
            freeNode.children[0] = r.get_index()
            self.split_child(freeNode, 0, r)
            self.insert_not_full(freeNode, item)
        else:
            self.insert_not_full(r, item)

    def insert_not_full(self, node, item):
        i = node.numOfKeys - 1
        if node.isLeaf:
            while i >= 0 and item < node.items[i]:
                node.items[i+1] = node.items[i]
                i -= 1
            node.items[i+1] = item
            node.numOfKeys += 1
        else:
            while i >= 0 and item < node.items[i]:
                i -= 1
            i += 1
            if self._get_node(node.children[i]).numOfKeys == 2 * self.degree - 1:
                self.split_child(node, i, self._get_node(node.children[i]))
                if item > node.items[i]:
                    i += 1
            self.insert_not_full(self._get_node(node.children[i]), item)