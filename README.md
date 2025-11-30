# String Cheese

Whoever invented the foodstuff known as string cheese is either a genius or a madman.

## Known Issues

Apart from being incomplete:

* No auto-balancing
* Still using malloc & free
* Returning a ptr to a local var is bad

## Random Thoughts

* An arena can contain multiple buckets, one for each size of memory allocation
* One bucket can also hold *just* the rope nodes
* The strings need a dedicated allocation system, divided by string lengths

## Needed features:

* allocate/release
* split
* concat
* append - concat(a, b)
* insert - split, concat, concat
* delete - split, split, concat
* auto-balancing - children should be balanced first
* rotate left
* rotate right