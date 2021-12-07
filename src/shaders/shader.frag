#version 450

// ---------------------------------------------------------------------
//  Fragment Shader
//
//  Input: vertex attributes (position, color, normal, texture coord.)
//  Output: final color for each fragment to the framebuffer
// ---------------------------------------------------------------------

// input/output variables
layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

// main() will be called for EVERY FRAGMENT, just like vertex shaders for vertices.
void main()
{
    outColor = vec4(fragColor, 1.0);
}
