#undef	G_LOG_DOMAIN
#define	G_LOG_DOMAIN "parse"

#define NAN -99999.0

// type parameters
#define PAR_VAR1 't'
#define PAR_CONST_FLOAT  'c'

//operand types that can be in the parse tree
#define OP_MULT      '*'
#define OP_ADD       '+'
#define OP_SUB       '-'
#define OP_DIV       '/'
#define OP_LEFT_PAREN '('

#define OP_INVERT    '_'

#define COMP_EQUAL      '='
#define COMP_LE_EQUAL   '{'
#define COMP_GR_EQUAL   '}'
#define COMP_NOT_EQUAL  '|'
#define COMP_LESS       '<'
#define COMP_GREATER    '>'

struct ParseData
{
  guint type;
  double value;
};

int parse_priority(guint op_type)
{
  switch (op_type)
    {
    case OP_INVERT:
      return 3; break;
    case OP_MULT:
    case OP_DIV:
      return 2; break;
    case OP_ADD:
    case OP_SUB:
      return 1; break;
    case COMP_EQUAL:
    case COMP_LE_EQUAL:
    case COMP_GR_EQUAL:
    case COMP_NOT_EQUAL:
    case COMP_LESS:
    case COMP_GREATER:
      return 0;
    case OP_LEFT_PAREN:
      return -1;
    default:
      g_error("Invalid priority encountered.\n");
      return 0;
    }
}

inline const char* parse_op_str(guint op_type)
{
  switch (op_type)
    {
    case OP_INVERT:
      return "-"; break;
    case OP_MULT:
      return "*"; break;
    case OP_DIV:
      return "/"; break;
    case OP_ADD:
      return "+"; break;
    case OP_SUB:
      return "-"; break;
    case COMP_EQUAL:
      return "=="; break;
    case COMP_LE_EQUAL:
      return "<="; break;
    case COMP_GR_EQUAL:
      return ">="; break;
    case COMP_NOT_EQUAL:
      return "!="; break;
    case COMP_LESS:
      return "<"; break;
    case COMP_GREATER:
      return ">"; break;
    case OP_LEFT_PAREN:
      return "("; break;
    default:
      g_error("Invalid operator encountered.\n");
      return 0;
    }
}

int parse_associativeness(guint prio)
{
  if (prio <= 0) 
    return 0; 
  else return 1;
}

void print_term(GArray *math_term, GString *output)
{
  for (guint i = 0; i < math_term->len; i++)
    {
      struct ParseData *cur_param = &g_array_index(math_term, struct ParseData, i);
      
      switch (cur_param->type)
	{
	case OP_MULT:
	case OP_DIV:
	case OP_ADD:
	case OP_SUB:
	case COMP_LESS:
	case COMP_GREATER:
	case COMP_EQUAL:
	  g_string_sprintfa(output, "%c ", (char)cur_param->type); break;
	  
	case COMP_LE_EQUAL:
	  g_string_sprintfa(output, "<= "); break;
	  
	case COMP_GR_EQUAL:
	  g_string_sprintfa(output, ">= "); break;

	case COMP_NOT_EQUAL:
	  g_string_sprintfa(output, "!= "); break;

	case PAR_VAR1:
	  g_string_sprintfa(output, "%c ", 't'); break;

	case PAR_CONST_FLOAT:
	  g_string_sprintfa(output, "%.2f ", cur_param->value); break;

	case OP_INVERT:
	  g_string_sprintfa(output, "_ "); break;

	default:
	  g_string_sprintfa(output, "(Unknown operand type 0x%x) ", cur_param->type); 
	}
    }  
}

/**
   Fills the symbol table nr. 2 of a GScanner with the recognized symbols for math parsing.
   @param scanner the scanner that should be configured
 */
void init_parser(GScanner *scanner)
{
  //g_scanner_freeze_symbol_table(scanner);
  // add operands for the parse tree
  g_scanner_scope_add_symbol(scanner, 2, ">=", (void *)COMP_GR_EQUAL);
  g_scanner_scope_add_symbol(scanner, 2, ">",  (void *)COMP_GREATER);
  g_scanner_scope_add_symbol(scanner, 2, "<=", (void *)COMP_LE_EQUAL);
  g_scanner_scope_add_symbol(scanner, 2, "<",  (void *)COMP_LESS);
  g_scanner_scope_add_symbol(scanner, 2, "=",  (void *)COMP_EQUAL);
  g_scanner_scope_add_symbol(scanner, 2, "!=", (void *)COMP_NOT_EQUAL);

  g_scanner_scope_add_symbol(scanner, 2, "+", (void *)OP_ADD);
  g_scanner_scope_add_symbol(scanner, 2, "-", (void *)OP_SUB);
  g_scanner_scope_add_symbol(scanner, 2, "*", (void *)OP_MULT);
  g_scanner_scope_add_symbol(scanner, 2, "/", (void *)OP_DIV);

  //g_scanner_thaw_symbol_table(scanner);
}

GArray *parse_term(GScanner *scanner, GTokenType terminator, GString *param_string, int *correct_parse)
{
  *correct_parse = false;
  struct ParseData cur_param;
  int old_scope;
  int open_parentheses = 0;

  old_scope = g_scanner_set_scope(scanner, 2);
  GArray *out_stack = g_array_new(FALSE, FALSE, sizeof(struct ParseData));
  GArray *side_stack = g_array_new(FALSE, FALSE, sizeof(struct ParseData));
  //bool operator_mode = false;

  GString *identifier_first = g_string_new(param_string->str);
  g_string_append(identifier_first, "<>*+-/!"); // only valid identifiers can be valid symbols - these chars are usually not allowed
  gchar *old_identifier_first = scanner->config->cset_identifier_first;
  scanner->config->cset_identifier_first = identifier_first->str;

  gchar *old_identifier_nth = scanner->config->cset_identifier_nth;
  scanner->config->cset_identifier_nth = "=";

  g_message("Parsing term.\n");
  /* while the next token is something else other than end of file */
  int operator_mode = FALSE;

  while(g_scanner_peek_next_token(scanner) != G_TOKEN_EOF && ((g_scanner_peek_next_token(scanner) != terminator) || open_parentheses)) 
    {      
      /* get the next token */
      GTokenType token = g_scanner_get_next_token(scanner);
      //printf("Got token: 0x%x %c, terminator is 0x%x %c\n", token, (char)token, terminator, (char)terminator);
      switch (token) 
	{
	case G_TOKEN_IDENTIFIER:
	  {
	    /* if we have an identifier, print it out */
	    GTokenValue value = g_scanner_cur_value(scanner);
	    g_message("PARAMETER: %s\n", value.v_identifier);
	    if (strncmp(value.v_identifier, param_string->str, param_string->len) == 0)
	      {
		cur_param.type = PAR_VAR1;
		cur_param.value = -1;
		g_array_append_val(out_stack, cur_param);
	      }
	    else
	      {
		g_scanner_error(scanner, "Invalid identifier %s, %s expected\n", 
				value.v_identifier, param_string->str);
		return NULL;
	      }
	    operator_mode = TRUE; // next expected token is an operator
	  }; break;

	case G_TOKEN_LEFT_PAREN:
	  {
	    g_warning("NOT TESTED: konj: %c\n", token);
	    cur_param.type = OP_LEFT_PAREN;
	    cur_param.value = -1;
	    g_array_append_val(side_stack, cur_param);
	    open_parentheses++;
	  }; break;		

	case G_TOKEN_RIGHT_PAREN: // pop latest op from the stack - NOT IMPLEMENTED
	  {
	    g_warning("NOT TESTED: konj: %c\n", token);
	    guint op = 0; 
	    while (side_stack->len > 0 && op != OP_LEFT_PAREN)
	      {
		ParseData *work_param = &g_array_index(side_stack, struct ParseData, side_stack->len-1);
		op = work_param->type;
		g_message("Popped %c\n", (char)op);
		if (op != OP_LEFT_PAREN) // do NOT add the parentheses themselves
		  {
		    g_array_append_val(out_stack, *work_param);// store the old op on the out stack		    		
		  }
		g_array_remove_index(side_stack, side_stack->len-1);
	      }

	    if (open_parentheses > 0) 
	      open_parentheses--;
	    operator_mode = TRUE; // next expected token is an operator, since the parentheses enclose a value
	  }; break;		

	case G_TOKEN_ERROR:
	  {
	    g_error("PARSE ERROR\n");
	  }; break;		

	case G_TOKEN_SYMBOL:
	  {
	    /* if we have a function symbol, print it out */
	    GTokenValue value = g_scanner_cur_value(scanner);
	    guint op = (guint)value.v_symbol;
	    g_message("OPERATOR: %c\n", (char)op);

	    cur_param.type = op;
	    cur_param.value = -1;

	    if (operator_mode)
	      {
		if (side_stack->len > 0)
		  {
		    ParseData *work_param = &g_array_index(side_stack, struct ParseData, side_stack->len-1);
		    int w_prio = parse_priority(work_param->type);
		    int c_prio = parse_priority(op);
		    int move_side_to_out = TRUE;
		    
		    if (w_prio > c_prio) // does the side stack op have higher prio ? 
		      move_side_to_out = TRUE;
		
		    if (w_prio == c_prio)
		      {
			if (parse_associativeness(w_prio))
			  move_side_to_out = TRUE;
			else
			  move_side_to_out = FALSE;
		      }
		    
		    if (w_prio < c_prio)
		      move_side_to_out = FALSE;
		    
		    if (move_side_to_out)
		      {
			g_array_append_val(out_stack, *work_param);// store the old op on the out stack		    
		      }
		  }
		operator_mode = FALSE;
	      }
	    else
	      {
		if (op == OP_SUB)
		  {
		    g_message("Subtraction in non-operator mode - assuming negation, adding operator to invert next value on stack\n");
		    cur_param.type = OP_INVERT;
		    cur_param.value = -1;		    
		  }
		else
		  {
		    g_error("Surplus operator %s while in non-operator mode - aborting\n", parse_op_str(op));		    
		  }
	      }
	    
	    g_array_append_val(side_stack, cur_param);// store the new op on the side stack
	  }; break;

	case G_TOKEN_FLOAT:
	  {
	    if (operator_mode)
	      g_error("Operator expected, but received second number.\n");

	    /* if we have an identifier, print it out */
	    GTokenValue value = g_scanner_cur_value(scanner);
	    g_message("float: %.2f\n", value.v_float);
	    cur_param.type = PAR_CONST_FLOAT;
	    cur_param.value = value.v_float;
	    g_array_append_val(out_stack, cur_param);
	    operator_mode = TRUE; // next expected token is an operator
	  }; break;
	  
	  
	default:
	  {
	    g_error("Unknown token type 0x%x\n", token);
	  }
	}
    }
  
  //printf("Popping side stack (len = %d).\n", side_stack->len);

  // parsing finished. Now empty and delete the side stack.
  for (int i = side_stack->len - 1; i >= 0; i--)
    {
      //printf("Transferring side param %d\n", i);
      ParseData *work_param = &g_array_index(side_stack, struct ParseData, i);
      g_array_append_val(out_stack, *work_param);// store the old op on the out stack		    
    }
  g_array_free(side_stack, TRUE);

  //printf("Finished parsing.\n");

  *correct_parse = true;

  // restore the scanner settings.
  g_scanner_set_scope(scanner, old_scope);

  scanner->config->cset_identifier_first = old_identifier_first;
  scanner->config->cset_identifier_nth = old_identifier_nth;

  GString *output = g_string_new("Parsing result is:");
  print_term(out_stack, output);
  g_message("%s", output->str);
  g_string_free(output, TRUE);
  
  g_message("\n");

  return out_stack;
}

GArray *parse_constant_to_term(double param_value)
{
  GArray *out_stack = g_array_new(FALSE, FALSE, sizeof(struct ParseData));
  struct ParseData cur_param;

  cur_param.type = PAR_CONST_FLOAT;
  cur_param.value = param_value;
  g_array_append_val(out_stack, cur_param);
  return out_stack;
}

/** 
    Evaluates a term in postfix notation. 
*/
double eval_term(GArray *term, double par1_value, int *correct_term)
{
  *correct_term = TRUE;
  double accum[200]; // no operand needs more than 3 values anyways
  int accum_pos = 0; // first empty position

  guint term_pos = 0;

  while (term_pos < term->len)
    {
      struct ParseData *op = &g_array_index(term, struct ParseData, term_pos);

      switch(op->type)
	{
	case PAR_VAR1:
	  {
	    g_message("Replacing parameter with %f\n", par1_value);
	    accum[accum_pos] = par1_value;
	    accum_pos++;
	  }; break;
	  
	case PAR_CONST_FLOAT:
	  {
	    g_message("Pushing %g\n", op->value);
	    accum[accum_pos] = op->value;
	    accum_pos++;	    
	  }; break;

	case OP_LEFT_PAREN:
	  {
	    g_error("Invalid parentheses left in op list !\n");
	    *correct_term = false; return NAN;
	  }; break;
  
	case OP_INVERT:
	  {
	    if (accum_pos < 1)
	      g_error("Inversion operator needs one operand.\n");

	    accum[accum_pos - 1] = -accum[accum_pos - 1];
	    g_message("Inversion: Result %g\n", accum[accum_pos - 1]);
	  }; break;
  
	default: // all the rest of the operands needs two parameters
	  {
	    if (accum_pos < 2)
	      {
		g_error("operators * + - / need two operands.\n");
		*correct_term = false; return NAN;
	      }
		
	    double result = NAN;
	    switch(op->type)
	      {
	      case OP_MULT:
		result = accum[accum_pos - 2] * accum[accum_pos - 1]; break;
	      case OP_ADD:
		result = accum[accum_pos - 2] + accum[accum_pos - 1]; break;
	      case OP_SUB:
		result = accum[accum_pos - 2] - accum[accum_pos - 1]; break;
	      case OP_DIV:
		result = accum[accum_pos - 2] / accum[accum_pos - 1]; break;
	      case COMP_EQUAL:
		result = (accum[accum_pos - 2] == accum[accum_pos - 1]); break;
	      case COMP_LE_EQUAL:
		result = (accum[accum_pos - 2] <= accum[accum_pos - 1]); break;
	      case COMP_GR_EQUAL:
		result = (accum[accum_pos - 2] >= accum[accum_pos - 1]); break;
	      case COMP_NOT_EQUAL:
		result = (accum[accum_pos - 2] != accum[accum_pos - 1]); break;
	      case COMP_LESS:
		result = (accum[accum_pos - 2] < accum[accum_pos - 1]); break;
	      case COMP_GREATER:
		result = (accum[accum_pos - 2] > accum[accum_pos - 1]); break;	
	      default:
		g_error("Unknown two param-operand, opcode 0x%x", op->type);
		*correct_term = false; return NAN;		
	      }

	    accum[accum_pos - 2] = result; // overwrite values in stack
	    accum_pos --; // adjust stack pointer
	  }; break;	    
	}

      term_pos++;
    }

  if (accum_pos != 1)
    {
      g_error("Error: Too many values left on accumulator stack after finishing term.\n");
      *correct_term = false; return NAN;		
    }
  else
    {
      return accum[0];
    }
}

