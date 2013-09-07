/*
 *  Function.h
 *  
 *
 *  Created by amir rahimi on 4/3/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef __FUNCTION_H_
#define __FUNCTION_H_
 /*TYPE = Computation Type */
#define TYPE_SPACE_TRANSFORM 0x100
#define TYPE_PRIMITIVE 0x101

/*  Space Transform Type */
#define ROTATION_X  0x1
#define ROTATION_Y  0x2
#define ROTATION_Z  0x4
#define TRANSLATION_X  0x10
#define TRANSLATION_Y  0x20
#define TRANSLATION_Z  0x40
#define SCALE_X 0x100
#define SCALE_Y 0x200
#define SCALE_Z 0x400

/*OpenGL Primitive types*/
#define PRIMITIVE_LINE 0x1
#define PRIMITIVE_CYLINDER 0x2
#define PRIMITIVE_NOTHING 0x3
#define PRIMITIVE_DISK 0x4

/// stores additional space transform data
struct space_transform_t
{
	int affect_field;
};

/// stores additional primitive data
struct primitive_data_t
{
	int type;
	float r, g, b;
};

/// Stores the supplemental data for each function type. 
union CompData
{
	struct space_transform_t space_transform ;
	struct primitive_data_t prim;
};

/**
 Stores and draws L-System alphabet entries/functions, like "F(t)" or "+(t)"
*/
class Function
{
	private:
		union CompData data; //stores additional detail data
		int type; //determines the type of this function (see TYPE)

	public:
		GString *ident; //the identity string (e.g. the "F" in "F(t)")
		Function(const char *_ident_str, int _type);
		~Function();

		void SpaceTransformData(int _affect_field);
		void PrimitiveData(int _type, float _r_color, float _g_color, float _b_color);

		void DrawGL(float value, guint time, guint stack_level); 
		void Print(GString *target); 
};

#endif

