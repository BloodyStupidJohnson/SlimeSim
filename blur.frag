uniform sampler2D texture;
uniform float blur_radius;
uniform float evaporate;
uniform float cutout;
void main()
{
    vec2 offx = vec2(blur_radius, 0.0);
    vec2 offy = vec2(0.0, blur_radius);
    vec4 pixel = (texture2D(texture, gl_TexCoord[0].xy)                +
                 texture2D(texture, gl_TexCoord[0].xy - offx)         +
                 texture2D(texture, gl_TexCoord[0].xy + offx)         +
                 texture2D(texture, gl_TexCoord[0].xy - offy)         +
                 texture2D(texture, gl_TexCoord[0].xy + offy)         +
                 texture2D(texture, gl_TexCoord[0].xy - offx - offy)  +
                 texture2D(texture, gl_TexCoord[0].xy - offx + offy)  +
                 texture2D(texture, gl_TexCoord[0].xy + offx - offy)  +
                 texture2D(texture, gl_TexCoord[0].xy + offx + offy))/ 9.0;
    
    //gl_FragColor =(pixel ) * evaporate);
    if (pixel.r < cutout)
    {
    	pixel.r = 0.0;
    }
    if (pixel.g < cutout)
    {
    	pixel.g = 0.0;
    }
    if (pixel.b < cutout)
    {
    	pixel.b = 0.0;
    }
    gl_FragColor = pixel * evaporate;
}
