uniform sampler2D Texture;

varying vec4 C;

void main()
{
    vec4 v = vec4( gl_Vertex );

    vec4 color = texture2D( Texture, ( gl_TextureMatrix[0] * gl_MultiTexCoord0 ).st );

    // vertex z value determines height of waterfall surface
    v.z = color.r;

    // v.z = ( color.r + color.g + color.b );

    C = color;
    gl_Position = gl_ModelViewProjectionMatrix * v;
}

