// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors

in  vec4 vPosition; // vertex position in world frame
in  vec3 vNormal; // the normal vector of a vertex in world frame
in  vec2 floorTextureCoord;// the texture coordinates for the floor object

out vec4 color; // color for the pixel to be passed into fragment shader
out vec4 dist; // distance of pixel to center of projection to be passed into fragment shader
out vec2 texCoord; // texture coordinate for a checkerboard mapped to the floor
out float sphereTextureS; // s attribute for an 1D texture mapped to the sphere
out float sphereTextureT; // t attribute for an 1D texture mapped to the sphere
out vec2 latticeCoord; // vector 2 coordinate for lattice effects to be passed into fragment shader

// lighting attributes for a globallight source
uniform vec4 GlobalAmbient; // the global ambient
// the ambient product, diffuse product and specular product of the global lightsource
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct; 
uniform vec4 LightDirection;   // the direction of the global light source. Must be in Eye Frame
uniform float Shininess; // shininess of the sphere 

uniform vec4 vColor; // if we only use ambient light to render an object, meaning no shading
uniform mat4 ModelView; // model view matrix to transform vertex from world frame to eye frame
uniform mat4 Projection; // projection matrix to transform vertex from world frame to eye frame
uniform mat3 Normal_Matrix; // normal matrix to transform the normal of the vertex from world frame to the correct value in eye frame
uniform float ConstAtt;  // Constant Attenuation for any lightsource
uniform float LinearAtt; // Linear Attenuation for any lightsource
uniform float QuadAtt;   // Quadratic Attenuation for any lightsource

// lighting attributes for an optional light source
uniform float LightType; // light type 0: point source, 1: spot light
uniform vec4 AmbientProduct1, DiffuseProduct1, SpecularProduct1;
uniform vec4 LightDirection1; //the direction of the spotlight in eye frame
uniform vec4 LightPosition; //the position of the spotlight or point source light in eye Frame
uniform float exp; // exponential attribute for spotlight
uniform float cutoffangle; // cutoff angle for spotlight

// flags for drawing different things
uniform  float cflag; // flag for color ,0: ambient color product, 1: shading
uniform int sphere_textureFlag; // flag for contour lines 0: vertical 1: slanted and object space 2: eyespace 3: vertical and object space
uniform int latticeType; // flag for drawing different lattice effects

void main()
{
    // applying the distant light source(global light source)
    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz; // get vertex's position in eye frame
    vec3 L = normalize(LightDirection.xyz); //get normalized direction vector of global light source, this is L in the equation
    vec3 E = normalize( -pos ); // get normalized direction vector of user position, this is V in the equation
    vec3 H = normalize( L + E ); // get normalized direction vector of L+V, this is H in the equation

    // Transform vertex normal into eye coordinates, we need to do this 
	// because doing rotations will screw up the normal(L+H) in eye view when it is multiplied with the modelview matrix

	vec3 N = normalize(Normal_Matrix * vNormal);

    // YJC Note: N must use the one pointing *toward* the viewer
    //     ==> If (N dot E) < 0 then N must be changed to -N
   if ( dot(N, E) < 0 ) N = -N;

 // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct; // global ambient product, this is ka*La*global_attenuation(=1)

    float d = max( dot(L, N), 0.0 ); // global diffuse product, this is kd*Ld*max{(l*n),0}
    vec4  diffuse = d * DiffuseProduct;

    float s = pow( max(dot(N, H), 0.0), Shininess ); // global specular product, this is [if l*n>=0]*ks*ls*[max{(n*h),0}]^shininess, n*h replaces r*v
    vec4  specular = s * SpecularProduct;
    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 


	// applying the optional light source

	// Transform vertex  position into eye coordinates
    vec3 L1 = normalize(LightPosition.xyz-pos);
    vec3 H1 = normalize( L1 + E );

    // Transform vertex normal into eye coordinates
    vec3 N1 = normalize(Normal_Matrix * vNormal);

    if ( dot(N1, E) < 0 ) N1 = -N1;

    // Compute terms in the illumination equation
    vec4 ambient1 = AmbientProduct1;

    float d1 = max( dot(L1, N1), 0.0 );
    vec4  diffuse1 = d1 * DiffuseProduct1;

    float s1 = pow( max(dot(N1, H1), 0.0), Shininess );
    vec4  specular1 = s1 * SpecularProduct1;
    if( dot(L1, N1) < 0.0 ) {
	specular1 = vec4(0.0, 0.0, 0.0, 1.0);
    } 

    float attenuation1 = 1; // global distant light source attenuation
	float attenuation2  = 1; // optional light source attenuation

	vec3 D1 = (LightPosition.xyz - pos);
	float dp =D1.x*D1.x+D1.y*D1.y+D1.z*D1.z;
	dp = pow(dp,0.5);

	
	if(LightType == 0) // point source
	{
	    attenuation2 = (1/(ConstAtt+LinearAtt*dp+QuadAtt*dp*dp));
	}
	else // if spotlight
	{
	vec3 f = normalize(LightDirection1.xyz);
	   float p = dot(f,-L1);
	    if( p < cos(cutoffangle))
	    { 
		attenuation2 = 0;
		}
		else
	    { 
		attenuation2 = ( 1/(ConstAtt+LinearAtt*dp+QuadAtt*dp*dp)) * pow(p,exp);
		}
	}

    gl_Position = Projection * ModelView * vPosition; // transform vertex position from world frame to eye frame

	if( cflag == 1) // if we choose shading
	{

	/*--- attenuation below must be computed properly using the overall formula ---*/
    color = ambient * GlobalAmbient +  attenuation1* (ambient + diffuse + specular) + attenuation2*(ambient1 + diffuse1 + specular1);

	}
	else // if no shading
	{
	color = vColor; // use the ambient color on everything
	}

	// get the distance from vertex to center of projection
	dist = vec4(pos,1);
	texCoord = floorTextureCoord; // send texture mapping coordinates to the fragment shader for the floor

	if(sphere_textureFlag ==0) // vertical object space
	{
	 sphereTextureS = 2.5*vPosition.x; // send 1D texture mapping coordinates to the fragment shader for the sphere
	}

	else if (sphere_textureFlag == 1) // slanted object space
	{
	 sphereTextureS = 1.5*(vPosition.x+vPosition.y+vPosition.z); // send 1D texture mapping coordinates to the fragment shader for the sphere
	}

	else if (sphere_textureFlag == 2) // vertical eye space
	{
	 sphereTextureS = 2.5 * pos.x; // send 1D texture mapping coordinates to the fragment shader for the sphere
	}

	else if (sphere_textureFlag == 3) // slanted eye space
	{
	 sphereTextureS = 1.5*(pos.x+pos.y+pos.z); // send 1D texture mapping coordinates to the fragment shader for the sphere
	}

	else if (sphere_textureFlag == 4) // vertical object space
	{
	     // send 2D texture mapping coordinates to the fragment shader for the sphere
		 sphereTextureS = 0.75*(vPosition.x + 1);  
		 sphereTextureT = 0.75*(vPosition.y + 1);  
	}

	else if(sphere_textureFlag ==5) // slanted object space
	{
         // send 2D texture mapping coordinates to the fragment shader for the sphere
		 sphereTextureS = 0.45*(vPosition.x + vPosition.y + vPosition.z);
		 sphereTextureT = 0.45*(vPosition.x - vPosition.y + vPosition.z);
	}

	else if(sphere_textureFlag == 6) // vertical eye space
	{
	     // send 2D texture mapping coordinates to the fragment shader for the sphere
		 sphereTextureS =0.75*(pos.x + 1);
		 sphereTextureT = 0.75*(pos.y + 1);
	}

	else if(sphere_textureFlag ==7) // slanted eye space
	{
	     // send 2D texture mapping coordinates to the fragment shader for the sphere
		 sphereTextureS = 0.45*(pos.x + pos.y + pos.z);
		 sphereTextureT = 0.45*(pos.x - pos.y + pos.z);
	}

	if(latticeType == 0) // upright
	{
	   // send 2D lattice effect texture mapping coordinates to the fragment shader for the sphere
	   latticeCoord = vec2(0.5*(vPosition.x+1),0.5*(vPosition.y+1));
	}
	else if (latticeType ==1) // slanted
	{
	   // send 2D lattice effect texture mapping coordinates to the fragment shader for the sphere
	   latticeCoord = vec2(0.3*(vPosition.x+vPosition.y+vPosition.z),0.3*(vPosition.x-vPosition.y+vPosition.z));
	}

}