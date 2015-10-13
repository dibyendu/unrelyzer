# Control-Flow-Graph
Parses any language defined by a subset of `Mini-C` grammar and generates **Parse-Tree**, **Abstract-Syntax-Tree** and **Control-Flow-Graph**.

###Usage
1. `./configure`
  * It'll check for existing `flex` and `bison` installations in the system. Otherwise compile from the source shipped with the project and install in a directory called `binary`.
2. `make`
  * Build the project and places the executable in a directory called `build`.
3.  `cd build`
4. `./parser <input-file>`
5. `dot -Tpng <filename>.dot -o <filename>.png`
  * `Graphviz` is required for this step.
6. `cd ..`
7. `make clean`
  * To clean everything.
