#version 450

// ---------------------------------------------------------------------
//  Fragment Shader
//
//  Input: vertex attributes (position, color, normal, texture coord.)
//  Output: final color for each fragment to the framebuffer
// ---------------------------------------------------------------------

// input/output variables
layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

layout(binding = 1) uniform sampler2D texSampler;

// main() will be called for EVERY FRAGMENT, just like vertex shaders for vertices.
void main()
{
    // outColor = vec4(fragColor, 1.0);
    outColor = texture(texSampler, fragTexCoord);
}
