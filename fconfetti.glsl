 #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 color; // color attribute gotten from vertex shader
in float y; // y-axis value gotten from vertex shader
out vec4 fColor; // output color of this vertex(pixel)


void main() 
{ 
    fColor = color; // set color of this pixel
	if (y<0.1) // if this vertex is under the floor  
      discard;  // don't draw this vertex(pixel)
	  }