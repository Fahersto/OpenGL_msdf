#version 330 core
in vec2 TexCoords;

uniform sampler2D image;

uniform vec2 sizeInPixels;

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

//TODO promote this to uniform, so it doesnt hav eto be calculated for each pixel!
float screenPxRange(){
// ./msdfgen.exe msdf -font comici.ttf 65 -o msdf.png -size 32 32 -pxrange 4 -autoframe -testrender render.png 1024 1024
    int pixelRange = 2;
    vec2 distanceField = vec2(256,256);
    vec2 quadPixelSize = vec2(sizeInPixels);
    return (quadPixelSize.x/distanceField.x) * pixelRange;
}


void main()
{
    vec4 spriteColor = vec4(1, 1, 1, 1);
	vec3 msd = texture(image, TexCoords).rgb;
	float sd = median(msd.r, msd.g, msd.b);

    vec4 bgColor = vec4(0., 0., .0, 1.);
    vec4 fgColor = spriteColor;

    float screenPxDistance = screenPxRange()*(sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    gl_FragColor = mix(bgColor, fgColor, opacity);

    if (opacity < 0.5)
    {
        discard;
    }

}  
