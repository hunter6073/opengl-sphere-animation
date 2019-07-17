 #version 150 

in vec4 vPosition;  // position of the vertex in the world frame
in vec4 vColor;  // color of the vertex
in vec3 velocity; // velocity of a vertex

out vec4 color; // vector4 value color to pass on to the fragment shader
out float y; // current y value of the pixel in eye frame to pass on to the fragment shader

uniform mat4 ModelView; // model view matrix to transform the vertex into eye frame
uniform mat4 Projection; // projection matrix to transform the vertex into eye frame
uniform float animation_time; // elapsed animation time or the animation time counter

void main() 
{

    float t = animation_time; // get time from time counter

	// calculate vertex's current position in world frame using point's velocity and animation time
	float f_x = vPosition.x + 0.001 * velocity.x * t;
    float f_z = vPosition.z + 0.001 * velocity.z * t;
	float a = -0.00000049;
    float f_y = vPosition.y + 0.001 * velocity.y * t + 0.5 * a * t * t;
	vec4 final_position = vec4(f_x, f_y, f_z, 1); // set vertex's current position in world frame using calculated attributes

    gl_Position = Projection * ModelView * final_position; // transform vertex from world frame to eye frame
    color = vColor; // set color attribute to pass to fragment shader
	y = f_y; // set y-axis value attribute to pass to fragment shader 


} 
