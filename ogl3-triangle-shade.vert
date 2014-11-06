#version 150 // GLSL 150 = OpenGL 3.2

in vec3 in_Position; // vertex position (object coordinates)
in vec3 in_Normal;   // normal vector   (object coordinates)

out vec4 out_VertexPos; // vertex position (camera coordinates)
out vec4 out_Normal;    // normal vector   (camera coordinates)

uniform mat4 ModelView;
uniform mat4 Projection;
uniform mat3 NormalMat;

uniform int red;

void main() 
{
	// Transform the normal by the NormalMat and send it to the
	// fragment program.
	vec3 transformedNormal = normalize(NormalMat * in_Normal);
	out_Normal = vec4(transformedNormal.xyz, 0);

	// Transform the vertex position from object coordinates into
	// camera coordinates.
	out_VertexPos = ModelView * vec4(in_Position, 1);

	// Transform the vertex position from object coordinates into
	// Normalized Device Coordinates (NDC).
	gl_Position = Projection * ModelView * vec4(in_Position.xyz, 1);
}
