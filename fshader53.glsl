/* 
File Name: "fshader53.glsl":
           Fragment Shader
*/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color; // color vector passed in by the vertex shader for color output
in  vec4 dist;  // distance from vertex to cop passed in by the vertex shader
in  vec2 texCoord; // texture coordinate for mapping the checkerboard to the floor
in float sphereTextureS; // texture coordinate for mapping texture to the sphere
in float sphereTextureT; // texture coordinate for mapping texture to the sphere
in vec2 latticeCoord; // texture coordinate for discarding pixels for the lattice effect

out vec4 fColor; // color of the pixel to be shown

uniform int fogFlag; // a flag for displaying fog effects
uniform int sphere_texture_mappingFlag; // if we should turn on texture for the sphere
uniform sampler2D ground_texture_2D; // 2D checkerboard texture for the ground texture
uniform sampler1D sphere_texture_1D; // 1D red texture for the sphere
uniform sampler2D sphere_texture_2D; // 2D checkboard texture for the sphere
uniform int f_shader_flag; // flag for drawing an object; 0: sphere 1: ground
uniform int latticeFlag; // flag for determining if we should display the lattice effect


void main() 
{ 
// if lattice effect is on and the conditions allow
 if(fract(4*latticeCoord.x)<0.35 && fract(4*latticeCoord.y)<0.35&& latticeFlag == 1)
	{
	      discard; // discard pixel and create the lattice effect
	}

vec4 main_color = color; // set the default color of the pixel, if we don't do anything, this will be the color to display for the pixel


 if (f_shader_flag == 1) // draw floor mapped with a 2d checkerboard texture
 {
    main_color = color * texture(ground_texture_2D,texCoord);
 }

  if(f_shader_flag == 0) // draw sphere mapped with a 1d red texture
 {
     main_color = color*texture(sphere_texture_1D,sphereTextureS);
 }

   if(f_shader_flag == 2) // draw sphere mapped with a 2d checkerboard texture
 {
     vec4 texColor = texture(ground_texture_2D,vec2(sphereTextureS,sphereTextureT));
     if(texColor.x < 0.5) // turn the green pixels to red
     {
	    texColor = vec4(0.9, 0.1, 0.1, 1.0); 	
	 }

     main_color = color*texColor; 
 }


	// generate fog effects
	vec4 fogcolor = vec4(0.7, 0.7, 0.7, 0.5); // set fog color
	float start = 0;  // start point
	float end = 18; // end point
	float expvalue = 0.09; // exponential value of the fog
	float f = 1;
	float dist1 =  dist.x*dist.x+dist.y*dist.y+dist.z*dist.z; 
	dist1 = pow(dist1,0.5);
	switch(fogFlag)
	{
		case 1:
		f = (end-dist1)/(end-start); // linear fog
		break;

		case 2:
		f = exp(-1*expvalue*dist1); // exponential fog
		break;

		case 3:
		f = exp(-1*pow(expvalue*dist1,2)); // exponential square fog
		break;
	}
	f = clamp(f,0,1); // clamp fog values to (0,1)
	vec4 afterfog = mix(fogcolor,main_color,f); // mix fog color with main color with a degree of f
    fColor = afterfog;


	










} 

