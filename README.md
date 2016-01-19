# Probabilistic-Range-Analysis
The tool resides in the directory named `code`. It parses any language defined by a subset of `Mini-C` grammar. Each arithmatic and boolean operation in the program is probabilistic/unreliable. This tool statically analyze the program using **Abstract Interpretation** and displays result of the analysis on standard output. As byproduct it also generates **Parse-Tree**, **Abstract-Syntax-Tree** and **Control-Flow-Graph** for the language.

###Usage
####Build
1. `cd code`
2. `./configure`
  * It'll check for existing `flex` and `bison` installations in the system. Otherwise compile from the source shipped with the project and install in a directory called `binary`.
3. `make`
  * Builds the project inside `src` itself.
4. `make clean`
  * To clean everything.

####Run
4.  `cd code/src`
5. `./analyzer [OPTION]... [FILE]`
  * Statically analysis `FILE`, generates the graphs in form of `dot` files and displays the result on standard output.
  * 
  * `-c, --concrete` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; analysis the `[FILE]` in the concrete domain
  * `-w, --widening` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; analysis in the abstract domain uses `widening` operation to accelerate convergence
  * `-p, --parse-tree` &nbsp;&nbsp;&nbsp;&nbsp; generates the `Parse Tree` in a file called `parse-tree.dot`
  * `-a, --ast`  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; generates the `Abstract Syntax Tree` in a file called `ast.dot`
  * `-f, --cfg`  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; generates the `Control Flow Graph` in a file called `cfg.dot`
6. `dot -Tpng [FILE].dot -o [FILE].png`
  * `Graphviz` is required for this step.