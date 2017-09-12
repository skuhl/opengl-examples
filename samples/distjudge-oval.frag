#version 150 // GLSL 150 = OpenGL 3.2
 
out vec4 fragColor;
in vec2 out_TexCoord;


uniform sampler2D tex;

void main() 
{
		float k1 = .7;
		float texScale = 1;

		float circleSizeA = 260;
		float circleSizeB = (circleSizeA * 4) / 5;
		float xCoord = gl_FragCoord.x - 1182/2;
		float yCoord = gl_FragCoord.y - 1464/2;
		if( (xCoord*xCoord/(circleSizeA*circleSizeA) + yCoord*yCoord/(circleSizeB*circleSizeB)) > 1.0 ){
			fragColor = vec4(0.0,0.0,0.0,1.0); // Black
		}
		else{
			fragColor = texture(tex, out_TexCoord);
		}


}
