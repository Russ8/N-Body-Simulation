#version 330 core
out vec4 FragColor;
in float mass_g;
void main()
{
	vec4 color;
	float mass = mass_g / 10.0f;
	if(mass >= 0.9f) {
		color = vec4(0.9f, 0.9f, 1.0f, 1.0f);
	}
	else if(mass < 0.8f) {
		color = vec4(1.0f, 0.5f, 0.5f, 1.0f);
	} else {
		color = vec4(1.0f, 1.0f, 0.4f, 1.0f);
	}


    FragColor = color;
} 