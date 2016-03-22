#include "argparse.h"

const char *argp_program_version = "unrelyzer 2.0";
const char *argp_program_bug_address = "<dibyendu.das.in@gmail.com>";

/* Program documentation. */
static char doc[] = "unrelyzer -- analyzer for unreliable programs,\n\
that performs static value and interval analysis of any `C-like` unreliable program.\
\vThe two required arguments are:\n  FILE, which is the input source file and\n\
  FUNCTION, which is the function to be analyzed in the input file.\n\
PARAMS is an optional list of integers, each corresponding to one integer parameter\
 the FUNCTION has. The intergers are to be given in the same order as they appear in\
 the parameter list of FUNCTION.";

/* A description of the accepted arguments. */
static char args_doc[] = "FILE FUNCTION [PARAMS...]";

/* The accepted options. */ 
static struct argp_option options[] = {
  {"concrete",   'c', 0,        0,                   "Analyse the program to produce concrete/discrete values" },
  {"abstract",   'a', 0,        0,                   "Analyse the program to produce abstract (range of) values" },
  {"parse",      'p', 0,        0,                   "Generate the 'Parse Tree' in a file called 'parse.dot'" },
  {"ast",        's', 0,        0,                   "Generate the 'Abstract Syntax Tree' in a file called 'ast.dot'" },
  {"cfg",        'f', 0,        0,                   "Generate the 'Control Flow Graph' in a file called 'cfg.dot'" },
  {"verbose",    'v', 0,        0,                   "Produce verbose output" },
  {"debug",      'd', 0,        OPTION_ALIAS },
  {"output",     'o', "FILE",   0,                   "Output to FILE instead of standard output" },
  {"iteration",  'i', "COUNT",  OPTION_ARG_OPTIONAL, "Iterate through the 'Data Flow Equations' maximum COUNT (default 20) number of times, to reach fixed point"},
  {0,             0,  0,        0,                   "The following option can only be used along with option -a (or --abstract):" },
  {"widening",   'w', 0,        0,                   "Use 'widening' technique to accelerate convergence (reduced number of \
iterations through the Iterate through the Data Flow Equations) of the abstract result" },
  { 0 }
};

static error_t parse_opt(int key, char *arg, struct argp_state *state) {
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
    case 'i':
      arguments->N_iteration = arg ? (atoi(arg) > 0 ? atoi(arg) : 20) : 20;
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num == 0)
        arguments->input = arg;
      else if (state->arg_num == 1)
        arguments->function = arg;
      else
        return ARGP_ERR_UNKNOWN;
      break;
    case ARGP_KEY_ARGS:
      arguments->params = &state->argv[state->next];
      arguments->N_params = state->argc - state->next;
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 2 || arguments->widening && !arguments->abstract)
        argp_usage(state);
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

void parse_arguments(int argc, char **argv, Arguments *arguments) {
  argp_parse(&argp, argc, argv, 0, 0, arguments);
}