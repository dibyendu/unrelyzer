# Probabilistic-Range-Analysis
The tool resides in the directory named `code`. It parses any language defined by the [`Mini-C`](http://jamesvanboxtel.com/projects/minic-compiler/minic.pdf#page=2 "Mini-C grammar rules") grammar. Each arithmetic and boolean operation in the program is probabilistic/unreliable. This tool statically analyze the program using **Abstract Interpretation**. As byproduct it also generates **Parse-Tree**, **Abstract-Syntax-Tree** and **Control-Flow-Graph** for the program.

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
1.  `cd code/src`
2. `./unrelyzer --help`
  * Shows all the options and required arguments to run this tool.

> Some of the primary options are listed below :

> `-a, --abstract` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Analyse the program to produce abstract (range of) values

> `-c, --concrete` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Analyse the program to produce concrete/discrete values

> `-p, --parse-tree` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Generate the **Parse Tree** in a file called `parse.dot`

> `-s, --ast` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Generate the **Abstract Syntax Tree** in a file called `ast.dot`

> `-f, --cfg` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Generate the **Control Flow Graph** in a file called `cfg.dot`

> `-i, --iteration[=COUNT]` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Iterate through the `Data Flow Equations` maximum COUNT (default 20) times

> `-l, --column[=COUNT]` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Display output in COUNT (default 1) column(s) of variables

> `-o, --output=FILE` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Output to FILE instead of standard output

> `-v, -d, --verbose, --debug` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Produce verbose output

> > The following option can only be used along with option `-a` (or `--abstract`):

> `-w, --widening` &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; Use **widening** technique to accelerate convergence (reduced number of iterations through the Iterate through the Data Flow Equations) of the abstract result

For example, suppose the following C code is stored in a file named `input`:
```c
int limit = 10;

int foo(int num) {
  while (num < limit)
    num = num + 3;
  return 0;
}
```
To analysis the function `foo` in concrete domain, the command would be `./unrelyzer -cf -l2 input foo num=0`. It generates the following *Control Flow Graph* followed by the analysis result:

![Control Flow Graph](http://i.imgur.com/X4E8hzD.png)

```c
G₁ =  ⊥
G₃ =  sp(G₁, limit=10)
G₄ =  sp(G₃, num=0) ⊔  sp(G₅, num=(num+3))
G₅ =  sp(G₄, num<limit)
G₇ =  sp(G₄, num>=limit)

In the following result   m = -32768  & M = 32767

---------------------- Result of Concrete Analysis (65 clock ticks) ----------------------
Reached fixed point after 6 iterations

G₁  ::
    limit=<{a∈ ℤ | m ≤ a ≤ M}, 1>    num=<{a∈ ℤ | m ≤ a ≤ M}, 1>
G₃  ::
    limit=<{10}, 0.999999899998>     num=<{a∈ ℤ | m ≤ a ≤ M}, 1>
G₄  ::
    limit=<{10}, 0.999999899998>     num=<{0 3 6 9 12}, 0.999998499987>
G₅  ::
    limit=<{10}, 0.999999749998>     num=<{0 3 6 9}, 0.99999869999>
G₇  ::
    limit=<{10}, 0.999999749998>     num=<{12}, 0.999998349988>
```

####Visualization of graphs
1. `dot -Tpng [FILE].dot -o [FILE].png`
  * Converts the graphs from `.dot` format to `.png` image format.
  * [`Graphviz`](http://www.graphviz.org "Graph Visualization Software") is required for this step.
2. alternatively, an online service like http://graphviz.herokuapp.com, can also be used.

####Clean
1. `make clean`
  * To clean up everything.
 
###Licensing
This code is released under  [GNU General Public License (Version 3)](http://www.gnu.org/licenses/gpl-3.0.en.html "GPLv3"), a copy of which is also shipped with the software. ![GPLv3][gpl3]

[gpl3]: http://www.gnu.org/graphics/gplv3-127x51.png  "GPLv3 Logo"