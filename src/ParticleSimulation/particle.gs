#version 330 core
layout (points) in;
layout (triangle_strip, max_vertices = 6) out;

in float mass[];
out float mass_g;

void main() {    

	mass_g = mass[0];
	float size = mass_g;

    gl_Position = gl_in[0].gl_Position + vec4(-size, -size, 0.0, 0.0);
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4( -size, size, 0.0, 0.0);
    EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4( size, size, 0.0, 0.0);
    EmitVertex();
	EndPrimitive();

    gl_Position = gl_in[0].gl_Position + vec4(-size, -size, 0.0, 0.0); 
    EmitVertex();

    gl_Position = gl_in[0].gl_Position + vec4( size, -size, 0.0, 0.0);
    EmitVertex();

	gl_Position = gl_in[0].gl_Position + vec4( size, size, 0.0, 0.0);
    EmitVertex();
     
    EndPrimitive();
}  