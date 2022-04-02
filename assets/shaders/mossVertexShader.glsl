
#version 330 core

// Render data
layout ( location = 0 ) in vec3 vertexPosition;
layout ( location = 1 ) in vec2 vertexCoord;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 modelMatrix;

// Outputs for the fragment shader
out vec3 fragmentPosition;
out vec2 fragmentCoord;

void main()
{
    vec4 pos = projectionMatrix * viewMatrix * modelMatrix * vec4( vertexPosition, 1.0 );
    //vec4 pos = projectionMatrix * modelMatrix * vec4( vertexPosition, 1.0 );
    //vec4 pos = projectionMatrix * viewMatrix * vec4( vertexPosition, 1.0 );
    //vec4 pos = modelMatrix * vec4( vertexPosition, 1.0 );
    //vec4 pos = vec4( vertexPosition, 1.0 );

	gl_Position = pos;
	fragmentPosition = vertexPosition;
	fragmentCoord = vertexCoord;
}
