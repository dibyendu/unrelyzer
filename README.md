# Probabilistic-Range-Analysis
The tool resides in the directory named `code`. It parses any language defined by a subset of `Mini-C` grammar. Each arithmetic and boolean operation in the program is probabilistic/unreliable. This tool statically analyze the program using **Abstract Interpretation**. As byproduct it also generates **Parse-Tree**, **Abstract-Syntax-Tree** and **Control-Flow-Graph** for the program.

The type of programs it can analyse are similar to that of `C`. A specimen `input` file is shipped that states all the *necessary constraints* that any program parsable by this tool must follow. It also contains few sample code blocks.

Moreover, one file named `hardware_specification.h` is also there, that specifies correctness probabilities of the Arithmetic, Logical and Memory operations used in the language. The hardware manufacturer sets these probabilities.

###Usage
####Build
1. `cd code`
2. `./configure`
  * This step will check for existing `flex` and `bison` installations in the system. Otherwise compile from the source that is shipped with the project and install in a directory called `binary`.
3. `make`
  * Builds the project inside `src` itself.

####Run
4.  `cd code/src`
5. `./unrelyzer --help`
  * Shows all the options and required arguments to run this tool.
  * Some of the primary options are listed below :
  * `-a, --abstract` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Analyse the program to produce abstract range of values
  * `-c, --concrete` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Analyse the program to produce concrete/precise values
  * `-f, --cfg`  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Generate the `Control Flow Graph` in a file called `cfg.dot`
  * `-o, --output=FILE` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Output to FILE instead of standard output
  * `-p, --parse` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Generate the `Parse Tree` in a file called `parse.dot`
  * `-s, --ast`  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Generate the `Abstract Syntax Tree` in a file called `ast.dot`
  * `-v, -d, --verbose, --debug`  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Produce verbose output
  * `-w, --widening` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Use `widening' technique to accelerate convergence of the abstract result
6. `dot -Tpng [FILE].dot -o [FILE].png`
  * Converts the graphs from `.dot` format to `.png` image format.
  * `Graphviz` is required for this step.
7. `make clean`
  * To clean up everything.
