# Vector Tree

*A collection of vector based tree implementations.*

Classical Trees are implemented like a double linked list with links to parents and children.

Vector based trees store all nodes in a vector or at least all children of on node in a vector.

These kind of vector based trees should be explored in more detail.

## Drift Tree

The entire tree is stored in a single continous vector. Each node stores a level drift.

Properties:
* [x] efficient storage
* [x] very fast depth first iteration for any subtree
* [x] efficient for building with append operation
* [x] easy to read and understand
* [ ] insert requires the vector to move elements
* [ ] same level siblings have to be searched

## License

Apache License Version 2.0
See LICENSE file for more details

## Contributions

This project is open for pull requests.
Bug fixes and new tree implementations are welcome!
