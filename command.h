/*
 *  parse_command.h
 *  
 *
 *  Created by amir rahimi on 4/3/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#undef	G_LOG_DOMAIN
#define	G_LOG_DOMAIN "parse_command"

#define STACK_PUSH '['
#define STACK_POP ']'
#define FUNC_CALL 0x100

struct CommandData
{
	guint type; // type of this command, starts with ...
	Function *func; // in case of FUNC_CALL, stores associated function object
	GArray *infix_term; // in case of FUNC_CALL, stores parameter term  
};

void print_command(GArray *commands, GString *output)
{
	for (guint i = 0; i < commands->len; i++)
    {
		struct CommandData *cur_command = &g_array_index(commands, struct CommandData, i);
		switch (cur_command->type)
		{
			case FUNC_CALL:
			{
				g_string_sprintfa(output, "%s(", cur_command->func->ident->str);
				print_term(cur_command->infix_term, output);
				g_string_append(output, ") ");
			}; break;
				
			case STACK_PUSH:
				g_string_sprintfa(output, "[ "); break;
				
			case STACK_POP:
				g_string_sprintfa(output, "] "); break;
				
			default:
				g_string_sprintfa(output, "?? "); break;
		}
    }
}


GArray *parse_command(GScanner *scanner, GString *param_string, int *correct_rule)
{
	GArray *commands = g_array_new(FALSE, FALSE, sizeof(struct CommandData));
	*correct_rule = TRUE;
	
	while(g_scanner_peek_next_token(scanner) != G_TOKEN_EOF && g_scanner_peek_next_token(scanner) != '<') 
    {
		/* get the next token */
		GTokenType token = g_scanner_get_next_token(scanner);
		struct CommandData cur_command;
		
		switch (token) 
		{
			case G_TOKEN_SYMBOL:
			{
				/* if we have a function symbol, print it out */
				GTokenValue value = g_scanner_cur_value(scanner);
				Function *func = (Function *)value.v_symbol;
				g_message("SYMBOL: %s\n", func->ident->str);
				GTokenType token = g_scanner_get_next_token(scanner);
				int correct_param_content;
				
				if (token == '(')
					cur_command.infix_term = parse_term(scanner, G_TOKEN_RIGHT_PAREN, param_string, &correct_param_content);
				else
				{
					g_scanner_error(scanner, "Function %s needs a parameter list using ().", func->ident->str);
					*correct_rule = FALSE; continue;
				}
				
				if (correct_param_content)
				{
					if (g_scanner_get_next_token(scanner) == ')')
					{
						g_message("Adding function call\n");
						cur_command.type = FUNC_CALL;
						cur_command.func = func;
						//cur_command.infix_term = infix_term; // set above			
						g_array_append_val(commands, cur_command);
					}
					else
					{
						g_scanner_error(scanner, "Function %s needs a final ) after parameter definition.", func->ident->str);
						*correct_rule = FALSE; continue;
					}
				}
				else
				{
					g_scanner_error(scanner, "Function %s: Parameter list parsing failed.", func->ident->str);
					*correct_rule = FALSE; continue;
				}
			}; break;
				
			case '[':
			{
				g_message("STACK PUSH\n");
				cur_command.type = STACK_PUSH;
				cur_command.func = NULL;
				cur_command.infix_term = NULL;			
				g_array_append_val(commands, cur_command);		
			}; break;
				
			case ']':
			{
				g_message("STACK POP\n");
				cur_command.type = STACK_POP;
				cur_command.func = NULL;
				cur_command.infix_term = NULL;			
				g_array_append_val(commands, cur_command);		
			}; break;
				
			default:
			{
				g_scanner_error(scanner, "Invalid token encountered. %c 0x%x\n", (char)token, token);
				*correct_rule = FALSE;
			}
		}	  
    }
	
	return commands;
}


