/*
 *  LSystem.h
 *  
 *
 *  Created by amir rahimi on 4/3/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#define MAX_LEVEL 8

/*
 LSystem interpreter. 
 */

#include "Function.h"

class LSystem
	{
	private:
		GList *funcset; // the function set / alphabet of this LSystem
		GPtrArray *rules; // describes the rules that this LSystem obeys
		GPtrArray *results; // an array that contains the command sequences to draw the result in each development step
		gint level; // the current level that has been developed
		int time; // the current animation time
		GScanner *lscanner; // parses the different rules and development step command sequences
		
		GString *param_string; // defines the parameter that is used in all command sequences and rules
		int max_depth; // the maximum depth to which new results shall be calculated
		
	public:
		int max_level;
		
		LSystem(guint _max_depth);
		~LSystem();
		
		void Draw();
		void DrawLevel(int level);
		void Develop();
		void Animate();
		bool LSystem::AddRule(const char *rulestr);
		
		void LSystem::AddFunction_SpaceTransform(const char *ident_str, const guint rotate, const guint translate, const guint scale);
		void LSystem::AddFunction_Primitive(const char *ident_str, int prim_type, float r, float g, float b);
		void LSystem::AddScannerSymbol(const char *ident, gpointer new_func);
		
		static void trav_func(gpointer data, gpointer user_data);
		void LSystem::PrintFunctionSet();
		GList *LSystem::ParseXFix(GScanner *scanner, GList *xfix, bool &correct_xfix); // parses prefix and postfix statements
		bool LSystem::SetResult(int level, const char *init_axiom);
	};


