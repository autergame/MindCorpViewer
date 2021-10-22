//author https://github.com/autergame
#ifndef _SKL_H_
#define _SKL_H_

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <string>
#include <vector>

#include "MemReader.h"
#include "skn.h"

typedef struct Bone
{
	std::string Name;
	uint32_t Hash = 0;

	int16_t ID = 0;
	int16_t ParentID = 0;

	glm::mat4 LocalMatrix = glm::mat4(1.f);
	glm::mat4 InverseGlobalMatrix = glm::mat4(1.f);
	glm::mat4 GlobalMatrix = glm::mat4(1.f);

	Bone* Parent = nullptr;
	std::vector<Bone*> Children;
} Bone;

enum Type : uint32_t
{
	Classic = 0x746C6B73,
	Version2 = 0x22FD4FC3
};

typedef struct Skeleton
{
	Type Type;
	uint32_t Version = 0;
	std::vector<Bone> Bones;
	std::vector<uint32_t> BoneIndices;
} Skeleton;

void RecursiveInvertGlobalMatrices(glm::mat4 Parent, Bone* Bone)
{
	auto Global = Parent * Bone->LocalMatrix;
	Bone->GlobalMatrix = Global;
	Bone->InverseGlobalMatrix = glm::inverse(Global);

	for (uint32_t i = 0; i < Bone->Children.size(); i++)
		RecursiveInvertGlobalMatrices(Global, Bone->Children[i]);
}

uint32_t StringToHash(char* str, size_t strlen)
{
	uint32_t hash = 0, temp = 0;
	for (size_t i = 0; i < strlen; i++)
	{
		hash = (hash << 4) + tolower(str[i]);
		temp = hash & 0xf0000000;
		if (temp != 0)
		{
			hash = hash ^ (temp >> 24);
			hash = hash ^ temp;
		}
	}
	return hash;
}

int openskl(Skeleton *myskl, char* filename)
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
	
	input.m_Pointer += 4;

	input.MemRead(&myskl->Type, 4);
	input.MemRead(&myskl->Version, 4);

	if (myskl->Type == Classic)
	{
		input.m_Pointer += 4;

		uint32_t BoneCount = input.MemRead<uint32_t>();

		myskl->Bones.resize(BoneCount);
		for (uint32_t i = 0; i < BoneCount; i++)
		{			
			Bone& Bone = myskl->Bones[i];

			char Name[32] = { '\0' };
			input.MemRead(Name, 32);
			Bone.Hash = StringToHash(Name, strlen(Name));
			Bone.Name = Name;

			Bone.ID = i;
			input.MemRead(&Bone.ParentID, 4);
			if (Bone.ParentID >= 0)
			{
				Bone.Parent = &myskl->Bones[Bone.ParentID];
				Bone.Parent->Children.emplace_back(&Bone);
			}
			else {
				Bone.Parent = nullptr;
			}

			float Scale = input.MemRead<float>();

			for (int y = 0; y < 3; y++)
				for (int x = 0; x < 4; x++)
					input.MemRead(&Bone.GlobalMatrix[x][y], 4);

			Bone.GlobalMatrix[3][3] = 1.0f;
			Bone.InverseGlobalMatrix = glm::inverse(Bone.GlobalMatrix);
		}

		if (myskl->Version < 2)
		{			
			myskl->BoneIndices.resize(BoneCount);
			for (uint32_t i = 0; i < BoneCount; i++)
				myskl->BoneIndices[i] = i;
		}
		else if (myskl->Version == 2)
		{
			uint32_t BoneIndexCount = input.MemRead<uint32_t>();

			myskl->BoneIndices.resize(BoneIndexCount);
			input.MemRead(myskl->BoneIndices.data(), BoneIndexCount * 4);
		}
	}
	else if (myskl->Type == Version2)
	{
		input.m_Pointer += 2;

		uint16_t BoneCount = input.MemRead<uint16_t>();
		uint32_t BoneIndexCount = input.MemRead<uint32_t>();
		uint16_t DataOffset = input.MemRead<uint16_t>();

		input.m_Pointer += 2;

		uint32_t BoneIndexMapOffset = input.MemRead<uint32_t>();
		uint32_t BoneIndicesOffset = input.MemRead<uint32_t>();

		input.m_Pointer += 8;

		uint32_t BoneNamesOffset = input.MemRead<uint32_t>();

		input.m_Pointer = DataOffset;

		myskl->Bones.resize(BoneCount);
		for (int i = 0; i < BoneCount; ++i)
		{
			Bone& Bone = myskl->Bones[i];

			input.m_Pointer += 2;

			input.MemRead(&Bone.ID, 2);
			if (Bone.ID != i)
			{
				printf("skl noticed an unexpected ID value for bone %d: %d\n", i, Bone.ID);
				fclose(file);
				return 1;
			}

			input.MemRead(&Bone.ParentID, 2);
			Bone.Parent = Bone.ParentID >= 0 ? &myskl->Bones[Bone.ParentID] : nullptr;

			input.m_Pointer += 2;

			input.MemRead(&Bone.Hash, 4);

			input.m_Pointer += 4;

			glm::vec3 Position;
			input.MemRead(&Position, 12);

			glm::vec3 Scale;
			input.MemRead(&Scale, 12);

			glm::quat Rotation;
			input.MemRead(&Rotation, 16);

			input.m_Pointer += 44;

			Bone.LocalMatrix = glm::translate(Position) * glm::mat4_cast(Rotation) * glm::scale(Scale);

			if (Bone.ParentID != -1)
			{
				myskl->Bones[Bone.ParentID].Children.emplace_back(&Bone);
			}
		}

		input.m_Pointer = BoneIndicesOffset;

		myskl->BoneIndices.resize(BoneIndexCount);
		for (uint32_t i = 0; i < BoneIndexCount; i++)
		{
			uint16_t BoneIndex = input.MemRead<uint16_t>();
			myskl->BoneIndices[i] = BoneIndex;
		}

		input.m_Pointer = BoneNamesOffset;

		char* Pointer = (char*)(input.m_Array.data() + input.m_Pointer);
		for (int i = 0; i < BoneCount; ++i)
		{
			myskl->Bones[i].Name = Pointer;
			Pointer += strlen(Pointer);
			while (*Pointer == '\0')
				Pointer++;
		}

		for (size_t i = 0; i < myskl->Bones.size(); i++)
		{
			if (myskl->Bones[i].ParentID != -1)
				continue;
			RecursiveInvertGlobalMatrices(glm::identity<glm::mat4>(), &myskl->Bones[i]);
		}
	}
	else
	{
		printf("skl has an unsupported version: %d\n", myskl->Type);
		fclose(file);
		return 1;
	}

	printf("skl version %d was succesfully loaded: Type: %d Bones: %zu BoneIndices: %zu\n",
		myskl->Version, myskl->Type, myskl->Bones.size(), myskl->BoneIndices.size());
	fclose(file);
	return 0;
}

void fixbone(Skin* skn, Skeleton* skl)
{
	for (uint32_t i = 0; i < skn->BoneIndices.size(); i++)
	{
		skn->BoneIndices[i][0] = (float)skl->BoneIndices[(uint32_t)skn->BoneIndices[i][0]];
		skn->BoneIndices[i][1] = (float)skl->BoneIndices[(uint32_t)skn->BoneIndices[i][1]];
		skn->BoneIndices[i][2] = (float)skl->BoneIndices[(uint32_t)skn->BoneIndices[i][2]];
		skn->BoneIndices[i][3] = (float)skl->BoneIndices[(uint32_t)skn->BoneIndices[i][3]];
	}
}

#endif //_SKL_H_