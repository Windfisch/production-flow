The output explained
====================

The program generates a lot of verbose debugging output when running. In this
document, we dissect and explain the output.

All graphs, recognizable by the surrounding `digraph ... { ... }`, can be drawn
using the [graphviz](http://www.graphviz.org/) tool **dot**. E.g. by pasting the
graph data into `dot -Tpng | display -`.


Initialisation
--------------

The program reads the `.tgf`-file, parsing all nodes (facility-recipes) and
edges (transport lines):

```
recipe 'coal' with 7/15
recipe 'coal' with 5/15
<...>
recipe 'circuit' with 0/20
transport of iron-ore 3->4, distance 0.3 (line exists=1)
transport of coal 1->4, distance 2.5 (line exists=1)
<...>
transport of copper-plate 31->11, distance 2 (line exists=1)
```

Finding the cheapest upgrade
----------------------------

```
openlist has size 1, total expanded = 10
inspecting item: 1, nodes: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0, edges: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
```

We can see the number of nodes already expanded, and number of nodes in the
openlist of the Dijkstra algorithm. Also, the currently inspected node in the
actiongraph is dumped:
- `item: 1`: the current item is 1 (i.e. `IRON_ORE`)
- `nodes: 0 0 0 ...`: the current upgrade levels for the facilities
- `edges: 0 0 0 ...`: same for transport lines



After that, we simulate the flow for this particular item and output the
flowgraph in the dot file format:

```
simulating flow for current item type 1
digraph "FINAL" {
	0 [label="0in, 13300/15000prod\n13300avail, 0exc"];
	1 [color=red,label="13300in, -13300/-15000prod\n0avail, 0exc"];
	2 [label="0in, 0/0prod\n0avail, 0exc"];
	3 [label="0in, 0/0prod\n0avail, 0exc"];

	0 -> 1 [label="13300/13300(13300)"];
	2 -> 3 [label="0/13300(13300)"];
}
```

Finally, we list all successor nodes reachable from this one (i.e.: all
possible upgrades of upgradeable facilities or edges, or -- in case of a valid
flow -- considering the next item):

```
  -> successor item: 1, nodes: 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0, edges: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0; not seen yet, adding to openlist
  -> successor item: 1, nodes: 0 0 0 0 0 0 0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0, edges: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0; already in closedlist
  -> successor item: 1, nodes: 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0, edges: 1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0; not seen yet, adding to openlist
```

This kind of message is repeated a lot, for each node the Dijkstra visits.


Node/Edge map
-------------

Then the `Factory::facility_toposort` and `Factory::edge_table_per_item` lists
are dumped. They denote the mappings from node/edge numbers in the flow graphs
to node/edge numbers in the factory. (And by this, also in the `.tgf`-file.
Note that the `.tgf`-file is 1-based-indexed, i.e. you must increment all
numbers after the `->`):

```
it follows the node/edge map. for item i, a->b means that flowgraph-node/edge a
corresponds to factory-facility/transportline b. Note that b is given as zero
indexed number, but the .tgf file is one-based-indexed. Add 1 before looking it up
item 0: facilities 0->0, 1->1, 2->21, 3->9, 4->7, 5->5, 6->3, 7->20, transport lines 0->1, 1->3, 2->16, 3->17, 4->18, 5->20, 6->36, 
item 1: facilities 0->2, 1->3, 2->8, 3->9, transport lines 0->0, 1->19, 
<...>
```


Before/After graph of the entire factory
-----------------------------------------

```
now simulating the entire factory in its before-state
```

First, all flow graphs for the single items are dumped:

```
digraph "FINAL" {
	0 [label="0in, 5000/7000prod\n5000avail, 0exc"];
	1 [label="0in, 0/5000prod\n0avail, 0exc"];
	<...>
	7 [label="0in, 0/0prod\n0avail, 0exc"];

	0 -> 6 [label="3000/3000(13300)"];
	0 -> 5 [label="2000/2000(13300)"];
	<...>
	0 -> 2 [label="0/0(13300)"];
}
digraph "FINAL" {
	<...>
}
<...>
```

Then, the factory as a whole is dumped:

```
digraph "factory" {
	0 [label="coal:5000/7000, "];
	1 [label="coal:0/5000, "];
	<...>
	30 [label=""];

	2 -> 3 [label="iron-ore: 13300/13300"];
	0 -> 3 [label="coal: 3000/13300"];
	<...>
	30 -> 10 [label="copper-plate: 500/13300"];
}
```

Same is done for the factory state after optimisation:

```
now simulating the entire factory in its after-state
<...>
digraph "factory" {
	<...>
}
```

