#version 150 // GLSL 150 = OpenGL 3.2
 
out vec4 fragColor;
in vec2 out_TexCoord;


uniform sampler2D tex;

void main() 
{
		float k1 = .7;
		float texScale = 1;

		float cubeSizeA = 360;
		float cubeSizeB = (cubeSizeA * 4) / 5;
		float xCoord = gl_FragCoord.x - 1182/2;
		float yCoord = gl_FragCoord.y - 1464/2;
		if( sqrt(xCoord*xCoord) > cubeSizeA || sqrt(yCoord*yCoord) > cubeSizeB ){
			fragColor = vec4(0.0,0.0,0.0,1.0); // Black
		}
		else{
			fragColor = texture(tex, out_TexCoord);
		}


}
