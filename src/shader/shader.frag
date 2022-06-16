#version 330 core
in vec2 TexCoords;
in vec4 color;

uniform sampler2D image;

uniform float screenPxRange;

float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

void main()
{
	vec3 msd = texture(image, TexCoords).rgb;
	float sd = median(msd.r, msd.g, msd.b);

    vec4 bgColor = vec4(.0, .0, .0, 1.0);

    float screenPxDistance = screenPxRange*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    gl_FragColor = mix(bgColor, color, opacity);

    if (opacity < 0.5f)
    {
       discard;
    }
}  
