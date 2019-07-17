/************************************************************
Name: Hechun Wang
NetId: hw1964
N11482140
**************************************************************/
#include "Angel-yjc.h"
#include "CheckError.h"
#include "mat-yjc-new.h"
#include "vec.h"
#include "glut.h"
#include "glew.h"
#include <fstream>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string>
#include <vector> 
#include <time.h>  
using namespace std;

typedef Angel::vec4  color4; // define new type for storing point colors vec4
typedef Angel::vec4  point4; // define new type for storing point transform vec4

// data structure for storing triangle coordinates and radius from the files
struct Node {
	double data[3];
	struct Node *next;
};
struct Node** trianglearray = NULL;

GLuint Angel::InitShader(const char* vShaderFile, const char* fShaderFile); // initialize shader
GLuint ModelView, Projection; // model view and projection id
GLuint program;        /* shader program object id using vshader53.glsl and fshader53.glsl */
GLuint confettiprogram; /* shader program object id using vconfetti.glsl and fconfetti.glsl */

// buffers for each object in the frame
GLuint sphere_buffer;  /* vertex buffer object id for sphere */
GLuint shadow_buffer;  /* vertex buffer object id for shadow */
GLuint floor_buffer;   /* vertex buffer object id for floor */
GLuint axis_buffer_x;  /* vertex buffer object id for x axis */
GLuint axis_buffer_y;  /* vertex buffer object id for y axis */
GLuint axis_buffer_z;  /* vertex buffer object id for z axis */
GLuint confetti_buffer;  /* vertex buffer object id for confettis */

// points in the trail that the ball needs to change directions
vec3 pointA = vec3(-4, 1, 4); // starting point of the ball
vec3 pointB = vec3(-1, 1, -4); // second point of the trail
vec3 pointC = vec3(3, 1, 5); // third point of the trail

// Projection transformation parameters
GLfloat  fovy = 45.0;  // Field-of-view in Y direction angle (in degrees)
GLfloat  aspect;       // Viewport aspect ratio
GLfloat  zNear = 0.5, zFar = 18.0;                                                                                                

GLfloat angle = 0.0; // rotation angle in degrees
vec4 init_eye(7.0, 3, -10.0, 1.0); // initial viewer position, the four coordinates are x,y,z and distance?
vec4 eye = init_eye;               // current viewer position

// flags for switching displays
int animationFlag = 0; // 1: animation; 0: non-animation. Toggled by right mouse
int playanimation = 0; // 1: play animation; 0: non-animation. Toggled by 'b' or 'B' 
int sphereFlag = 0;   // 1: solid cube; 0: wireframe cube. Toggled by key 'c' or 'C'
int floorFlag = 1;  // 1: solid floor; 0: wireframe floor. Toggled by key 'f' or 'F'
int shadowFlag = 1; // 1: enable shadow; 0: disable shadow. Toggled by menu
int lightingFlag = 1; //1: enable lighting; 0: disable lighting. Toggled by menu
int shadingFlag = 1; // 1: smooth shading; 0: flat shading. Toggled by menu
int lightFlag = 1; // 1: spot light; 0: point source. Toggled by menu
int fogFlag = 0; // 0: no fog; 1: linear fog; 2: linear fog; 3: exponential fog; 4: exponential square fog
int shadow_blendingFlag = 1; //0: no blending 1: blending
int floor_textureFlag = 1; //-1: no texture; 1: checkered texture
int sphere_texture_mappingFlag = 0; // 0: no texture: 1: contour lines 2:checker board 
int contour_lineFlag = 0; // 0: vertical 1: slanted
int countour_frame = 0; // 1: object frame 2: eye frame
int confettiFlag = 0; // 0: no confetti; 1: start confetti animation
int latticeFlag = 0; // 0: lattice effect off; 1: lattice effect on
int latticeType = 0; // 0:upright 1: tilted

// parameters for sphere generation
int num = 0; // total number oF triangles
int sphere_NumVertices = 0; //total number of vertices in the sphere * 3
int Index = 0;
point4 sphere_points[1024*3]; // array of positions assigned to each triangle vertex
vec3 normals[1024*3]; // array of color assigned to the normal vector in each triangle
// shadow generation

point4 shadow_points[1024 * 3]; // array of positions assigned to the shadow vertex
color4 shadow_colors[1024 * 3]; // array of shadow points assigned to the shadow vertex

// floor related configurations
int floor_NumVertices = 6; //(1 face)*(2 triangles/face)*(3 vertices/triangle)
point4 floor_points[6]; // positions for all vertices
vec3 floor_normals[6]; // normals for all vertices
point4 floor_vertices[4] = {
	point4(5, 0, 8, 1),
	point4(5, 0, -4, 1),
	point4(-5, 0, 8, 1),
	point4(-5, 0, -4, 1)
};

// axis related configurations
int axis_NumVertices = 2;

color4 axis_colors_x[2];
point4 axis_points_x[2] = {
	point4(0,0,0,1),
	point4(10,0,0,1),
};

color4 axis_colors_y[2];
point4 axis_points_y[2] = {
	point4(0,0,0,1),
	point4(0,10,0,1),
};

color4 axis_colors_z[2];
point4 axis_points_z[2]= {
	point4(0,0,0,1),
	point4(0,0,10,1),
};

int confetti_Num = 300;
color4 confetti_colors[300];
point4 confetti_points[300];
vec3 confetti_velocity[300];
float largest_confetti_y = 0;
int start_confetti = 1; // 0: isn't the start of the confetti animation 1: the start of the confetti animation
float animation_time = 0;

// parameters for sphere trail
double t_x = -4, t_y = 1, t_z = 4; // rolling direction vector
double r_x = -8, r_y = 0, r_z = -3; // spinnning direction vector
int direction = 1; // current direction in the trail; 1: a to b. 2: b to c. 3: c to a.
mat4 M = mat4(1); // matrix to get ball rotation accumulation, make sure rotation doesn't get refreshed

// global parameters for light attenuation
color4 global_light_ambient(1, 1, 1, 1.0); // global ambient light with white ambient color
color4 distant_light_ambient(0, 0, 0, 1); //distant light source light ambient
color4 distant_light_diffuse(0.8, 0.8, 0.8, 1); //distant light source light diffuse
color4 distant_light_specular(0.2, 0.2, 0.2, 1.0); // distant light source light specular
vec4 distant_light_direction(0.1, 0, -1, 0); // distant light source direction

/* this is the light source which we can assign optional type*/
point4 light_optional_position(-14, 12, -3, 1);
color4 light_optional_ambient(0, 0, 0, 1);
color4 light_optional_diffuse(1, 1, 1, 1);
color4 light_optional_specular(1, 1, 1, 1);
vec4 light_optional_direction(8, -12, -1.5, 0);
float exp1 = 15;
float cutoffangle = 20;
float const_att = 2.0;
float linear_att = 0.01;
float quad_att = 0.001;



/* global definitions for constants and global image arrays */

#define ImageWidth  32
#define ImageHeight 32
GLubyte Image[ImageHeight][ImageWidth][4];
static GLuint floor_texture;

#define	stripeImageWidth 32
GLubyte stripeImage[4 * stripeImageWidth];
static GLuint sphere_texture;

vec2 floor_texCoord[6] = { // draw a square quad
 // upper half
  vec2(0.0, 0.0),  // for a
  vec2(0.0, 6.0),  // for b
  vec2(5.0, 6.0),  // for c
  // bottom half
  vec2(5.0, 6.0),  // for c
  vec2(5.0, 0.0),  // for d
  vec2(0.0, 0.0),  // for a 
};

// functions declaration
void SetUp_Lighting(mat4 mv, color4 ambient_product, color4 diffuse_product, color4 specular_product, vec4 lightdirection, float shininess);
void SetUp_Lighting_optional(mat4 mv, color4 ambient_product, color4 diffuse_product, color4 specular_product, float shininess);
mat4 shadow_projection(mat4 mv, point4 light_position); // project a ball as a shadow
void file_in(int);
void myMouse(int button, int state, int x, int y);
void createMenu();
void move(void);
void colorsphere();
void floor();
void drawaxis(vec4 at, vec4 up);
void drawsphere(vec4 at, vec4 up);
void drawshadow(vec4 at, vec4 up);
void drawfloor(vec4 at, vec4 up);
void flat_normal();
void smooth_normal();
void image_set_up(void);
void confetti_setup(void);
void draw_confetti();


// OpenGL initialization, create vertex array and color(normal) array for each object to be used in display()
void init()
{
	colorsphere(); // Create and initialize a vertex buffer object and a color(normal) buffer object for the sphere
	floor(); // fill floor arrays ,point array and vertex array
	image_set_up(); // set up the texture
	confetti_setup(); // set up confetti vertex buffer

	/*--- Create and Initialize a texture object for the 2d mapping for the checkerboard ---*/
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &floor_texture);      // Generate texture obj name(s)
	glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to be 0 in this texture unit
	glBindTexture(GL_TEXTURE_2D, floor_texture); // Bind the texture to this texture unit
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, ImageWidth, ImageHeight,0, GL_RGBA, GL_UNSIGNED_BYTE, Image); //first 0 means the levels of details = 0,

		/*--- Create and Initialize a texture object for the 1d mapping for the sphere ---*/
	
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &sphere_texture);
	glActiveTexture(GL_TEXTURE0);  // Set the active texture unit to be 0 
	glBindTexture(GL_TEXTURE_1D, sphere_texture); // Bind the texture to this texture unit
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, stripeImageWidth, 0, GL_RGBA, GL_UNSIGNED_BYTE, stripeImage); //first 0 means the levels of details = 0,
	

	//create and initialize a vertex buffer object and a color(normal) buffer object for the shadow
	glGenBuffers(1, &shadow_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, shadow_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(shadow_points) + sizeof(shadow_colors),NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(shadow_points), shadow_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(shadow_points),	sizeof(shadow_colors), shadow_colors);

	// Create and initialize a vertex buffer object and a color(normal) buffer object for the floor
	
	glGenBuffers(1, &floor_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, floor_buffer);
	glBufferData(GL_ARRAY_BUFFER, (sizeof(floor_points) + sizeof(floor_normals) + sizeof(floor_texCoord)),NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floor_points), floor_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points), sizeof(floor_texCoord), floor_texCoord);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(floor_points)+sizeof(floor_texCoord), sizeof(floor_normals), floor_normals);

	// Create and initialize a vertex buffer object and a color(normal) buffer object for x axis
	glGenBuffers(1, &axis_buffer_x);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer_x);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points_x) + sizeof(axis_colors_x), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points_x), axis_points_x);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points_x), sizeof(axis_colors_x), axis_colors_x);

    // Create and initialize a vertex buffer object  and a color(normal) buffer object for y axis, to be used in display()
	glGenBuffers(1, &axis_buffer_y);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer_y);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points_y) + sizeof(axis_colors_y), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points_y), axis_points_y);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points_y), sizeof(axis_colors_y), axis_colors_y);

    // Create and initialize a vertex buffer object and a color(normal) buffer object for z axis, to be used in display()
	glGenBuffers(1, &axis_buffer_z);
	glBindBuffer(GL_ARRAY_BUFFER, axis_buffer_z);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axis_points_z) + sizeof(axis_colors_z), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(axis_points_z), axis_points_z);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(axis_points_z), sizeof(axis_colors_z), axis_colors_z);


	// Create and initialize a vertex buffer object and a color buffer object for confetti
	glGenBuffers(1, &confetti_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, confetti_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(confetti_points) + sizeof(confetti_colors)+sizeof(confetti_velocity), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(confetti_points), confetti_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(confetti_points), sizeof(confetti_velocity), confetti_velocity);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(confetti_points)+sizeof(confetti_velocity), sizeof(confetti_colors), confetti_colors);

	
	// Load shaders and create a shader program (to be used in display())
	program = InitShader("vshader53.glsl", "fshader53.glsl");
	confettiprogram = InitShader("vconfetti.glsl", "fconfetti.glsl");
	glEnable(GL_DEPTH_TEST); // enable z-buffer testing
	glClearColor(0.529, 0.807, 0.92, 0.0);
	glLineWidth(1.5); // for the wireframe and x,y,z axis
	glPointSize(3.0); // for the confetti particle system
}

//   draw the object that is associated with the vertex buffer object "buffer" and has "num_vertices" vertices.
void drawObj(GLuint buffer, int num_vertices)
{
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute -----*/
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint vNormal = glGetAttribLocation(program, "vNormal");
	glEnableVertexAttribArray(vNormal);

	GLuint vTexCoord = glGetAttribLocation(program, "floorTextureCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(vTexCoord, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(floor_points)));

	GLuint sphereTexCoord = glGetAttribLocation(program, "sphereTextureCoord");
	glEnableVertexAttribArray(vTexCoord);
	glVertexAttribPointer(sphereTexCoord, 1, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(sphere_points)));

	// the offset is the (total) size of the previous vertex attribute array(s)

  /* Draw a sequence of geometric objs (triangles) from the vertex buffer
	 (using the attributes specified in each enabled vertex attribute array) */
	if (num_vertices >= 7)//draw sphere
	{
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(sphere_points)));
		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	}
	else if (num_vertices <= 6 && num_vertices >= 3)
	{
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(floor_points)+sizeof(floor_texCoord)));
		glDrawArrays(GL_TRIANGLES, 0, num_vertices);
	}
	else if (num_vertices>1 && num_vertices<=2) // draw a line from two points
	{
		glVertexAttribPointer(vNormal, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(axis_points_x)));
		glDrawArrays(GL_LINE_STRIP, 0, num_vertices);
	}


	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vNormal);
    glDisableVertexAttribArray(vTexCoord);
	glDisableVertexAttribArray(sphereTexCoord);



}

// main function for drawing each object, everything happens here
void display(void)
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(program); // Use the shader program
	ModelView = glGetUniformLocation(program, "ModelView"); // pass modelview matrix to vshader
	Projection = glGetUniformLocation(program, "Projection"); // pass projection matrix to vshader

	/*---  Set up and pass on Projection matrix to the shader ---*/
	mat4  p = Perspective(fovy, aspect, zNear, zFar);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major

/*---  Set up and pass on Model-View matrix to the shader ---*/
	vec4    at(0, 0, 0, 1);
	vec4    up(0.0, 1.0, 0.0, 0.0); // the four parameters are x,y,z and a zero for indicating this is a vector, the fourth value is not really needed nor has any actual use
	mat4  mv = LookAt(eye, at, up);	// eye is a global variable of vec4 set to init_eye and updated by keyboard()


	glEnable(GL_DEPTH_TEST); // always enable z-buffer testing
	glDepthMask(GL_FALSE); // disable writing to the z-buffer

	// draw ground only to the frame buffer
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // enable writing to the frame buffer

	glUniform1i(glGetUniformLocation(program, "fogFlag"), fogFlag); // decide if we draw fog or not

	drawfloor(at,up); // draw floor only to frame buffer
	if (shadow_blendingFlag == 0)
	{
		glDepthMask(GL_TRUE); // enable writing to z-buffer
	}
	
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // enable writing to the frame buffer
	//draw shadow to both buffer
	if (shadow_blendingFlag == 1)
	{
		glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	drawshadow(at, up);
	if (shadow_blendingFlag == 1)
	{
		glDisable(GL_BLEND);
	}

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE); // disable writing to frame buffer

	glDepthMask(GL_TRUE); // enable writing to z-buffer
   //draw ground only to z-buffer

	drawfloor(at,up); // draw the floor

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE); // enable frame buffer
	glEnable(GL_DEPTH_TEST); // enable z- buffer


   // resume normal operations

	drawaxis(at, up);// draw the axis

	drawsphere(at, up); // draw the sphere


	glUseProgram(confettiprogram); // Use the confetti shader program
	ModelView = glGetUniformLocation(confettiprogram, "ModelView"); // pass modelview matrix to vshader
	Projection = glGetUniformLocation(confettiprogram, "Projection"); // pass projection matrix to vshader
	glUniformMatrix4fv(Projection, 1, GL_TRUE, p); // GL_TRUE: matrix is row-major
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, LookAt(eye, at, up)); // GL_TRUE: matrix is row-major
	if (confettiFlag == 1) // if we choose to draw confetti
	{
		if (start_confetti == 1) // if this is the start of a confetti animation
		{
			animation_time = (float)glutGet(GLUT_ELAPSED_TIME); // store starting time of current confetti animation
		}
		draw_confetti(); // draw the confetti;
		start_confetti = 0;

		// test for new animation cycle
		float a = -0.00000049;
		float t = (float)glutGet(GLUT_ELAPSED_TIME) - animation_time;
		float f_y = 0.1 + 0.001 * largest_confetti_y * t + 0.5 * a * t * t;
		if (f_y < 0)
		{
			cout << t;
		  start_confetti = 1; // set new round of animation
		}
	}


	glutSwapBuffers();

}
// change the value of angle to calculate moving parameters and show animation
void idle(void) 
{
	if (animationFlag == 1)
	{
		
		    //YJC: change this value to adjust the cube rotation speed.

		angle += 0.1;
		move();
		glutPostRedisplay();
	}

}
// keyboard function, assign keys with their functions
void keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033: // Escape Key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;

	case 'X': eye[0] += 1.0; break;
	case 'x': eye[0] -= 1.0; break;
	case 'Y': eye[1] += 1.0; break;
	case 'y': eye[1] -= 1.0; break;
	case 'Z': eye[2] += 1.0; break;
	case 'z': eye[2] -= 1.0; break;

	case 'b': case 'B': // Toggle between animation and non-animation
		playanimation = 1;
		animationFlag = 1;
		break;

	case 'c': case 'C': // Toggle between filled and wireframe cube
		sphereFlag = 1 - sphereFlag;
		break;

	case 'f': case 'F': // Toggle between filled and wireframe floor
		floorFlag = 1 - floorFlag;
		break;

	case ' ':  // reset to initial viewer/eye position
		eye = init_eye;
		break;

	case 'v': case 'V': // switch sphere contour line texture to vertical 
		contour_lineFlag = 0;
		break;
	case 's': case 'S': // switch sphere contour line texture to slanted object space
		contour_lineFlag = 1;
		break;
	case 'o': case 'O': // switch sphere contour line texture to eye space
		countour_frame = 0;
		break;
	case 'e': case 'E': // switch sphere contour line texture to vertical object space
		countour_frame = 1;
		break;
	case 'u': case 'U': // switch sphere contour line texture to vertical object space
		latticeType = 0;
		break;
	case 't': case 'T': // switch sphere contour line texture to vertical object space
		latticeType = 1;
		break;
	case 'l': case 'L': // switch sphere contour line texture to vertical object space
		latticeFlag = 1-latticeFlag;
		break;

	}
	glutPostRedisplay();
}

// reshape function, deals with when user enlarges of shrinks the openGL window
void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
	aspect = (GLfloat)width / (GLfloat)height;
	glutPostRedisplay();
}

// main function, opens the program
int main(int argc, char **argv)
{

	glutInit(&argc, argv);

	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(512, 512);
	glutCreateWindow("Homework 4");
	/* Call glewInit() and error checking */
	int err = glewInit();
	if (GLEW_OK != err)
	{
		printf("Error: glewInit failed: %s\n", (char*)glewGetErrorString(err));
		exit(1);
	}

	// Get info of GPU and supported OpenGL version
	printf("Renderer: %s\n", glGetString(GL_RENDERER));
	printf("OpenGL version supported %s\n", glGetString(GL_VERSION));

	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(myMouse);

	init();
	createMenu();
	glutMainLoop();
	return 0;
}

// read the triangles' coordinates and and store them in an array list
void file_in(int num1) 
{
	string filename = "sphere.256";
	if (num1 == 2)
	{
		filename = "sphere.1024";
	}
	int i = 0;

	std::ifstream file(filename);
	if (file.is_open()) {
		std::string line;
		while (getline(file, line)) {
			if (i == 0) // get the first line of the file, which is how many circles there are and their coordinates and radius
			{
				num = atoi(line.c_str());
				trianglearray = new struct Node*[num]; // initialize the arraylist
				i++;
			}
			else if (i % 4 == 1)
			{
				struct Node* n = (struct Node*) malloc(sizeof(struct Node)); // new node
				struct Node* head = n;
				for (int j = 0; j < 3; j++)
				{
					getline(file, line);
					char* pch = strtok((char*)line.c_str(), " "); // get line
					int k = 0;
					while (pch != NULL) // split line and store in node list
					{
						n->data[k] = atof(pch);
						k++;
						pch = strtok(NULL, " ");

					}
					struct Node* pp = (struct Node*) malloc(sizeof(struct Node));
					n->next = pp;
					n = n->next;
					trianglearray[i / 4] = head; // put node list in the arraylist
					i++;
				}
				i++;
			}
		}
		file.close();
	}
}

// menu function, use the id of the selected item to show different features
void top_menu(int id)
{
	switch (id)
	{
	case 1: // return view eye position to the initial position
		eye = init_eye;
		animationFlag = 1 - animationFlag;
		if (animationFlag == 1) glutIdleFunc(idle);
		else                    glutIdleFunc(NULL);
		break;
	case 2: // exit the program
		exit(0);
		break;
	case 3:// don't create shadow
		shadowFlag = 0;
		break;
	case 4:// create the correct shadow
		shadowFlag = 1;
		break;
	case 5:// disable lighting, render floor with only ambient color
		lightingFlag = 0;
		break;
	case 6:// enable lighting, render floor with shading
		lightingFlag = 1;
		break;
	case 7:// set sphere to wireframe
		sphereFlag = 1;
		break;
	case 8:// flat shading of the sphere
		shadingFlag = 0;
		sphereFlag = 0;
		flat_normal();
		break;
	case 9:// smooth shading of the sphere
		shadingFlag = 1;
		sphereFlag = 0;
		smooth_normal();
		break;
	case 10:// set light source to spotlight 
		lightFlag = 1;
		break;
	case 11:// set light source to point source
		lightFlag = 0;
		break;
	case 12:// set fog to no fog
		fogFlag = 0;
		break;
	case 13:// set fog to linear
		fogFlag = 1;
		break;
	case 14:// set fog to exponential
		fogFlag = 2;
		break;
	case 15:// set fog to exponential square
		fogFlag = 3;
		break;
	case 16: // no shadow blending
		shadow_blendingFlag = 0;
		break;
	case 17:// shadow blending
		shadow_blendingFlag = 1;
		break;
	case 18:// no texture for the floor
		floor_textureFlag = -1;
		break;
	case 19:// checkered texture for the floor
		floor_textureFlag = 1;
		break;
	case 20:// no texture for the sphere
		sphere_texture_mappingFlag = 0;
		break;
	case 21:// 1D contour line texture  for the sphere
		sphere_texture_mappingFlag = 1;
		break;
	case 22:// 2D checker board texture for the sphere
		sphere_texture_mappingFlag = 2;
		break;
	case 23:// disable confetti animation
		confettiFlag = 0;
		start_confetti = 1;
		break;
	case 24:// enable confetti animation
		confettiFlag = 1;
		break;
		

	}

	glutPostRedisplay();
}

// mouse function, binds functions to either key of the mouse
void myMouse(int button, int state, int x, int y)
{
	if (button == GLUT_RIGHT_BUTTON && state == GLUT_DOWN)
	{
		if (playanimation == 1)
		{

			animationFlag = 1 - animationFlag;
			if (animationFlag == 1) glutIdleFunc(idle);
			else                    glutIdleFunc(NULL);
		}

	}

}

// create a menu and binds it to the left mouse key
void createMenu()
{
	//creating a submenu for shadow options;
	int submenu_shadow = glutCreateMenu(top_menu);
	glutAddMenuEntry("No", 3);
	glutAddMenuEntry("Yes", 4);

	// creating a submenu for lighting options
	int submenu_lighting = glutCreateMenu(top_menu);
	glutAddMenuEntry("No", 5);
	glutAddMenuEntry("Yes", 6);

	// creating a sbumenu for shading options
	int submenu_shading = glutCreateMenu(top_menu);
	glutAddMenuEntry("flat shading", 8);
	glutAddMenuEntry("smooth shading", 9);

	// create a submenu for light source options
	int submenu_lightsource = glutCreateMenu(top_menu);
	glutAddMenuEntry("spot light", 10);
	glutAddMenuEntry("point source", 11);

	// create a submenu for light source options
	int submenu_fog = glutCreateMenu(top_menu);
	glutAddMenuEntry("no fog", 12);
	glutAddMenuEntry("linear", 13);
	glutAddMenuEntry("exponential", 14);
	glutAddMenuEntry("exponential square", 15);

	// create a submenu for shadow blending
	int submenu_shadow_blending = glutCreateMenu(top_menu);
	glutAddMenuEntry("no", 16);
	glutAddMenuEntry("yes", 17);

	// create a submenu for generating ground texture
	int submenu_ground_texture = glutCreateMenu(top_menu);
	glutAddMenuEntry("no", 18);
	glutAddMenuEntry("yes", 19);

	// create a submenu for generating sphere texture
	int submenu_sphere_texture = glutCreateMenu(top_menu);
	glutAddMenuEntry("No", 20);
	glutAddMenuEntry("Yes-Contour Lines", 21);
	glutAddMenuEntry("Tes-Checkerboard", 22);

	// create a submenu for generating confettis
	int submenu_confetti = glutCreateMenu(top_menu);
	glutAddMenuEntry("No", 23);
	glutAddMenuEntry("Yes", 24);

	glutCreateMenu(top_menu); // the main menu
	glutAddMenuEntry("Default View Point", 1); // add menu entry
	glutAddSubMenu("Shadow", submenu_shadow); // add a sub menu entry
	glutAddSubMenu("Enable Lighting", submenu_lighting); // add a sub menu entry
	glutAddMenuEntry("Wire frame sphere", 7); // add menu entry
	glutAddSubMenu("Shading", submenu_shading); // add submenu entry
	glutAddSubMenu("Light source", submenu_lightsource); // add submenu entry
	glutAddSubMenu("fog", submenu_fog); // add submenu entry
	glutAddSubMenu("Blending shadow", submenu_shadow_blending); // add submenu entry
	glutAddSubMenu("Texture mapped ground", submenu_ground_texture); // add submenu entry
	glutAddSubMenu("Texture mapped sphere", submenu_sphere_texture);
	glutAddSubMenu("Firework", submenu_confetti);
	glutAddMenuEntry("Quit", 2);
	glutAttachMenu(GLUT_LEFT_BUTTON);
}

// moving the sphere
void move()
{
	switch (direction)
	{
		
	case 1:
		if (abs(t_x-(-1)) <= 0.001 && abs(t_z-(-4))<=0.001)
		{
			direction = 2;
			M = Rotate(angle, r_x, r_y, r_z)*M;
			angle = 0;
		}
		else
		{
			float d = angle * (2 * 3.1415926 * 1) / 360;
			vec3 ab = pointB - pointA;
			float length = ab.x*ab.x+ab.y*ab.y+ab.z*ab.z;
			length = pow(length, 0.5);
			vec3 direct = pointA + d * (ab /length);

			t_x = direct.x;
			t_y = direct.y;
			t_z = direct.z;

			r_x = -8;
			r_y = 0;
			r_z = -3;
		}
		break;
	case 2:
		if (abs(t_x - (3)) <= 0.001 && abs(t_z -(5))<=0.001)
		{
			direction = 3;

			M = Rotate(angle, r_x, r_y, r_z) * M;
				angle = 0;
		}
		else
		{
			float d = angle * (2 * 3.1415926 * 1) / 360;
			vec3 bc = pointC - pointB;
			float length = bc.x* bc.x + bc.y* bc.y + bc.z* bc.z;
			length = pow(length, 0.5);
			vec3 direct = pointB + d * (bc / length);

			t_x = direct.x;
			t_y = direct.y;
			t_z = direct.z;
			r_x = 9;
			r_y = 0;
			r_z = -4;
		}
		break;
	case 3:
		if (abs(t_x - (-4)) <= 0.001 && abs(t_z - (4)) <= 0.001)
		{
			direction = 1;

			M = Rotate(angle, r_x, r_y, r_z)*M;
			angle = 0;
		}
		else
		{
			float d = angle * (2 * 3.1415926 * 1) / 360;
			vec3 ca = pointA - pointC;
			float length = ca.x*ca.x + ca.y*ca.y + ca.z*ca.z;
			length = pow(length, 0.5);
			vec3 direct = pointC + d * (ca / length);

			
			t_x = direct.x;
			t_y = direct.y;
			t_z = direct.z;
			r_x = -1;
			r_y = 0;
			r_z = 7;
		}
		break;
	}
	
}

//----------------------------------------------------------------------
// SetUp_Lighting_Uniform_Vars(mat4 mv):
// Set up lighting parameters that are uniform variables in shader.
//
// Note: "LightPosition" in shader must be in the Eye Frame.
//       So we use parameter "mv", the model-view matrix, to transform
//       light_position to the Eye Frame.
//----------------------------------------------------------------------


// matrix for shadow projection , we use this matrix to convert a sphere object to a flat shadow object
mat4 shadow_projection(mat4 mv, point4 light_position)
{
	vec4 light_position_eyeFrame =  light_position;
	float a = light_position_eyeFrame.x;
	float b = light_position_eyeFrame.y;
	float c = light_position_eyeFrame.z;
	mat4 sp = mat4(vec4(b, -a, 0, 0), vec4(0, 0, 0, 0),  vec4(0, -c, b, 0), vec4( 0, -1, 0, b));
	return sp;
};

// read file and populate the sphere and shadow arrays
void colorsphere()
{
	int p = 2;
	cout << "please input the selection of your choosing\n1. sphere.256\n2. shpere.1024\n";
	cin >> p;
	file_in(p);
	sphere_NumVertices = 3 * num; //number of triangles*(1 triangles / face)*(3 vertices / triangle)
	flat_normal();
	smooth_normal();
}

// use flat normal by generating a normal on the flat surface, gives a flat normal effect
void flat_normal()
{
	Index = 0;
	for (int j = 0; j < num; j++) // put vertices data into the buffers
	{
		struct Node* p = trianglearray[j];
		vec4 u = point4(p->next->data[0], p->next->data[1], p->next->data[2], 1) - point4(p->data[0], p->data[1], p->data[2], 1);
		vec4 v = point4(p->next->next->data[0], p->next->next->data[1], p->next->next->data[2], 1) - point4(p->data[0], p->data[1], p->data[2], 1);

		vec3 normal = normalize(cross(u, v));
		normals[Index] = normal;
		sphere_points[Index] = point4(p->data[0], p->data[1], p->data[2], 1);
		shadow_points[Index] = point4(p->data[0], p->data[1], p->data[2], 1);
		Index++;
		p = p->next;

		normals[Index] = normal;
		sphere_points[Index] = point4(p->data[0], p->data[1], p->data[2], 1);
		shadow_points[Index] = point4(p->data[0], p->data[1], p->data[2], 1);
		Index++;
		p = p->next;

		normals[Index] = normal;
		sphere_points[Index] = point4(p->data[0], p->data[1], p->data[2], 1);
		shadow_points[Index] = point4(p->data[0], p->data[1], p->data[2], 1);
		Index++;
	}

	glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_points) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sphere_points), sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sphere_points), sizeof(normals), normals);


	// Create and initialize a vertex buffer object and a color(normal) buffer object for floor

}

// use cop to vertex as normal, this gives a smooth normal effect
void smooth_normal() 
{
	Index = 0;
	for (int j = 0; j < num; j++) // put vertices data into the buffers
	{
		struct Node* p = trianglearray[j];
		vec3 u1 = vec3(p->data[0], p->data[1], p->data[2]);
		vec3 normal = normalize(u1);
		normals[Index] = normal;
		Index++;
		p = p->next;

		vec3 u2 = vec3(p->data[0], p->data[1], p->data[2]);
		normal = normalize(u2);
		normals[Index] = normal;
		Index++;
		p = p->next;

		vec3 u3 = vec3(p->data[0], p->data[1], p->data[2]);
		normal = normalize(u3);
		normals[Index] = normal;
		Index++;
	}	
	
		glGenBuffers(1, &sphere_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, sphere_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(sphere_points) + sizeof(normals), NULL, GL_STATIC_DRAW);
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(sphere_points), sphere_points);
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(sphere_points), sizeof(normals), normals);


}

// generate 2 triangles to make a floor: 6 vertices and 6 colors
void floor() 
{
	vec4 u = floor_vertices[1]-floor_vertices[0];
	vec4 v = floor_vertices[2]-floor_vertices[1];
	vec3 normal = normalize(cross(u,v));
	for (int i = 0; i < 6; i++)
	{
		floor_normals[i] = normal;
	}
	floor_points[0] = floor_vertices[3];
	floor_points[1] = floor_vertices[2];
	floor_points[2] = floor_vertices[0];
	floor_points[3] = floor_vertices[0];
	floor_points[4] = floor_vertices[1];
	floor_points[5] = floor_vertices[3];
}

// draw the axis in display in only ambient light
void drawaxis(vec4 at, vec4 up)
{
	// set the parameters
	mat4 mv = LookAt(eye, at, up) * Scale(1, 1, 1);
	glUniform1f(glGetUniformLocation(program, "cflag"), 0); // set to ambient
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	glUniform4fv(glGetUniformLocation(program, "vColor"),1, color4(1,0,0,1));
	drawObj(axis_buffer_x, axis_NumVertices);  // draw the X axis
	glUniform4fv(glGetUniformLocation(program, "vColor"), 1, color4(1, 0, 1, 1));
	drawObj(axis_buffer_y, axis_NumVertices);  // draw the Y axis
	glUniform4fv(glGetUniformLocation(program, "vColor"), 1, color4(0, 0, 1, 1));
	drawObj(axis_buffer_z, axis_NumVertices);  // draw the Z axis
	
	glUniform1f(glGetUniformLocation(program, "cflag"), 1); // set to shading

}

// draw sphere in display()
void drawsphere(vec4 at, vec4 up)
{


	color4 material_ambient(0.2, 0.2, 0.2, 1.0); // ambient color for the sphere
	color4 material_diffuse(1.0, 0.84, 0.0, 1.0); // diffuse color for the sphere
	color4 material_specular(1.0, 0.84, 0.0, 1.0); // specular color for the sphere
	float shininess = 125; // shininess coefficient for the sphere
	color4 ambient_product = distant_light_ambient * material_ambient;
	color4 diffuse_product = distant_light_diffuse * material_diffuse;
	color4 specular_product = distant_light_specular * material_specular;


	// optional light source product
	color4 ambient_product1 = light_optional_ambient * material_ambient;
	color4 diffuse_product1 = light_optional_diffuse * material_diffuse;
	color4 specular_product1 = light_optional_specular * material_specular;


	/********************************************************/

    /*----- Set Up the Model-View matrix for the SPHERE -----*/
	mat4 mv;
	mv = LookAt(eye, at, up) *  Scale(1, 1, 1);
	mv = mv * Translate(t_x, t_y, t_z) *Scale(1, 1, 1)*Rotate(angle, r_x, r_y, r_z)*M;
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv);
	if (lightingFlag == 1) //if the light is on
	{
		// moving the sphere, scaling the sphere and rotating the sphere along the X axis 

		SetUp_Lighting(LookAt(eye, at, up), ambient_product, diffuse_product, specular_product, distant_light_direction, shininess);
		SetUp_Lighting_optional(LookAt(eye,at,up), ambient_product1, diffuse_product1, specular_product1, shininess);
		mat3 normal_matrix = NormalMatrix(mv, 1);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"),1, GL_TRUE, normal_matrix);
		if (sphere_texture_mappingFlag == 1) // 1d mappping
		{
			glUniform1i(glGetUniformLocation(program, "f_shader_flag"), 0); // draw the sphere

		
		if (countour_frame == 0)
		{
			if (contour_lineFlag == 0)
			{
				glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 0);
			}
			else
			{
				glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 1);
			}
		}
		else if (countour_frame == 1)
		{
			if (contour_lineFlag == 0)
			{
				glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 2);
			}
			else
			{
				glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 3);
			}
		}

		glUniform1i(glGetUniformLocation(program, "sphere_texture_1D"), 0);

		}



		// 2D mapping
		if (sphere_texture_mappingFlag == 2)
		{
			glUniform1i(glGetUniformLocation(program, "f_shader_flag"), 2); // draw the sphere


			if (countour_frame == 0)
			{
				if (contour_lineFlag == 0)
				{
					glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 4);
				}
				else
				{
					glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 5);
				}
			}
			else if (countour_frame == 1)
			{
				if (contour_lineFlag == 0)
				{
					glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 6);
				}
				else
				{
					glUniform1i(glGetUniformLocation(program, "sphere_textureFlag"), 7);
				}
			}

			glUniform1i(glGetUniformLocation(program, "ground_texture_2D"), 0);
		}

		glUniform1i(glGetUniformLocation(program, "latticeFlag"), latticeFlag); 
		glUniform1i(glGetUniformLocation(program, "latticeType"), latticeType);
	
	}
  else // if light is not on
			{
			color4 flatcolor = color4(1.0, 0.84, 0, 1);  // golden yellow
			glUniform4fv(glGetUniformLocation(program, "vColor"), 1, flatcolor); // pass light color to vshader
			glUniform1f(glGetUniformLocation(program, "cflag"), 0); // set to ambient
			}
	if (sphereFlag == 1) // Wireframe cube
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	else              // Filled cube
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	drawObj(sphere_buffer, sphere_NumVertices);  // draw the sphere
	glUniform1f(glGetUniformLocation(program, "cflag"), 1); // set to shading
	glUniform1i(glGetUniformLocation(program, "f_shader_flag"), -1); // draw the sphere
	glUniform1i(glGetUniformLocation(program, "latticeFlag"), 0);
}

// draw the shadow in display()
void drawshadow(vec4 at, vec4 up)
{

	// lighting parameters
	point4 shadow_light_position(-14.0, 12.0, -3.0, 1.0); // light position only, no material or light parameters since we're drawing the shadow in an ambient color
	mat4 mv;
	mv = LookAt(eye, at, up); // set up eye frame matrix
	// order is from right to left, lookat matrix mv -> accumulate rotation -> rotate -> scale -> translate -> shadow projection
	// get final model-view matrix for shadow, this is used to map the shadow vertex array
	mv = mv * shadow_projection(LookAt(eye, at, up), shadow_light_position)* Translate(t_x, t_y, t_z) *  Scale(1, 1, 1)*Rotate(angle, r_x, r_y, r_z)*M;
	glUniform1f(glGetUniformLocation(program, "cflag"), 0); // set to shading
	glUniform4fv(glGetUniformLocation(program, "vColor"), 1, color4(0.25, 0.25, 0.25, 0.65));
	glUniform1i(glGetUniformLocation(program, "latticeFlag"), latticeFlag);
	glUniform1i(glGetUniformLocation(program, "latticeType"), latticeType);
	// moving the sphere, scaling the sphere and rotating the sphere along the X axis 
	glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // pass the mv matrix to the vshader
	if (sphereFlag == 1)
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wire frame shadow
	}
	else
	{
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // complete sphere shadow
	}

	if (shadowFlag == 1 && eye.y > 0) // don't draw shadow if user's eye is below the floor 
	{
		drawObj(shadow_buffer, sphere_NumVertices);  // draw the shadow
	}

	glUniform1f(glGetUniformLocation(program, "cflag"), 1); // set to shading
	glUniform1i(glGetUniformLocation(program, "latticeFlag"), 0);
}

// draw the floor in display()
void drawfloor(vec4 at, vec4 up)
{
	color4 material_ambient(0.2, 0.2, 0.2, 1.0); // the ambient color for the ground
	color4 material_diffuse(0, 1, 0.0, 1.0); // give the ground a green diffuse color
	color4 material_specular(0.0, 0.0, 0.0, 1.0); // the specular color for the ground
	float shininess = 0;
	// distant light source product 
	color4 ambient_product = distant_light_ambient * material_ambient;
	color4 diffuse_product = distant_light_diffuse * material_diffuse;
	color4 specular_product = distant_light_specular * material_specular;

	// optional light source product
	color4 ambient_product1 = light_optional_ambient * material_ambient;
	color4 diffuse_product1 = light_optional_diffuse * material_diffuse;
	color4 specular_product1 = light_optional_specular * material_specular;



	mat4 mv = LookAt(eye, at, up);

	if (lightingFlag == 1) // if there's shading
	{
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniform1f(glGetUniformLocation(program, "cflag"), 1); // set to shading
		glUniform1f(glGetUniformLocation(program, "&sphere_texture_mappingFlag"), 0); 
		SetUp_Lighting(mv, ambient_product, diffuse_product, specular_product, distant_light_direction, shininess);
		SetUp_Lighting_optional(mv, ambient_product1, diffuse_product1, specular_product1, shininess);
		mat3 normal_matrix = NormalMatrix(mv, 0);
		glUniformMatrix3fv(glGetUniformLocation(program, "Normal_Matrix"), 1, GL_TRUE, normal_matrix);// set Normal_Matrix to the shader
		glUniform1i(glGetUniformLocation(program, "f_shader_flag"), floor_textureFlag); // draw the floor
		// Set the value of the fragment shader texture sampler variable
	//   ("texture_2D") to the appropriate texture unit. In this case,
	//   0, for GL_TEXTURE0 which was previously set in init() by calling
	//   glActiveTexture( GL_TEXTURE0 ).
		glUniform1i(glGetUniformLocation(program, "ground_texture_2D"), 0);

	}
	else // if there's no shading
	{
		glUniformMatrix4fv(ModelView, 1, GL_TRUE, mv); // GL_TRUE: matrix is row-major
		glUniform1f(glGetUniformLocation(program, "cflag"), 0); // set to ambient
		mat4 mv;
		mv = LookAt(eye, at, up) *  Scale(1, 1, 1);
		color4 flatcolor = color4(0, 1, 0, 1);  // green
		glUniform4fv(glGetUniformLocation(program, "vColor"), 1, flatcolor); // pass light color to vshader
	}
	
	if (floorFlag == 1) // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	else              // Filled floor
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	drawObj(floor_buffer, floor_NumVertices);  // draw the floor
	glUniform1f(glGetUniformLocation(program, "cflag"), 1); // set to shading
	glUniform1i(glGetUniformLocation(program, "f_shader_flag"), -1); 
	glUniform1f(glGetUniformLocation(program, "&sphere_texture_mappingFlag"), 1);
}

// set up the global distant light source by sending the values into the shader
void SetUp_Lighting(mat4 mv, color4 ambient_product, color4 diffuse_product, color4 specular_product, vec4 lightdirection, float shininess)
{
	
	glUniform4fv(glGetUniformLocation(program, "GlobalAmbient"), 1, global_light_ambient); // pass global ambient color to vshader
	glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);

	glUniform4fv(glGetUniformLocation(program, "AmbientProduct"), 1, ambient_product); // pass ambient product to vshader

	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct"), 1, diffuse_product); // pass diffuse product to vshader
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct"), 1, specular_product); // pass specular product to vshader

	vec4 light_direction_eyeFrame = -lightdirection; // set the light direction in the eye frame
	glUniform4fv(glGetUniformLocation(program, "LightDirection"), 1, light_direction_eyeFrame); // pass light position to vshader
	glUniform1f(glGetUniformLocation(program, "Shininess"), shininess);
}

// set up the optional light source by sending the values into the shader
void SetUp_Lighting_optional(mat4 mv, color4 ambient_product, color4 diffuse_product, color4 specular_product,float shininess)
{
	if (lightFlag == 1) // spot light
	{
		glUniform1f(glGetUniformLocation(program, "LightType"), 1);
	}
	else // point source
	{
		glUniform1f(glGetUniformLocation(program, "LightType"), 0);
	}
	glUniform4fv(glGetUniformLocation(program, "AmbientProduct1"), 1, ambient_product); // pass optional ambient product to vshader
	glUniform4fv(glGetUniformLocation(program, "DiffuseProduct1"), 1, diffuse_product); // pass optional diffuse product to vshader
	glUniform4fv(glGetUniformLocation(program, "SpecularProduct1"), 1, specular_product); // pass optional specular product to vshader

	// The Light Position in Eye Frame
	vec4 light_position_eyeFrame = mv * light_optional_position;
	vec4 light_direction_eyeFrame = mv * light_optional_direction;
	glUniform4fv(glGetUniformLocation(program, "LightPosition"), 1, light_position_eyeFrame); // pass light position to vshader
	glUniform4fv(glGetUniformLocation(program, "LightDirection1"), 1, light_direction_eyeFrame); // pass light position to vshader
	glUniform1f(glGetUniformLocation(program, "ConstAtt"), const_att);
	glUniform1f(glGetUniformLocation(program, "LinearAtt"), linear_att);
	glUniform1f(glGetUniformLocation(program, "QuadAtt"), quad_att);
	glUniform1f(glGetUniformLocation(program, "exp"), exp1);
	glUniform1f(glGetUniformLocation(program, "cutoffangle"), cutoffangle*0.0174533);
}

// set up textures of the floor and the sphere
void image_set_up(void)
{
	int i, j, c;

	/* --- Generate checkerboard image to the image array ---*/
	for (i = 0; i < ImageHeight; i++)
		for (j = 0; j < ImageWidth; j++)
		{
			c = (((i & 0x8) == 0) ^ ((j & 0x8) == 0));

			if (c == 1) /* white */
			{
				c = 255;
				Image[i][j][0] = (GLubyte)c;
				Image[i][j][1] = (GLubyte)c;
				Image[i][j][2] = (GLubyte)c;
			}
			else  /* green */
			{
				Image[i][j][0] = (GLubyte)0;
				Image[i][j][1] = (GLubyte)150;
				Image[i][j][2] = (GLubyte)0;
			}

			Image[i][j][3] = (GLubyte)255;
		}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

	/*--- Generate 1D stripe image to array stripeImage[] ---*/
	for (j = 0; j < stripeImageWidth; j++) {
		/* When j <= 4, the color is (255, 0, 0),   i.e., red stripe/line.
		   When j > 4,  the color is (255, 255, 0), i.e., yellow remaining texture
		 */
		stripeImage[4 * j] = (GLubyte)255;
		stripeImage[4 * j + 1] = (GLubyte)((j > 4) ? 255 : 0);
		stripeImage[4 * j + 2] = (GLubyte)0;
		stripeImage[4 * j + 3] = (GLubyte)255;
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	/*----------- End 1D stripe image ----------------*/

	/*--- texture mapping set-up is to be done in
		  init() (set up texture objects),
		  display() (activate the texture object to be used, etc.)
		  and in shaders.
	 ---*/

}

// set up confetti vertex arrays
void confetti_setup()
{
	int i;
	for (i = 0; i < 300; i++)
	{
		float c_x, c_y, c_z;
		c_x = (rand() % 256) / 256.0;
		c_y = (rand() % 256) / 256.0;
		c_z = (rand() % 256) / 256.0;
		confetti_colors[i] = color4(c_x, c_y, c_z, 1); // set up random color for each confetti
		
		confetti_points[i] = point4(0, 0.1, 0, 1); // set up confetti starting position in world frame

		float v_x,v_y,v_z;
		v_x = 2.0*((rand() % 256) / 256.0 - 0.5);
		v_y = 1.2*2.0*((rand() % 256) / 256.0);
		if (v_y > largest_confetti_y)
		{
			largest_confetti_y = v_y; // get the last confetti to fall down the floor
		}
		v_z = 2.0*((rand() % 256) / 256.0 - 0.5);
		confetti_velocity[i] = vec3(v_x, v_y, v_z); // set up confetti velocity(flying direction) in eye frame
	}
}

// function for drawing the confetti in the display function
void draw_confetti()
{
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	float t = (float)glutGet(GLUT_ELAPSED_TIME) - animation_time;
	glUniform1f(glGetUniformLocation(confettiprogram, "animation_time"), t); // set animation time for the confetti animation
   
	//--- Activate the vertex buffer object to be drawn ---//
	glBindBuffer(GL_ARRAY_BUFFER, confetti_buffer);

	/*----- Set up vertex attribute arrays for each vertex attribute, using different glsl files for the confetti -----*/
	GLuint vPosition = glGetAttribLocation(confettiprogram, "vPosition"); // position of each vertex(confetti)
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	GLuint velocity = glGetAttribLocation(confettiprogram, "velocity"); // velocity of each vertex(confetti)
	glEnableVertexAttribArray(velocity);
	glVertexAttribPointer(velocity, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(confetti_points)));

	GLuint vColor = glGetAttribLocation(confettiprogram, "vColor"); // color of each vertex(confetti)
	glEnableVertexAttribArray(vColor);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(confetti_points) + sizeof(confetti_velocity)));

	glDrawArrays(GL_POINTS, 0, confetti_Num); // draw all the confettis

	/*--- Disable each vertex attribute array being enabled ---*/
	glDisableVertexAttribArray(vPosition);
	glDisableVertexAttribArray(vColor);
	glDisableVertexAttribArray(velocity);
}





