/*
 *  LSystem.cpp
 *  
 *
 *  Created by amir rahimi on 4/3/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <glib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <OpenGL/OpenGL.h>
#include "LSystem.h"

#include "parse.h"
#include "command.h"
#include "Rule.h"
#include "Function.cpp"

#undef	G_LOG_DOMAIN
#define G_LOG_DOMAIN "lsys"

/**
 Initializes level to invalid, sets the parameter string to "t", initializes the internal lexical scanner.
 _max_depth is The maximum depth of the L-System calculation if lsys.Develop() is called repeatedly
 */
LSystem::LSystem(guint _max_depth)
{
	max_depth = _max_depth;
	
	level = -1;
	max_level = -1;
	
	rules = g_ptr_array_new();
	results = g_ptr_array_new();
	
	param_string = g_string_new("t");
	
	/* create a new scanner */
	lscanner = g_scanner_new(NULL);
	lscanner->config->scan_identifier_1char = TRUE;
	lscanner->config->scan_identifier = TRUE;
	lscanner->config->scan_symbols = TRUE;
	lscanner->config->int_2_float = TRUE;
	
	init_parser(lscanner); /* add the mathematical symbols */
}

/**
 Sets the calc result for a certain level
 returns true if SetResult was successful
 */
bool LSystem::SetResult(int _level, const char *init_axiom)
{
	int correct_commands;
	
	printf("SetResult for level %d, string %s\n", _level, init_axiom);
	
	/* set up the scanner to read from the string */
	g_scanner_input_text(lscanner, init_axiom, strlen(init_axiom));
	
	GArray *new_result = parse_command(lscanner, param_string, &correct_commands);
	
	if (correct_commands)
    {
		GString *seed_str = g_string_new("Correct result. Seed string is:");
		print_command(new_result, seed_str);
		printf("%s (len %d)\n", seed_str->str, new_result->len);
		g_string_free(seed_str, TRUE);
		
		g_ptr_array_add (results, (gpointer) new_result);
		level = _level;
		return true;
    }
	else
    {
		printf("SetResult failed for level %d, string %s\n", level, init_axiom);
		return false;
    }
}

/**
 Lets the scanner know about a new function symbol
 ident_str is the identity string of this function
 new_func is the function pointer
 */
void LSystem::AddScannerSymbol(const char *ident_str, gpointer new_func)
{
	g_scanner_scope_add_symbol(lscanner, 0, ident_str, new_func);
}

/** 
 Add a space transform to the LSystem function set (its alphabet).
 ident_str is the name of the function
 rotate is bitwise OR if the function's parameter should affect x (0x1), y (0x2) and/or z (0x4) rotation 
 translate is bitwise OR if the function's parameter should affect x, y and/or z translation (encoding see above)
 scale is bitwise OR if the function's parameter should affect x, y and/or z scaling (encoding see above)
 */
void LSystem::AddFunction_SpaceTransform(const char *ident_str, const guint rotate, const guint translate, const guint scale)
{
	Function *new_func = new Function(ident_str, TYPE_SPACE_TRANSFORM);
	
	new_func->SpaceTransformData((rotate) | (translate << 4) | (scale << 8)); 
	
	funcset = g_list_append(funcset, (gpointer)new_func);  
	
	AddScannerSymbol(ident_str, new_func);
}

/** 
 Add a primitive drawer to the LSystem function set (its alphabet).
 ident_str is the name of the function
 prim_type the type of primitive (starts with PRIMITIVE_ @see Function.h ) 
 */
void LSystem::AddFunction_Primitive(const char *ident_str, int prim_type, float r, float g, float b)
{
	Function *new_func = new Function(ident_str, TYPE_PRIMITIVE);
	
	new_func->PrimitiveData(prim_type, r, g, b);
	
	funcset = g_list_append(funcset, (gpointer)new_func);  
	
	AddScannerSymbol(ident_str, new_func);
}

/**
 An internal traversal function to retrieve the function set names.
 data points to the current function set
 user_data contains the output GString
 */
void LSystem::trav_func(gpointer data, gpointer user_data)
{
	Function *cur_func =(Function *)data;
	GString *outstr = (GString *)user_data;
	
	cur_func->Print(outstr); 
};

/**
 Prints the function names in the function set to stdout. 
 */
void LSystem::PrintFunctionSet()
{
	GString *outstr = g_string_new("Current Function set:");
	g_list_foreach(funcset, trav_func, outstr);
	
	printf("%s\n", outstr->str);
}

/**
 Basic format is 
 {candidate}, {precondition}, {postcondition}, [{parameter condition}]: <{probability}> {command sequence}
 
 F(t): name of the candidate term.
 
 precondition: <br>
 Sets a requirement on what the candidate term must be prepended with, can be
 - A sequence of function names (without parameters), e.g.: F G F F
 - An asterisk (*) to signal that anything is allowed
 - An empty precondition, to signal that nothing is allowed before the candidate term. 
 
 postcondition: <br>
 Sets a requirement on what the successor of the candidate term must be like, can be
 - A sequence of function names (without parameters), e.g.: F G F F
 - An asterisk (*) to signal that anything is allowed
 - An empty precondition, to signal that nothing is allowed after the candidate term. 
 
 parameter condition:<br>
 sets a requirement for a parameter, can be:
 - Any logical expression using <, >, <=, >=, =, !=
 - empty to signal that any parameter value is allowed
 
 probability:<br>
 the probability that the following command sequence replaces the candidate term, can be
 - a number (0.0 < p < 1.0), describes p*100 percent of probability
 - empty, which will cover all the remaining probability value up to 100%
 
 <p>The probability and command sequences are repeated until 100% is reached. 
 
 <p>Example: <br>
 F(t), *, *, [t<9]: <0.5> F(t*3) <> F(t) G(t)
 
 rulestr is the rule string in the format described above. 
 returns true on success
 */
bool LSystem::AddRule(const char *rulestr)
{
	Rule *new_rule = new Rule(); 

	g_scanner_input_text(lscanner, rulestr, strlen(rulestr));

	if (new_rule->SetRule(lscanner, funcset))
	{
		g_ptr_array_add (rules, (gpointer) new_rule);
		printf("Rule %s added.\n", rulestr);

		return true;
	}
	else
	{
		printf("Rule add failed.\n");
		delete new_rule;      
		return false;
	}
}  

LSystem::~LSystem()
{
	g_ptr_array_free (results, TRUE);
	g_ptr_array_free (rules, TRUE);
	
	g_scanner_destroy(lscanner);  
}

/** 
 Develops the L-system one or several steps from a given level, up to max_depth.
 */
void LSystem::Develop()
{
	if (level >= max_depth) 
		return;
	
	g_message("\nDevelop called, level %d.\n", level);
	
	GArray *seed_commands = (GArray *)g_ptr_array_index(results, level);
	GString *seed_str = g_string_new("");
	print_command(seed_commands, seed_str);
	g_message("Seed commands: %s (len %d)\n", seed_str->str, seed_commands->len);
	g_string_free(seed_str, TRUE);
	
	GArray *output_commands = g_array_new(FALSE, FALSE, sizeof(struct CommandData));
	
	guint cur_pos = 0; // current position in input
	while (cur_pos < seed_commands->len)
    {	  
		g_message("At pos. %d\n", cur_pos);
		struct CommandData *cur_command = &g_array_index(seed_commands, struct CommandData, cur_pos);
		
		switch (cur_command->type)
		{
			case STACK_PUSH:
			case STACK_POP:
			{
				g_array_append_val(output_commands, *cur_command);
				cur_pos++;     
			}; break;
			case FUNC_CALL:
			{
				gint match = 0;
				guint rule_index = 0;
				bool match_found = false; 
				int correct_term;
				double value = eval_term(cur_command->infix_term, NAN, &correct_term);
				
				if (!correct_term)
				{
					g_error("Couldn't evaluate parameter term in seed !\n");
				}
				else
				{
					g_message("Function call %s(%g)\n", cur_command->func->ident->str, value);
				}
				
				while (match == 0 && rule_index < rules->len) // go through all the rules
				{	  
					g_message("Applying rule nr. %d:\n", rule_index);
					Rule *cur_rule = (Rule *)g_ptr_array_index (rules, rule_index);
					gint match = cur_rule->Match(cur_command, seed_commands, cur_pos, value); // does this rule apply now ? 
					
					if (match) // this rule applies
					{     
						//g_message("Rule matched.\n");
						int correct_gen; 
						cur_rule->Generate(cur_command, output_commands, value, &correct_gen); // let it generate its output
						cur_pos += match;
						match_found = true;
					}
					else
						g_message("Rule didn't match.\n");
					rule_index++;
				}
				
				if (!match_found)
				{
					g_array_append_val(output_commands, *cur_command);
					cur_pos++;
				}	
			}; break;
			default:
				g_error("Unknown command type: 0x%x", cur_command->type); cur_pos++;
		}
    }
	
	g_ptr_array_add(results, output_commands);
	level++;
	if (max_level < level)
		max_level = level; 
}

/**
 Draws Lsystem at a certain level 
 */
void LSystem::DrawLevel(int level)
{
	g_message("DRAW Level: %d max_level %d\n", level, max_level);
	
	if (level > max_level)
    {
		g_warning("Invalid level %d (max: %d).", level, max_level);
		return;
    }
	
	GArray *draw_commands = (GArray *)g_ptr_array_index(results, level);
	
	
	guint stack_level = 0;
	
	for (guint i = 0; i < draw_commands->len; i++)
    {
		struct CommandData *cur_command = &g_array_index(draw_commands, struct CommandData, i);
		switch (cur_command->type)
		{
			case FUNC_CALL:
			{
				// to make this quick, we assume that the function has one constant as a parameter.
				Function *func = cur_command->func;
				struct ParseData *param = &g_array_index(cur_command->infix_term, struct ParseData, 0);
				//g_message("Value is %f", param->value);
				func->DrawGL(param->value, time, stack_level);
			}; break;
				
			case STACK_PUSH:
			{
				glPushMatrix();
				stack_level++;
			}; break;
				
			case STACK_POP:
			{
				glPopMatrix();
				stack_level--;
			}; break;
				
			default:
				g_error("Unknown drawing command 0x%x", cur_command->type);
		}
    }
}

void LSystem::Draw()
{
	DrawLevel(level);
}
void LSystem::Animate()
{
	time++;
	//time = 49;
	Develop();
}




