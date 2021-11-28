#!/bin/bash

echo "start shader compilation"

glslc ./shaders/shader.vert -o ./shaders/vert.spv
glslc ./shaders/shader.frag -o ./shaders/frag.spv

echo "shader compilation done."