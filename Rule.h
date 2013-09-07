/*
 *  Rule.h
 *  
 *
 *  Created by amir rahimi on 4/3/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "Rule"

/**
 Stores info for one subrule (one probability branch, or: one possible 
 command sequence that is going to replace the original one)
 */
struct SubRuleData
{
	float probability;  //Probability that this subrule is going to be applied. (0.0 < prob < 1.0)
	GArray *commands; //A chain of CommandData structs that describes the new command sequence. @see CommandData
};

/**
 This object can store one L-System rule and provides functions for 
 - adding subrules (with their probability)
 - matching this rule against a given snippet of a command sequence
 - generating 
 */
class Rule
	{
	private:
		bool Rule::MatchXfix(GArray *commands, int index, GList *xfix, int dir);
		bool Rule::MatchPrefix(GArray *commands, guint match_index, GList *prefix);
		bool Rule::MatchPostfix(GArray *commands, guint match_index, GList *postfix);
		GList *Rule::ParseXFix(GScanner *scanner, GList *xfix, bool &correct_xfix);
		void Rule::PrintXfix(GList *xfix, GString *output); // Print either rule match postfix or prefix to the output.
		
	public:
		Function *bound_func; //the name of the bound function (the function with the ident name)
		GString *ident; // the identity string (candidate function name) of this rule. 
		GList *prefix; // Contains the allowed prefixes. NULL means nothing allowed before. NULL in an otherwise empty GList means anything allowed.
		GList *postfix; // Contains the allowed postfixes. NULL means nothing allowed before. NULL in an otherwise empty GList means anything allowed.
		GArray *subrules; // Contains the different rules that apply if the rule matches. Each rule has its probability.
		GArray *condition; // Contains the math-postfix term that describes the condition.
		
		bool correct_prefix;
		bool correct_postfix;
		int correct_condition;
		bool correct_pattern;  
		
		Rule::Rule();
		~Rule();
		int Rule::Match(struct CommandData *cur_command, GArray *commands, int match_index, double param_value); ///< Check if this rule matches
		
		/// Generate the rule output.
		void Rule::Generate(struct CommandData *cur_command, GArray *output_commands, double param_value, int *correct_generation);
		
		void Rule::Print(GString *output); // Prints the content of this rule to the output. 
		void Rule::SetCondition(GArray *_infix_cond); // Set the rule parameter value condition.
		void Rule::AddSubRule(float _prob, GArray *_commands); // Add a sub rule with its probability.
		void Rule::PrintCommands(GArray *commands, GString *output); // Print the commands of one command sequence to the output.
		
		bool Rule::SetRule(GScanner *scanner, GList *funcset); // Set the rule content.
	};

/**
 Initializes the invalid variables and creates a new subrule array.
 */
Rule::Rule():
prefix(NULL), postfix(NULL), condition(NULL),
correct_prefix(false), correct_postfix(false), correct_condition(FALSE),
correct_pattern(false)
{
	subrules = g_array_new(FALSE, FALSE, sizeof(SubRuleData));
}

Rule::~Rule()
{
	if (prefix)
		g_list_free(prefix);
	
	if (postfix)
		g_list_free(postfix);
	
	g_string_free(ident, TRUE);
	g_array_free(subrules, TRUE);
}

void Rule::SetCondition(GArray *_infix_cond)
{
	condition = _infix_cond;
}

/**
 Tries to match a valid rule pre/postfix sequence to the given command sequence.
 commands is the command sequence to match to
 index is the position of the command that should be replaced
 xfix is the list of pre/postfixes that the command sequence must match to 
 dir is the search direction (1 or -1)
 returns true if match succeeded
 */
bool Rule::MatchXfix(GArray *commands, int index, GList *xfix, int dir)
{
	while (xfix != NULL)
    {
		Function *func = (Function *)xfix->data;
		if (func != NULL)
		{
			struct CommandData *cur_command = &g_array_index(commands, struct CommandData, index);
			if (cur_command == NULL)  // beyond the end of the array ? 
				return false;
			switch(cur_command->type)
			{
				case STACK_PUSH:
				case STACK_POP:
					g_message("Ignoring stack push/pop\n");
					index += dir;
					break;
					
				case FUNC_CALL:
				{	 
					g_message("Comparing %s to %s:", cur_command->func->ident->str, func->ident->str);
					if (strncmp(cur_command->func->ident->str, func->ident->str, func->ident->len) == 0)
					{
						g_message("Match !\n");
						index += dir;
						xfix = xfix->next;      							   
					}
					else
					{
						g_message("No match !\n");
						return false;
					}
				}; break;
				default:
					g_message("Unrecognized command type 0x%x - aborting\n", cur_command->type);
					return false;	      
			}
		}
		else
			return true; // anything is allowed
    }  
	
	return true; // we looped through, everything ok
}

/**
 Tries to match the rule's prefix to the given command sequence 
 returns true on success
 */
bool Rule::MatchPrefix(GArray *commands, guint match_index, GList *prefix)
{
	g_message("Matching Prefix.\n");
	if(prefix == NULL) // nothing allowed before ? 
    {
		if (match_index == 0) // at start pos ? 
			return true;
		else
			return false;
    }
	
	return MatchXfix(commands, match_index - 1, prefix, -1);
}

/**
 Tries to match the rule's postfix to the given command sequence 
 */
bool Rule::MatchPostfix(GArray *commands, guint match_index, GList *postfix)
{
	g_message("Matching Postfix.\n");
	if(postfix == NULL) // nothing allowed before ? 
    {
		if (match_index == commands->len - 1) // at end pos ? 
			return true;
		else
			return false;
    }
	
	return MatchXfix(commands, match_index + 1, postfix, 1);
}

/**
 Tries to match the rule to the given command sequence 
 cur_command is the command to match to (simply commands[match_index])
 */
int Rule::Match(struct CommandData *cur_command, GArray *commands, int match_index, double param_value)
{
	/* Assume that this command is a func call */
	Function *ret_func = cur_command->func;
	if (ret_func == bound_func)
    {
		g_message("Function symbols match (%s) !\n", bound_func->ident->str);
    }
	else 
		return FALSE;
	
	if (MatchPrefix(commands, match_index, prefix) &&
		MatchPostfix(commands, match_index, postfix))
    {
		g_message("Postfix & Prefix match !\n");
    }
	else
		return FALSE;
	
	if (condition->len != 0) // is there a condition ? 
    {
		int correct_condition = FALSE;
		double value = eval_term(condition, param_value, &correct_condition);
		GString *cond_str = g_string_new("");
		print_term(condition, cond_str);
		g_warning("Condition term %s, with t=%g (len %d): result is %g\n", 
				  cond_str->str, param_value, condition->len, value);
		if (correct_condition)
		{
			g_message("Correct condition encountered.\n");
			return (int)value;
		}
		else
		{
			g_message("Unfulfilled condition.\n");
			return FALSE;
		}  
    }  
	else
		return TRUE;
}

/**
 */
void Rule::PrintXfix(GList *xfix, GString *output)
{
	while (xfix != NULL)
    {
		Function *func = (Function *)xfix->data;
		if (func != NULL)
			g_string_sprintfa(output, "%s ", func->ident->str);
		else
			g_string_append(output, "*");
		
		xfix = xfix->next;
    }  
}

/**
 Prints a given command sequence to a GString output.
 */
void Rule::PrintCommands(GArray *commands, GString *output)
{
	print_command(commands, output);
}

/**
 Prints a complete rule to a GString output. 
 */
void Rule::Print(GString *output)
{
	gchar *rulestr = g_strdup_printf("{%s(t),", ident->str);
	g_string_append(output, rulestr);
	g_free(rulestr);
	
	PrintXfix(prefix, output);
	g_string_append(output, ",");
	PrintXfix(postfix, output);
	g_string_append(output, ",[");
	print_term(condition, output);
	g_string_append(output, "]:");
	
	for (guint rules = 0; rules < subrules->len; rules++)
    {
		struct SubRuleData *subrule = &g_array_index(subrules, struct SubRuleData, rules);      
		g_string_sprintfa(output, "<%.2f> ", subrule->probability);
		PrintCommands(subrule->commands, output);
    }
	
	g_string_append(output, "}\n");
}

/**
 Adds a subrule to the internal array
 _prob is the probability for this subrule to be used
 commands is the command sequence that this subrule is going to insert
 */
void Rule::AddSubRule(float _prob, GArray *commands)
{
	struct SubRuleData new_subrule;
	new_subrule.probability = _prob;
	new_subrule.commands = commands;
	
	g_array_append_val(subrules, new_subrule);
}

/**
 Parses the pre-/postfix part of a given rule description string. 
 xfix Holds the pre-/postfix sequence after parsing 
 correct_xfix flag to show correct parsing 
 returns xfix again (since xfix itself can change)
 */
GList *Rule::ParseXFix(GScanner *scanner, GList *xfix, bool &correct_xfix)
{
	correct_xfix = true;
	/* scan the xfix pattern. No evaluations allowed, and no parameters, either, only a sequence of symbols or *. */
	g_scanner_set_scope(scanner, 0); /* let it detect symbols again */
	
	while(correct_xfix && g_scanner_peek_next_token(scanner) != ',') 
    {
		/* get the next token */
		GTokenType token = g_scanner_get_next_token(scanner);
		
		switch(token)
		{
			case '*':  // allow anything
			{
				xfix = g_list_append(xfix, NULL);	    
				g_message("SYMBOL: *\n");
			}; break;
				
			case G_TOKEN_SYMBOL:
			{
				/* if we have a function symbol, print it out */
				GTokenValue value = g_scanner_cur_value(scanner);
				Function *func = (Function *)value.v_symbol;
				g_message("SYMBOL: %s\n", func->ident->str);
				xfix = g_list_append(xfix, value.v_symbol);	    	    
			}; break;
				
			default:
			{
				g_scanner_unexp_token(scanner, G_TOKEN_SYMBOL, "a list of symbols without parameter list", NULL, NULL, "Aborting rule add.", TRUE);
				correct_xfix = false; 
			}
		}
    }
	
	return xfix;
}

/**
 Sets the rule description.
 Scans through the description string from left to right, checking the syntax and creating 
 the internal data objects. 
 See LSystem::AddRule 
 returns true on success
 */
bool Rule::SetRule(GScanner *scanner, GList *funcset)
{
	GString *param_string = NULL;
	Function *rule_symbol;
	GTokenType token;
	
	correct_pattern = false;
	
	/* scan the rule application pattern. No evaluations allowed, and the first parameter encountered here is the only one allowed afterwards. */
	/* while the next token is something else other than end of file */
	token = g_scanner_get_next_token(scanner);
	
	if (token == G_TOKEN_SYMBOL)
    {
		/* if we have a function symbol, print it out */
		GTokenValue value = g_scanner_cur_value(scanner);
		rule_symbol = (Function *)value.v_symbol;
		g_message("RULE SYMBOL: %s\n", rule_symbol->ident->str);
		g_scanner_set_scope(scanner, 1); /* switch scope, since there are no recursive loops for functions allowed */
		
		GTokenType token = g_scanner_get_next_token(scanner);
		if (token == '(')
		{
			GTokenType token = g_scanner_get_next_token(scanner);
			if (token == G_TOKEN_IDENTIFIER)
			{
				/* if we have an identifier, print it out */
				GTokenValue value = g_scanner_cur_value(scanner);
				param_string = g_string_new(value.v_identifier);
				g_message("ident: %s\n", value.v_identifier);
				
				GTokenType token1 = g_scanner_get_next_token(scanner);
				GTokenType token2 = g_scanner_get_next_token(scanner);
				
				if (token1 == ')' && token2 == ',') 
					correct_pattern = true;
			}
		}      
    }
	
	if (!correct_pattern)
    {
		g_message("Incorrect pattern, aborting rule add.\n");
		return 0;
    }
	else
    {
		bound_func = rule_symbol;
		ident = g_string_new(rule_symbol->ident->str);
		g_message("Correct matching pattern found: %s(%s).\n", rule_symbol->ident->str, param_string->str);
    }
	
	correct_prefix = false;
	g_message("Scanning prefix sequence:\n");
	prefix = ParseXFix(scanner, prefix, correct_prefix);
	
	/* get the next token */
	token = g_scanner_get_next_token(scanner);
	
	if (token != ',' || !correct_prefix)
    {
		printf("Incorrect pattern, aborting rule add.\n");
    }
	else
    {
		printf("Correct prefix sequence found. Number of elements: %d\n", g_list_length(prefix));
    }
	
	correct_postfix = false;
	g_message("Scanning postfix sequence:\n");
	/* scan the postfix pattern. No evaluations allowed, and no parameters, either, only a sequence of symbols or *. */
	postfix = ParseXFix(scanner, postfix, correct_postfix);
	
	/* get the next token */
	token = g_scanner_get_next_token(scanner);
	
	if (token != ',' || !correct_postfix)
    {
		g_message("Incorrect pattern, aborting rule add.\n");
		return false;
    }
	else
    {
		g_message("Correct postfix sequence found. Number of elements: %d\n", g_list_length(postfix));
    }
	
	correct_condition = false;
	
	g_message("Scanning condition sequence:\n");
	token = g_scanner_get_next_token(scanner);
	if (token == '[')
    {
		int correct_cond_content = FALSE;
		condition = parse_term(scanner, G_TOKEN_RIGHT_BRACE, param_string, &correct_cond_content);
		
		if (correct_cond_content)
		{
			token = g_scanner_get_next_token(scanner);
			if (token == ']')
				correct_condition = true;
		}
    }
	
	/* get the next token */
	token = g_scanner_get_next_token(scanner);
	
	if (token != ':' || !correct_condition)
    {
		g_message("Incorrect condition, aborting rule add.\n");
    }
	else
    {
		g_message("Correct condition sequence found. Number of elements: %d\n", condition->len);
    }
	
	g_message("Scanning rule sequence:\n");
	
	float acc_prob = 0.0;
	bool last_rule = false;
	bool correct_probability = true;
	float cur_prob = 0.0;
	int rule_nr = 1;
	bool correct_rule = true;
	
	while(correct_probability && g_scanner_peek_next_token(scanner) == '<' && !last_rule) 
    {
		token = g_scanner_get_next_token(scanner);
		g_message("Subrule nr. %d.\n", rule_nr);
		g_message("Detecting probability.\n");
		/* read the probablity part of the rule */
		token = g_scanner_get_next_token(scanner);
		correct_probability = false;
		
		switch (token)
		{
			case G_TOKEN_FLOAT:
			{
				/* if we have an identifier, print it out */
				GTokenValue value = g_scanner_cur_value(scanner);
				g_message("given probability: %f\n", value.v_float);
				cur_prob = value.v_float;
				if ((cur_prob + acc_prob) > 1.0)
				{
					g_error("Invalid probability, sums to more than 1.0.\n acc_prob + cur_prob = %f + %f = %f\n", 
							acc_prob, cur_prob, acc_prob + cur_prob);
					continue;
				}
				
				acc_prob += cur_prob;
				token = g_scanner_get_next_token(scanner);
				if (token == '>')
				{
					correct_probability = true;
				}
			}; break;
			case '>':
			{
				last_rule = true; /* the last rule, with empty <>, takes all the rest of the prob */
				g_message("Last rule. Assuming probability %f\n", 1.0 - acc_prob);
				cur_prob = 1.0 - acc_prob;
				correct_probability = true;
			}; break;
			default:
			{
				g_error("Unknown token 0x%x in rule sequence", token);
			}
		}
		
		if (correct_probability)
			g_message("Found probability statement, value %f\n", cur_prob);
		else 
		{
			g_error("Incorrect probability statement, aborting rule add\n");
			break;
		}
		
		int correct_commands = FALSE;
		GArray *commands = parse_command(scanner, param_string, &correct_commands);
		
		correct_rule = true;
		if (correct_rule && correct_commands)
		{
			AddSubRule(cur_prob, commands);
		}
		else
			g_error("Incorrect subrule encountered.\n");
		
		rule_nr++;
    }
	
	g_message("Done parsing rule string.\n");
	
	GString *output = g_string_new("Rule content is:");
	Print(output);
	g_message("%s", output->str);
	g_string_free(output, TRUE);
	
	g_message("\n\n");
	
	return true;
}

/**
 Generates a replacement term for a candidate term. 
 The rule must match (see Rule::Match) and the current parameter value must be given (is contained in cur_command). 
 The choice of the replacing subrule is random and based on the probability distribution in the rule.  
 cur_command is the candidate term that is going to be replaced in the output
 output_commands is the array that contains the list of output commands (is appended)
 param_value is the current value of the function parameter (e.g. 7.3 for t=7.3)
 correct_generation flags the correct generation of a result
 */
void Rule::Generate(struct CommandData *cur_command, GArray *output_commands, double param_value, int *correct_generation)
{
	g_message("Generate called.\n");
	float choose_rule  = 1.0 * random()/RAND_MAX;
	float lower_bound = 0.0; 
	float upper_bound = 0.0;
	
	*correct_generation = TRUE;
	
	for (guint i = 0; i < subrules->len; i++)
    {
		struct SubRuleData *subrule = &g_array_index(subrules, struct SubRuleData, i);
		lower_bound = upper_bound; 
		upper_bound += subrule->probability; 
		
		g_message("rule %d:\n (%f <= choose_rule = %f <= %f) ... ", i, lower_bound, choose_rule, upper_bound);
		
		if ((choose_rule <= upper_bound) && (choose_rule >= lower_bound))
		{
			g_message("MATCHED !\n");
			
			GString *command_str = g_string_new("");
			print_command(subrule->commands, command_str);
			
			g_message("Copying subrule commands %s \nto output, evaluating the terms on the fly.\n", command_str->str);
			
			for (guint comm_nr = 0; comm_nr < subrule->commands->len; comm_nr++)
			{
				struct CommandData *cur_command = &g_array_index(subrule->commands, struct CommandData, comm_nr);
				g_message("Command nr. %d\n", comm_nr);
				
				if (cur_command == NULL)  // beyond the end of the array ? 
				{
					g_error("Strange error in %s: Command is NULL (array empty ?)\n", __FUNCTION__);
				}
				
				switch(cur_command->type)
				{
					case STACK_PUSH:
					case STACK_POP:
						g_message("Copying stack push/pop\n");
						g_array_append_val(output_commands, *cur_command);
						break;
						
					case FUNC_CALL:
					{	 
						GString *command_term = g_string_new("");
						print_term(cur_command->infix_term, command_term);
						int correct_funcall;
						double replace_value = eval_term(cur_command->infix_term, param_value, &correct_funcall);
						if (correct_funcall)
						{
							g_message("Evaluated command term for \"%s\" with argument t=%g => result: %g\n", 
									  command_term->str, param_value, replace_value);
							struct CommandData result;
							result.func = cur_command->func; 
							result.type = cur_command->type;
							result.infix_term = parse_constant_to_term(replace_value);
							g_array_append_val(output_commands, result);
						}
						else
						{
							g_error("Erroneous function call in %s\n", command_term->str);
						}
						
					}; break;
						
					default:
						g_error("Unrecognized command type 0x%x\n", cur_command->type);
				}
			}	  
		}
    }
}

