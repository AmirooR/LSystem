/*
 *  Function.cpp
 *  
 *
 *  Created by amir rahimi on 4/3/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */


#undef G_LOG_DOMAIN
#define G_LOG_DOMAIN "Function" 
#include <OpenGL/OpenGL.h>
#include <GLUT/GLUT.h>
/**
 _ident_str is the name of the function, e.g. "F" or "rz" ("+" or "-" are NOT allowed because of parser restrictions !)
 _type is the type of the function, a TYPE_... define
 */
Function::Function(const char *_ident_str, int _type)
{
	ident = g_string_new(_ident_str);
	type = _type;
	//data.mesh.data_ptr = NULL;
	
	switch(_type)
    {
		case TYPE_SPACE_TRANSFORM:
			g_message("Creating %s, Space Transform\n", _ident_str);
			break;
		case TYPE_PRIMITIVE:
			g_message("Creating %s, Primitive\n", _ident_str);
			break;
		default: 
			printf("FATAL: In creating %s: Unknown data type 0x%x !", _ident_str, _type);
    };
}

Function::~Function()
{

}

/**
 Generates the OpenGL drawing commands that express this function type in 3D space.
 value is the current parameter value, e.g. 7.3 for the t in F(t)
 time is the current animation time frame (for time-dependent alteration)
 stack_level is the current OpenGL stack level (for stack-dependent alteration)
 */
void Function::DrawGL(float value, guint time, guint stack_level)
{
	switch(type)
    {
		case TYPE_SPACE_TRANSFORM:
		{	
			g_message("Drawing %s, Space Transform, value %f, affect_field 0x%x\n", ident->str, value, data.space_transform.affect_field);
			if (data.space_transform.affect_field & ROTATION_X)
				glRotatef(value,1,0,0);
			if (data.space_transform.affect_field & ROTATION_Y)
#if 0
				glRotatef(value,0,1,0);
#else
			if (time % 100 < 50)
				glRotatef(value * ((time % 100)/50.0),0,1,0);
			else
				glRotatef(value * (1.0 - ((time -50) % 100)/50.0),0,1,0);
#endif
			
			if (data.space_transform.affect_field & ROTATION_Z)
#if 1
				//glRotatef(100 * ((time % 100)/100.0),0,0,1);
				glRotatef(value,0,0,1);
#else
			if (time % 100 < 50)
				glRotatef(100 * ((time % 100)/100.0),0,0,1);
			else
				glRotatef(100 * (1.0 - (time % 100)/100.0),0,0,1);
#endif
			
			if (data.space_transform.affect_field & TRANSLATION_X)
				glTranslatef(value,0,0);
			if (data.space_transform.affect_field & TRANSLATION_Y)
				glTranslatef(0,value,0);
			if (data.space_transform.affect_field & TRANSLATION_Z)
				glTranslatef(0,0,value);
			if (data.space_transform.affect_field & SCALE_X)
				glScalef(value,0,0);
			if (data.space_transform.affect_field & SCALE_Y)
				glScalef(0,value,0);
			if (data.space_transform.affect_field & SCALE_Z)
				glScalef(0,0,value);
		}
			break;
		case TYPE_PRIMITIVE:
		{
			switch (data.prim.type)
			{
				case PRIMITIVE_LINE:
				{
					g_message("Drawing %s, Line primitive, length %f\n", ident->str, value);
					glPushMatrix();
					glLineWidth(2.0);	  
					GLfloat model_ambient[] = {data.prim.r,data.prim.g,data.prim.b,1.0};
					glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
					
					glColor3f(data.prim.r,data.prim.g,data.prim.b);
					glBegin(GL_LINES);
					glVertex3f(0, 0, 0);
					glVertex3f(0, value, 0);
					glEnd();
					glPopMatrix();
					glTranslatef(0, value, 0);
				}; break;
				case PRIMITIVE_CYLINDER:
				{
#if 1
					{
						glPushMatrix(); 
						GLUquadricObj *cylquad = gluNewQuadric();
						glColor3f(data.prim.r,data.prim.g,data.prim.b);
						gluQuadricDrawStyle(cylquad, GLU_FLAT);
						gluQuadricNormals(cylquad, GLU_FLAT);
						glRotatef(-90,1,0,0);
						gluCylinder(cylquad,0.05,0.5*0.075*value,0.75*value,10,2);
						glPopMatrix();
					}
#endif
					glTranslatef(0, 0.75*value, 0);
				}; break;
				case PRIMITIVE_DISK:
				{
					glPushMatrix();
					GLUquadricObj *diskquad = gluNewQuadric();
					glColor3f(data.prim.r, data.prim.g, data.prim.b);
					gluQuadricDrawStyle(diskquad, GLU_FLAT);
					gluQuadricNormals(diskquad, GLU_FLAT);

					gluDisk(diskquad,0, 0.5*0.075*value,10,10);
					glPopMatrix();
				}break;
				case PRIMITIVE_NOTHING:
				break;
				default:
				{
					g_warning("Unknown primitive type 0x%x !", data.prim.type);
				}; break;
			}
		}
			break;
		default: 
			g_error("FATAL: In drawing %s: Unknown data type 0x%x !", ident->str, type);
    };
}

/**
 Prints the function name (e.g. "F(t)", parameter is always "t") into a GString for later output. 
 */
void Function::Print(GString *target)
{
	gchar name[255];
	g_snprintf(name, 255, " %s(t)", ident->str);
	
	g_string_append(target, name);
}

void Function::SpaceTransformData(int _affect_field)
{
	if (_affect_field & 0x777 == 0)
    {
		g_error("Empty affect field if using 0x%x\n", _affect_field);
    }
	else
		data.space_transform.affect_field = _affect_field;
}

/**
 Sets the primitive data. 
 _type is the type of the primitive
 */
void Function::PrimitiveData(int _type, float _r_color, float _g_color, float _b_color)
{
	if (_type > 0x5)
	{
		g_error("Error: Invalid prim_type 0x%x\n", _type);
	}
	else
    {
		data.prim.type = _type;
		data.prim.r = _r_color;
		data.prim.g = _g_color;
		data.prim.b = _b_color;
    }
}



