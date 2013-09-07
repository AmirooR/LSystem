/*
 *  main.cpp
 *  
 *
 *  Created by amir rahimi on 4/3/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <GLUT/GLUT.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <glib.h>

#define MODE_ROTATE 0
#define MODE_ZOOM 1

#include "LSystem.h"

void drawCoordinate()
{

	glBegin(GL_LINES);

	glColor3f(1,0,0);
	glVertex3f(0,0,0);
	glVertex3f(1,0,0);
	
	glColor3f(0,1,0);
	glVertex3f(0,0,0);
	glVertex3f(0,1,0);

	glColor3f(0,0,1);
	glVertex3f(0,0,0);
	glVertex3f(0,0,1);

	glEnd();
}

float rotX=10,rotY=0,zoom=2;

int mx,my,mode=0;

int level; //LSystem level

LSystem lsys(30);

void draw() 
{
	glMatrixMode(GL_MODELVIEW);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	
	glPushMatrix();
	
		
	glRotatef(rotX,1,0,0);
	glRotatef(rotY,0,1,0);
	
	glPushMatrix();
	glTranslatef(3,0,0);
	drawCoordinate();
	glPopMatrix();

	glScaled(zoom,zoom,zoom);
	
	glPushMatrix();
	lsys.DrawLevel(level);
	glPopMatrix();
	
	glPopMatrix();
	glutSwapBuffers();
}

void idle(void) 
{
	lsys.Animate();
	glutPostRedisplay();
}

void create_frustum(int width, int height)
{
	GLfloat h = (GLfloat) height / (GLfloat) width;
	glFrustum(-1.0, 1.0, -h, h, 5.0, 500.0);
}

void reshape(int width, int height) 
{
	glViewport(0, 0, (GLint) width, (GLint) height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	create_frustum(width, height);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glTranslatef(0.0, -3.0, -50.0);
}

void key(unsigned char c, int a, int b) 
{
	switch (c)
    {
		case 27:
		{
			exit(0);
		}; break;
		case '+':
			if (level < lsys.max_level)
				level++;
			break;
		case '-':
			if (level > 0)
				level--;
			break;
		case 'z':
			zoom+=.1;
			break;
		case 'Z':
			zoom-=.1;
			break;
			
    }
}

void mouse(int mouse, int state, int x, int y) 
{
	switch(mouse)
    {
		case GLUT_LEFT_BUTTON:
		{
			mode=MODE_ROTATE;
			mx=x;
			my=y;
		}
			break; 
		case GLUT_RIGHT_BUTTON: 
		{
			mode=MODE_ZOOM;
			mx=x;
			my=y;
		}
			break; 
    }
}

void motion(int x, int y) 
{
	if (mode==MODE_ROTATE) 
    {
		rotY+=(x-mx)*0.2;
		rotX+=(y-my)*0.2;
		if (rotX>90) rotX=90;
		if (rotX<0) rotX=0;
		//printf("rotY, rotX, y, my: %f, %f, %d, %d\n", rotY, rotX, y, my);
    } 
	
	if (mode==MODE_ZOOM) 
    {
		zoom+=(my-y)*0.004;
		if (zoom<0.1) zoom=0.1;
    }
	mx=x;
	my=y;
}

void parse_log_handler(const gchar *log_domain,
					   GLogLevelFlags log_level,
					   const gchar *message,
					   gpointer user_data)
{
	//printf("Parse returned: %s", message);
}

void parse_command_log_handler(const gchar *log_domain,
							   GLogLevelFlags log_level,
							   const gchar *message,
							   gpointer user_data)
{
	//printf("Command parser:%s", message);
}

void Rule_log_handler(const gchar *log_domain,
					  GLogLevelFlags log_level,
					  const gchar *message,
					  gpointer user_data)
{
	//printf("Rule:%s", message);
}

void Function_log_handler(const gchar *log_domain,
						  GLogLevelFlags log_level,
						  const gchar *message,
						  gpointer user_data)
{
	// printf("Rule:%s", message);
}

void lsys_log_handler(const gchar *log_domain,
					  GLogLevelFlags log_level,
					  const gchar *message,
					  gpointer user_data)
{
	printf("lsys:%s", message);
}

//Initializes stuff
void init() 
{
#if 1
	GLfloat mat_ambient[] = {0.8,0.15,0.05,1.0};
	GLfloat mat_specular[] = {0.8,0.15,0.04,1.0};
	GLfloat mat_shininess[] = {10.0};
	GLfloat light_position[] = {10.0,10.0,1.0,1.0 };
	GLfloat model_ambient[] = {0.8,0.15,0.04,1.0 };
	GLfloat mat_diffuse[] = {0.8,0.15,0.05,1.0 };
#endif	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glClearColor(1, 1, 1, 0);
#if 1
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
#endif 
	
	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_AUTO_NORMAL);
	glEnable(GL_NORMALIZE);
	
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_BLEND);
	
	g_log_set_handler ("parse", (GLogLevelFlags)(G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE),
					   parse_log_handler, NULL);
	g_log_set_handler ("parse_command", (GLogLevelFlags)(G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE), 
					   parse_command_log_handler, NULL);
	g_log_set_handler ("Rule", 
					   (GLogLevelFlags)(G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE), 
					   Rule_log_handler, NULL);
	g_log_set_handler ("Function", 
					   (GLogLevelFlags)(G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE), 
					   Function_log_handler, NULL);
	g_log_set_handler ("lsys", 
					   (GLogLevelFlags)(G_LOG_LEVEL_INFO | G_LOG_LEVEL_DEBUG | G_LOG_LEVEL_MESSAGE), 
					   lsys_log_handler, NULL);
	
	lsys.AddFunction_SpaceTransform("rx", 0x1, 0x0, 0x0); // create a rotation function around z 
	//lsys.AddFunction_SpaceTransform("ry", 0x2, 0x0, 0x0); // create a rotation function around y
	//lsys.AddFunction_SpaceTransform("G", 0x0, 0x2, 0x0); // create a translation function along x 
	lsys.AddFunction_Primitive("F", PRIMITIVE_LINE, 0.160, 0.048, 0.01); // create a line function
	lsys.AddFunction_Primitive("X", PRIMITIVE_LINE, 0.160, 0.048, 0.01);
	lsys.AddFunction_Primitive("G", PRIMITIVE_LINE, 0.160, 0.047, 0.02); // create a line function
	lsys.AddFunction_Primitive("H", PRIMITIVE_LINE, 0.160, 0.048, 0.01);
	lsys.AddFunction_Primitive("K", PRIMITIVE_LINE, 0.160, 0.048, 0.01);

	lsys.PrintFunctionSet();
	lsys.SetResult(0, "F(0)");
	//lsys.AddRule("F(t),*,*,[]:<0.5> G(t) [ rz(25) F(t) ] [ rz(-25) F(t) ] G(t) F(t) <> G(t) [ rz(5) F(t) ] [ rz(-5) F(t) ] G(t) F(t)");
	//lsys.AddRule("F(t),*,*,[]:<0.1> G(t) [ ry(60) rz(45) F(t*0.5) ] [ ry(-60) rz(15) F(t*0.5) ] [ ry(-180) rz(25) F(t*0.5) ] <> G(t) [ ry(60) rz(25) F(t*0.5) ] [ ry(-60) rz(25) F(t*0.5) ] [ ry(-180) rz(25) F(t*0.5) ] ");
	/*lsys.AddRule("F(t),*,*,[]:<0.1> G(t) [ ry(60) rz(45) F(t*0.5) ] [ ry(-60) rz(15) F(t*0.5) ] [ ry(-180) rz(25) F(t*0.5) ] <> G(t) [ ry(60) rz(25) F(t*0.5) ] [ ry(120) rz(25) F(t*0.5) ] [ ry(180) rz(25) F(t*0.5) ] ");*/
	//lsys.AddRule("X(t),*,*,[]:<> F(t*0.5) rx(-22.5) [[X(t*0.5)] rx(22.5)] rx(22.5) F(t*0.5) [rx(22.5) F(t*0.5) X(t*0.5)] rx(-22.5) X(t*0.5)");
	//lsys.AddRule("F(t),*,*,[]: <0.5> X(t)  [rx(50)  F(t*0.75)] [rx(-10) F(t*0.75)] <> X(t) [rx(-50) F(t*0.75)] [rx(10) F(t*0.75)]");
	/*lsys.AddRule("F(t),*,*,[t>1]:<> G(t)[rx(45) F(t*0.75)]");
	lsys.AddRule("F(t),*,*,[t<=1]:<> G(t)[rx(-45) F(t*0.75)]");
	lsys.AddRule("G(t),*,*,[]: <> G(t*1.1)");*/
	//lsys.AddRule("G(t),*,*,[t<=0]:<> X(1) [rx(45) F(6)] X(1) rx(3) H(0)");
	//lsys.AddRule("X(t),*,*,[]:<> X(t*1.15)");
	//lsys.AddRule("H(t),*,*,[t<=0]:<> X(1) [rx(-45) K(6)] X(1) rx(3) G(0)");
	//lsys.AddRule("K(t),*,*,[t>0]:<> K(t-1) H(t-1)");
	lsys.AddRule("F(t),*,*,[t>0]:<> F(t-.1)"); 
	lsys.AddRule("F(t),*,*,[t<=0]:<> X(.1) [rx(45) F(.6)] X(.1) rx(3) K(0)");
	lsys.AddRule("X(t),*,*,[]:<> X(t*1.15)");
	lsys.AddRule("K(t),*,*,[t<=0]:<> X(.1) [rx(-45) K(.6)] X(.1) rx(3) F(0)");
	lsys.AddRule("K(t),*,*,[t>0]:<> K(t-.1) ");

	//lsys.AddRule("G(t),*,*,[]:<> G(t*1.1)");
	
}	

int main(int argc, char **argv) 
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
	
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(800, 600);
	glutCreateWindow("Simple L-System experiment");
	init();
	glutDisplayFunc(draw);
	glutReshapeFunc(reshape);
	glutKeyboardFunc(key);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutVisibilityFunc(NULL);
	glutIdleFunc(idle);
	
	glutMainLoop();
	return 0;
}




