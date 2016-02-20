#include "argparse.h"

const char *argp_program_version = "unrelyzer 1.2";
const char *argp_program_bug_address = "<dibyendu.das.in@gmail.com>";

/* Program documentation. */
static char doc[] = "unrelyzer -- an unreliable program analyzer,\n\
that performs static value and interval analysis of any `C-like` unreliable program.";

/* A description of the accepted arguments. */
static char args_doc[] = "FILE";

/* The accepted options. */ 
static struct argp_option options[] = {
  {"concrete",   'c', 0,      0, "Analyse the program to produce concrete/precise values" },
  {"abstract",   'a', 0,      0, "Analyse the program to produce abstract range of values" },
  {"widening",   'w', 0,      0, "Use `widening' technique to accelerate convergence of the abstract result" },
  {"parse-tree", 'p', 0,      0, "Generate the `Parse Tree' in a file called `parse-tree.dot'" },
  {"ast",        's', 0,      0, "Generate the `Abstract Syntax Tree' in a file called `ast.dot'" },
  {"cfg",        'f', 0,      0, "Generate the `Control Flow Graph' in a file called `cfg.dot'" },
  {"verbose",    'v', 0,      0, "Produce verbose output" },
  {"debug",      'd', 0,      OPTION_ALIAS },
  {"output",     'o', "FILE", 0, "Output to FILE instead of standard output" },
  { 0 }
};

/* Parse a single option. */
static error_t parse_opt(int key, char *arg, struct argp_state *state) {
  /* Get the input argument from argp_parse, which we
     know is a pointer to our arguments structure. */
  Arguments *arguments = state->input;

  switch (key) {
    case 'd': case 'v':
      arguments->verbose = true;
      break;
    case 'c':
      arguments->concrete = true;
      break;
    case 'a':
      arguments->abstract = true;
      break;
    case 'w':
      arguments->widening = true;
      break;
    case 'p':
      arguments->parse_tree = true;
      break;
    case 's':
      arguments->ast = true;
      break;
    case 'f':
      arguments->cfg = true;
      break;
    case 'o':
      arguments->output = arg;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num >= 1) /* Too many arguments. */
        argp_usage (state);
      arguments->args[state->arg_num] = arg;
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 1) /* Not enough arguments. */
        argp_usage (state);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

/* Our argp parser. */
static struct argp argp = { options, parse_opt, args_doc, doc };

void parse_arguments(int argc, char **argv, Arguments *arguments) {
  argp_parse(&argp, argc, argv, 0, 0, arguments);
}