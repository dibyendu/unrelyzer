#include "argparse.h"

const char *argp_program_version = "unrelyzer 2.0";
const char *argp_program_bug_address = "<dibyendu.das.in@gmail.com>";

/* Program documentation. */
static char doc[] = "unrelyzer -- analyzer for unreliable programs,\n\
that performs static value and interval analysis of any 'C-like' unreliable program.\
\vThe two required arguments are:\n\
  FILE,\t\t\t\t\twhich is the input source file and\n\
  FUNCTION,\t\t\t\twhich is the function to be analyzed in the input file.\n\
PARAM=VALUE and PARAM=[VALUE,VALUE]\tare optional lists of parameters of the FUNCTION with integer \
values or range of values assigned to them. Each argument corresponds to one integer parameter \
of the FUNCTION. The PARAMs provided in this list need not be given in the same order as they \
appear in the FUNCTION.\n\n\
N.B: If concrete domain analysis is required (-c or --concrete option is provided), ALL \
parameters in the optional list of PARAMs MUST be provided with a single VALUE (instead \
of range of values i.e [VALUE,VALUE]).\n\n\
Examples:\n\
  unrelyzer -ca input collatz x=10\t\t\tAnalyze collatz function from file input, in both abstract and concrete domains, with its only parameter set to value 10.\n\
  unrelyzer -aw input collatz x=[-10,40]\t\tAnalyze collatz function from file input, in abstract domain using widening, with its only parameter set to range [-10,40].\n\
  unrelyzer -a input euclid_gcd b=-20 a=[-10,10]\tAnalyze euclid_gcd function from file input, in abstract domain, with parameters a set to range [-10,10] and b set to -20.\n\
  unrelyzer -ac input euclid_gcd a=35 b=7\t\tAnalyze euclid_gcd function from file input, in both abstract and concrete domains, with parameters a set to 35 and b set to 7.";

/* A description of the accepted arguments. */
static char args_doc[] = "FILE FUNCTION [PARAM=VALUE...] [PARAM=[VALUE,VALUE]...]";

/* The accepted options. */ 
static struct argp_option options[] = {
  { "concrete",   'c', 0,        0,                   "Analyse the program to produce concrete/discrete values" },
  { "abstract",   'a', 0,        0,                   "Analyse the program to produce abstract (range of) values" },
  { "parse-tree", 'p', 0,        0,                   "Generate the 'Parse Tree' in a file called 'parse.dot'" },
  { "ast",        's', 0,        0,                   "Generate the 'Abstract Syntax Tree' in a file called 'ast.dot'" },
  { "cfg",        'f', 0,        0,                   "Generate the 'Control Flow Graph' in a file called 'cfg.dot'" },
  { "verbose",    'v', 0,        0,                   "Produce verbose output" },
  { "debug",      'd', 0,        OPTION_ALIAS },
  { "output",     'o', "FILE",   0,                   "Output to FILE instead of standard output" },
  { "iteration",  'i', "COUNT",  OPTION_ARG_OPTIONAL, "Iterate through the 'Data Flow Equations' maximum COUNT (default 20) number of \
times, to reach fixed point"},
  { "column",     'l', "COUNT",  OPTION_ARG_OPTIONAL, "Display output in COUNT (default 1) column(s) of variables"},
  { 0,             0,  0,        0,                   "The following option can only be used along with option -a (or --abstract):" },
  { "widening",   'w', 0,        0,                   "Use 'widening' technique to accelerate convergence (reduced number of \
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
    case 'l':
      arguments->N_columns = arg ? (atoi(arg) > 0 ? atoi(arg) : 1) : 1;
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
      arguments->value_params = (long **) calloc(arguments->N_params, sizeof(long *));
      arguments->interval_params = (long **) calloc(arguments->N_params, sizeof(long *));
      size_t i;
      for (i = 0; i < arguments->N_params; ++i) {
        char *offest = strstr(arguments->params[i], "=");
        if (!offest || !(offest + 1)[0] || strstr(offest + 1, "="))
          argp_error(state, "Malformed optional argument ‘%s’", arguments->params[i]);
        if ((offest + 1)[0] == '[') {
          has_interval_param = true;
          arguments->interval_params[i] = (long *) calloc(2, sizeof(long));
          if (sscanf(offest + 1, "[%ld,%ld]", &arguments->interval_params[i][0], &arguments->interval_params[i][1]) != 2)
            argp_error(state, "Malformed optional argument ‘%s’", arguments->params[i]);
        }
        else {
          arguments->value_params[i] = (long *) calloc(1, sizeof(long));
          if (sscanf(offest + 1, "%ld", arguments->value_params[i]) != 1)
            argp_error(state, "Malformed optional argument ‘%s’", arguments->params[i]);
        }
        *offest = 0;
      }
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 2)
        argp_error(state, "Atleast 2 arguments are required");
      else if (arguments->widening && !arguments->abstract)
      	argp_error(state, "Widening (-w or --widening) can only be used with Abstract analysis (-a or --abstract)");
      else if (arguments->concrete && has_interval_param)
      	argp_error(state, "Concrete analysis (-c or --concrete) requires parameters to be assigned single values only");
      break;
    default:
      return ARGP_ERR_UNKNOWN;
  }
  return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

void parse_arguments(int argc, char **argv, Arguments *arguments) {
  /* 
   * Default values
   */
  arguments->concrete = arguments->abstract = arguments->widening = arguments->verbose =
  arguments->parse_tree = arguments->ast = arguments->cfg = has_interval_param = false;
  arguments->output = NULL;
  arguments->value_params = arguments->interval_params = NULL;
  arguments->N_iteration = 20;
  arguments->N_columns = 1;
  arguments->N_params = 0;
  argp_parse(&argp, argc, argv, 0, 0, arguments);
}

void free_arguments(Arguments *arguments) {
  size_t i;
  for (i = 0; i < arguments->N_params; ++i) {
  	if (arguments->interval_params[i])
  	  free(arguments->interval_params[i]);
  	if (arguments->value_params[i])
  	  free(arguments->value_params[i]);
  }
  if (arguments->N_params) {
  	free(arguments->interval_params);
  	free(arguments->value_params);
  }
}