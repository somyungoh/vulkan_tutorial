#version 450

// ---------------------------------------------------------------------
//  Vertex Shader
//
//  Input: vertex attributes (position, color, normal, texture coord.)
//  Output: final position in clip coordinates
//      * clip-coordinates, 4D vector will later be turned into a
//        Normalized Device Coordinates (NDC) which simply is a
//        homogeneous coordinates obtained by dividing in 4th element.
// ---------------------------------------------------------------------

// input/output variables
layout(location = 0) out vec3 fragColor;

// In the future, vertex data will be passed to the shader through
// the vertex buffer, but for a simple triangle we will hard-code the data.
vec2 pos[3] = vec2[]
(                       // Normalized Device Coordiante (NDC):
    vec2(0.0, -0.5),    // [-1,-1]-------------[1,-1]
    vec2(0.5, 0.5),     //    |                  |
    vec2(-0.5, 0.5)     //    |                  |
);                      // [-1, 1]-------------[1, 1]

vec3 color[3] = vec3[]
(
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

// main will be called for EVERY VERTEX, and
// gl_VertexIndex is a built-in variable that points to the current vertex
void main()
{
    // The last component is 1, so that it can be directly used as NDC
    gl_Position = vec4(pos[gl_VertexIndex], 0.0, 1.0);
    fragColor = color[gl_VertexIndex];
}
