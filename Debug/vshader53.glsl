/* 
File Name: "vshader53.glsl":
Vertex shader:
  - Per vertex shading for a single point light source;
    distance attenuation is Yet To Be Completed.
  - Entire shading computation is done in the Eye Frame.
*/

// #version 150  // YJC: Comment/un-comment this line to resolve compilation errors
                 //      due to different settings of the default GLSL version

in  vec4 vPosition;
in  vec3 vNormal;
out vec4 color;

uniform vec4 GlobalAmbient;
uniform vec4 AmbientProduct, DiffuseProduct, SpecularProduct;
uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 Normal_Matrix; //
uniform vec4 LightDirection;   // the direction of the distant light. Must be in Eye Frame
uniform float Shininess; // shininess of the object
uniform vec4 vColor; // if we only use ambient light to render an object

uniform float ConstAtt;  // Constant Attenuation
uniform float LinearAtt; // Linear Attenuation
uniform float QuadAtt;   // Quadratic Attenuation
uniform float LightType; // light type 0: point source, 1: spot light
uniform vec4 AmbientProduct1, DiffuseProduct1, SpecularProduct1;
uniform vec4 LightDirection1; //the direction of the spotlight in eye frame
uniform vec4 LightPosition; //the position of the spotlight or point source light in eye Frame
uniform float exp;
uniform float cutoffangle;

uniform  float cflag; // flag for color ,0: ambient color product, 1: shading

void main()
{
 // the distant light source
    // Transform vertex  position into eye coordinates
    vec3 pos = (ModelView * vPosition).xyz;
    vec3 L = normalize(LightDirection.xyz);
    vec3 E = normalize( -pos );
    vec3 H = normalize( L + E );

    // Transform vertex normal into eye coordinates
    vec3 N = normalize(Normal_Matrix * vNormal);

// YJC Note: N must use the one pointing *toward* the viewer
//     ==> If (N dot E) < 0 then N must be changed to -N
//
   if ( dot(N, E) < 0 ) N = -N;

 // Compute terms in the illumination equation
    vec4 ambient = AmbientProduct;

    float d = max( dot(L, N), 0.0 );
    vec4  diffuse = d * DiffuseProduct;

    float s = pow( max(dot(N, H), 0.0), Shininess );
    vec4  specular = s * SpecularProduct;
   

    if( dot(L, N) < 0.0 ) {
	specular = vec4(0.0, 0.0, 0.0, 1.0);
    } 


	// the optional light source

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

    float attenuation1 = 1; 
	float attenuation2  = 1;

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

    gl_Position = Projection * ModelView * vPosition;

	if( cflag == 1) 
	{

	/*--- attenuation below must be computed properly ---*/
    color = ambient * GlobalAmbient +  attenuation1* (ambient + diffuse + specular) + attenuation2*(ambient1 + diffuse1 + specular1);

	}
	else
	{
	color = vColor;
	}

}