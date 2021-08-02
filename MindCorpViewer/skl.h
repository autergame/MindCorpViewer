//author https://github.com/autergame
#pragma once

typedef struct Bone
{
	std::string Name;
	uint32_t Hash = 0;

	int16_t ID = 0;
	int16_t ParentID = 0;

	glm::mat4 LocalMatrix = glm::mat4(1.f);
	glm::mat4 InverseGlobalMatrix = glm::mat4(1.f);
	glm::mat4 GlobalMatrix = glm::mat4(1.f);

	Bone* Parent;
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

bool removechar(char pchar)
{
	if ((pchar >= '0' && pchar <= '9') || 
		(pchar >= 'A' && pchar <= 'Z') || 
		(pchar >= 'a' && pchar <= 'z') || 
		pchar == '_')
		return false;
	else
		return true;
}

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
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		printf("Error opening file: %s %d (%s)\n", filename, errno, strerror(errno));
	
	fseek(fp, 4, 0);
	fread(&myskl->Type, sizeof(uint32_t), 1, fp);
	fread(&myskl->Version, sizeof(uint32_t), 1, fp);

	if (myskl->Type == Classic)
	{
		fseek(fp, 16, 0);
		uint32_t BoneCount;
		fread(&BoneCount, sizeof(uint32_t), 1, fp);

		char Name[32];
		myskl->Bones.resize(BoneCount);
		for (uint32_t i = 0; i < BoneCount; i++)
		{			
			Bone& Bone = myskl->Bones[i];

			fread(Name, sizeof(char), 32, fp);
			Bone.Hash = StringToHash(Name, strlen(Name));
			Bone.Name = Name;

			Bone.ID = i;
			fread(&Bone.ParentID, sizeof(int32_t), 1, fp);
			if (Bone.ParentID >= 0)
			{
				Bone.Parent = &myskl->Bones[Bone.ParentID];
				Bone.Parent->Children.emplace_back(&Bone);
			}
			else Bone.Parent = nullptr;

			float Scale;
			fread(&Scale, sizeof(float), 1, fp);

			for (int y = 0; y < 3; y++)
				for (int x = 0; x < 4; x++)
					fread(&Bone.GlobalMatrix[x][y], sizeof(float), 1, fp);

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
			uint32_t BoneIndexCount;
			fread(&BoneIndexCount, sizeof(uint32_t), 1, fp);
			myskl->BoneIndices.resize(BoneIndexCount);
			fread(myskl->BoneIndices.data(), sizeof(uint32_t), BoneIndexCount, fp);
		}
	}
	else if (myskl->Type == Version2)
	{
		uint32_t Offset = 14;
		fseek(fp, Offset, 0);

		uint16_t BoneCount;
		fread(&BoneCount, sizeof(uint16_t), 1, fp); Offset += 2;

		uint32_t BoneIndexCount;
		fread(&BoneIndexCount, sizeof(uint32_t), 1, fp); Offset += 4;

		uint16_t DataOffset;
		fread(&DataOffset, sizeof(uint16_t), 1, fp); Offset += 2;

		Offset += sizeof(uint16_t);
		fseek(fp, Offset, 0);

		uint32_t BoneIndexMapOffset;
		fread(&BoneIndexMapOffset, sizeof(uint32_t), 1, fp); Offset += 4;

		uint32_t BoneIndicesOffset;
		fread(&BoneIndicesOffset, sizeof(uint32_t), 1, fp); Offset += 4;

		Offset += sizeof(uint32_t);
		Offset += sizeof(uint32_t);
		fseek(fp, Offset, 0);

		uint32_t BoneNamesOffset;
		fread(&BoneNamesOffset, sizeof(uint32_t), 1, fp); Offset += 4;

		Offset = DataOffset;
		fseek(fp, Offset, 0);

		myskl->Bones.resize(BoneCount);
		for (int i = 0; i < BoneCount; ++i)
		{
			Bone& Bone = myskl->Bones[i];

			Offset += sizeof(uint16_t);
			fseek(fp, Offset, 0);

			fread(&Bone.ID, sizeof(int16_t), 1, fp); Offset += 2;
			if (Bone.ID != i)
			{
				printf("skl noticed an unexpected ID value for bone %d: %d\n", i, Bone.ID);
				scanf("press enter to exit.");
				fclose(fp);
				return 1;
			}

			fread(&Bone.ParentID, sizeof(int16_t), 1, fp); Offset += 2;
			Bone.Parent = Bone.ParentID >= 0 ? &myskl->Bones[Bone.ParentID] : nullptr;

			Offset += sizeof(uint16_t);
			fseek(fp, Offset, 0);

			fread(&Bone.Hash, sizeof(uint32_t), 1, fp); Offset += 4;

			Offset += sizeof(uint32_t);
			fseek(fp, Offset, 0);

			glm::vec3 Position;
			fread(&Position.x, sizeof(float), 1, fp); Offset += 4;
			fread(&Position.y, sizeof(float), 1, fp); Offset += 4;
			fread(&Position.z, sizeof(float), 1, fp); Offset += 4;

			glm::vec3 Scale;
			fread(&Scale.x, sizeof(float), 1, fp); Offset += 4;
			fread(&Scale.y, sizeof(float), 1, fp); Offset += 4;
			fread(&Scale.z, sizeof(float), 1, fp); Offset += 4;

			glm::quat Rotation;
			fread(&Rotation.x, sizeof(float), 1, fp); Offset += 4;
			fread(&Rotation.y, sizeof(float), 1, fp); Offset += 4;
			fread(&Rotation.z, sizeof(float), 1, fp); Offset += 4;
			fread(&Rotation.w, sizeof(float), 1, fp); Offset += 4;

			Offset += 44;
			fseek(fp, Offset, 0);

			Bone.LocalMatrix = glm::translate(Position) * glm::mat4_cast(Rotation) * glm::scale(Scale);

			if (Bone.ParentID != -1)
			{
				myskl->Bones[Bone.ParentID].Children.emplace_back(&Bone);
			}
		}

		Offset = BoneIndicesOffset;
		fseek(fp, Offset, 0);

		myskl->BoneIndices.resize(BoneIndexCount);
		for (uint32_t i = 0; i < BoneIndexCount; i++)
		{
			uint16_t BoneIndex;
			fread(&BoneIndex, sizeof(uint16_t), 1, fp);
			myskl->BoneIndices[i] = BoneIndex;
			Offset += 2;
		}

		Offset = BoneNamesOffset;
		fseek(fp, Offset, 0);

		size_t NameChunkSize = 32 * BoneCount;
		std::vector<uint8_t> Start(NameChunkSize);
		memset(Start.data(), 0, NameChunkSize);
		fread(Start.data(), sizeof(char), NameChunkSize, fp);
		char* Pointer = (char*)Start.data();
		for (int i = 0; i < BoneCount; ++i)
		{
			myskl->Bones[i].Name = Pointer;
			Pointer += strlen(Pointer);
			while (removechar(*Pointer)) 
				Pointer++;
		}

		for (size_t i = 0; i < myskl->Bones.size(); i++)
		{
			if (myskl->Bones[i].ParentID != -1) continue;
			RecursiveInvertGlobalMatrices(glm::identity<glm::mat4>(), &myskl->Bones[i]);
		}
	}
	else
	{
		printf("skl has an unsupported version: %d\n", myskl->Type);
		scanf("press enter to exit.");
		fclose(fp);
		return 1;
	}

	printf("skl version %d was succesfully loaded: Type: %d Bones: %u BoneIndices: %d\n",
		myskl->Version, myskl->Type, myskl->Bones.size(), myskl->BoneIndices.size());
	fclose(fp);
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