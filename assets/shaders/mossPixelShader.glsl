
#version 330 core

in vec3 fragmentPosition;
in vec2 fragmentCoord;

uniform sampler2D diffuseMap;

out vec4 outColor;

float sampleAlpha( const vec2 coord, const float range, const int attempts )
{
    const float adjustedRange = range / float(attempts);
    float alpha = 0.0;

    const int halfAttempts = attempts / 2;
    for ( int x = -halfAttempts; x <= halfAttempts; x++ )
    {
        for ( int y = -halfAttempts; y <= halfAttempts; y++ )
        {
            vec2 tc = coord + vec2(x * adjustedRange, y * adjustedRange);
            alpha += texture( diffuseMap, tc ).a;
        }
    }

    return alpha / float(attempts * attempts);
}

void main()
{
    outColor = texture( diffuseMap, fragmentCoord );
    outColor.a = sampleAlpha( fragmentCoord, 1.0/256.0, 16 );
}
