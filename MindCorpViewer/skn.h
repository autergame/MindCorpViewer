//author https://github.com/autergame
#ifndef _SKN_H_
#define _SKN_H_

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "MemReader.h"

uint32_t FNV1Hash(char* str, size_t strlen)
{
	uint32_t Hash = 0x811c9dc5;
	for (size_t i = 0; i < strlen; i++)
		Hash = (Hash ^ tolower(str[i])) * 0x01000193;
	return Hash;
}

struct SubMeshHeader
{
	char Name[64] = { '\0' };
	uint32_t VertexOffset = 0;
	uint32_t VertexCount = 0;
	uint32_t IndexOffset = 0;
	uint32_t IndexCount = 0;
};

struct Mesh
{
	char Name[64] = { '\0' };
	GLuint texid = 0;
	uint32_t Hash = 0;
	uint16_t* Indices = nullptr;
	uint32_t IndexCount = 0;
};

struct Skin
{
	uint16_t Major = 0;
	uint16_t Minor = 0;
	glm::vec3 center;
	std::vector<glm::vec3> Positions;
	std::vector<glm::vec4> BoneIndices;
	std::vector<glm::vec4> Weights;
	std::vector<glm::vec3> Normals;
	std::vector<glm::vec2> UVs;
	std::vector<uint16_t> Indices;
	std::vector<Mesh> Meshes;
};

int openskn(Skin *myskn, char* filename)
{
	FILE* file;
	errno_t err = fopen_s(&file, filename, "rb");
	if (err)
	{
		char errMsg[255] = { "\0" };
		strerror_s(errMsg, 255, errno);
		printf("ERROR: Cannot read file %s %s\n", filename, errMsg);
		return 0;
	}

	printf("Reading file: %s\n", filename);
	fseek(file, 0, SEEK_END);
	size_t fsize = (size_t)ftell(file);
	fseek(file, 0, SEEK_SET);
	std::string fp(fsize + 1, '\0');
	myassert(fread(fp.data(), 1, fsize, file) != fsize)
	fclose(file);
	printf("Finised reading file\n");

	CharMemVector input;
	input.MemWrite(fp.data(), fsize);

	uint32_t Signature = input.MemRead<uint32_t>();
	if (Signature != 0x00112233)
	{
		printf("skn has no valid signature\n");
		fclose(file);
		return 0;
	}

	input.MemRead(&myskn->Major, 2);
	input.MemRead(&myskn->Minor, 2);

	uint32_t SubMeshHeaderCount = 0;
	std::vector<SubMeshHeader> SubMeshHeaders;
	if (myskn->Major > 0)
	{
		input.MemRead(&SubMeshHeaderCount, 4);

		SubMeshHeaders.resize(SubMeshHeaderCount);
		input.MemRead(SubMeshHeaders.data(), SubMeshHeaderCount * sizeof(SubMeshHeader));
	}

	if (myskn->Major == 4)
	{
		input.m_Pointer += 4;
	}

	uint32_t IndexCount = input.MemRead<uint32_t>();
	uint32_t VertexCount = input.MemRead<uint32_t>();

	glm::vec3 BBMin = glm::vec3(6e6);
	glm::vec3 BBMax = glm::vec3(-6e6);

	uint32_t HasTangents = 0;
	if (myskn->Major == 4)
	{
		uint32_t VertexSize = input.MemRead<uint32_t>();
		input.MemRead(&HasTangents, 4);

		input.MemRead(&BBMin, 12);
		input.MemRead(&BBMax, 12);

		input.m_Pointer += 16;
	}

	myskn->center.x = (BBMax.x + BBMin.x) / 2;
	myskn->center.y = (BBMax.y + BBMin.y) / 2;
	myskn->center.z = (BBMax.z + BBMin.z) / 2;

	myskn->Indices.resize(IndexCount);
	input.MemRead(myskn->Indices.data(), IndexCount * 2);

	myskn->Positions.resize(VertexCount);
	myskn->BoneIndices.resize(VertexCount);
	myskn->Weights.resize(VertexCount);
	myskn->Normals.resize(VertexCount);
	myskn->UVs.resize(VertexCount);
	for (uint32_t i = 0; i < VertexCount; i++)
	{
		input.MemRead(&myskn->Positions[i], 12);

		for (int j = 0; j < 4; j++)
		{
			uint8_t bone = input.MemRead<uint8_t>();
			myskn->BoneIndices[i][j] = bone;
		}

		input.MemRead(&myskn->Weights[i], 16);
		input.MemRead(&myskn->Normals[i], 12);
		input.MemRead(&myskn->UVs[i], 8);

		if (HasTangents)
		{
			input.m_Pointer += 4;
		}

		float WeightError = fabsf(myskn->Weights[i].x + myskn->Weights[i].y + myskn->Weights[i].z + myskn->Weights[i].w - 1.0f);
		if (WeightError > 0.02f)
		{
			printf("WeightError\n");
			scanf_s("press enter to exit.");
			fclose(file);
			return 0;
		}
	}

	if (SubMeshHeaderCount > 0)
	{
		myskn->Meshes.resize(SubMeshHeaderCount);
		for (uint32_t i = 0; i < SubMeshHeaderCount; i++)
		{
			myassert(memcpy(myskn->Meshes[i].Name, SubMeshHeaders[i].Name, 64) != myskn->Meshes[i].Name);
			myskn->Meshes[i].Hash = FNV1Hash(SubMeshHeaders[i].Name, strlen(SubMeshHeaders[i].Name));

			myskn->Meshes[i].IndexCount = SubMeshHeaders[i].IndexCount;
			myskn->Meshes[i].Indices = &myskn->Indices[SubMeshHeaders[i].IndexOffset];
		}
	}
	else
	{
		myskn->Meshes.resize(1);
		myskn->Meshes[0].IndexCount = IndexCount;
		myskn->Meshes[0].Indices = &myskn->Indices[0];
	}

	printf("skn version %u %u was succesfully loaded: SubMeshHeaderCount: %d IndexCount: %d VertexCount: %d\n",
		myskn->Major, myskn->Minor, SubMeshHeaderCount, IndexCount, VertexCount);
	fclose(file);
	return 1;
}

#endif //_SKN_H_