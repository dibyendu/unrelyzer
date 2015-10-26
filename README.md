# Probabilistic-Range-Analysis
The tool resides in the directory named `code`. As of now it parses any language defined by a subset of `Mini-C` grammar and generates **Parse-Tree**, **Abstract-Syntax-Tree** and **Control-Flow-Graph**.

###Usage
1. `cd code`
2. `./configure`
  * It'll check for existing `flex` and `bison` installations in the system. Otherwise compile from the source shipped with the project and install in a directory called `binary`.
3. `make`
  * Build the project inside `src` itself.
4.  `cd src`
5. `./parser <input-file>`
6. `dot -Tpng <filename>.dot -o <filename>.png`
  * `Graphviz` is required for this step.
7. `cd ..`
8. `make clean`
  * To clean everything.
