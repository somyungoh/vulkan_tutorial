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

// gloabl
layout(set = 0, binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

// input/output variables
layout(location = 0) in vec2 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

// main will be called for EVERY VERTEX, and
// gl_VertexIndex is a built-in variable that points to the current vertex
void main()
{
    // The last component is 1, so that it can be directly used as NDC
    gl_Position = ubo.proj * ubo.view * ubo.model * vec4(inPosition, 0.0, 1.0);

    fragColor = inColor;
    fragTexCoord = inTexCoord;
}
