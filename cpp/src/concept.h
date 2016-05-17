template< typename Data, typename Level = uint8_t >
struct depth_tree : public std::vector<std::pair< Level, Data>>
{};

template< typename Data>
struct breadth_tree : public std::vector<std::pair< size_t, Data>>
{};

/*
linearized trees

depth first
===========
- each node knows its level

find end of node subtree
- next node with equal level

find parent node
- previous node with smaller level

insert subtree before it
- move it .. end to make space
- adjust levels in subtree

remove subtree
- ensure only complete subtrees are removed

breadth first
=============
- each node knows its parent index

find first child
- first node with parent index == current

insert subtree before it
- insert parent before self
- find node with parent index
  - insert children
  - adjust parent indexes

*/
