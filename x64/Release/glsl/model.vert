#version 330
//author https://github.com/autergame

layout (location = 0) in vec3 Positions;
layout (location = 1) in vec2 UVs;
layout (location = 2) in vec4 BoneIndices;
layout (location = 3) in vec4 BoneWeights;

out vec2 UV;

uniform mat4 MVP;
uniform mat4 Bones[255];

void main()
{
    mat4 BoneTransform = Bones[int(BoneIndices[0])] * BoneWeights[0];
    BoneTransform     += Bones[int(BoneIndices[1])] * BoneWeights[1];
    BoneTransform     += Bones[int(BoneIndices[2])] * BoneWeights[2];
    BoneTransform     += Bones[int(BoneIndices[3])] * BoneWeights[3];

    UV = UVs;
    gl_Position = MVP * BoneTransform * vec4(Positions, 1.0);
}