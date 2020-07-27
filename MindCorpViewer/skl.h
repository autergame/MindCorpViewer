//author https://github.com/autergame
#pragma once

struct Bone
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
};

enum Type : uint32_t
{
	Classic = 0x746C6B73,
	Version2 = 0x22FD4FC3
};

class Skeleton
{
public:
	Type Type;
	uint32_t Version = 0;
	std::vector<Bone> Bones;
	std::vector<uint32_t> BoneIndices;
};

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

uint32_t StringToHash(const std::string& s)
{
	uint32_t hash = 0;
	uint32_t temp = 0;

	for (auto& c : s)
	{
		hash = (hash << 4) + tolower(c);
		temp = hash & 0xf0000000;

		if (temp != 0)
		{
			hash = hash ^ (temp >> 24);
			hash = hash ^ temp;
		}
	}

	return hash;
}

int openskl(Skeleton *myskin, const char* filename)
{
	FILE *fp = fopen(filename, "rb");
	
	fseek(fp, 4, 0);
	fread(&myskin->Type, sizeof(uint32_t), 1, fp);
	fread(&myskin->Version, sizeof(uint32_t), 1, fp);

	if (myskin->Type == Classic)
	{
		fseek(fp, 16, 0);
		uint32_t BoneCount;
		fread(&BoneCount, sizeof(uint32_t), 1, fp);

		char Name[32];
		myskin->Bones.resize(BoneCount);
		for (uint32_t i = 0; i < BoneCount; i++)
		{			
			Bone& Bone = myskin->Bones[i];

			fread(Name, sizeof(char), 32, fp);
			Bone.Hash = StringToHash(Name);
			Bone.Name = Name;

			Bone.ID = i;
			fread(&Bone.ParentID, sizeof(int32_t), 1, fp);
			if (Bone.ParentID >= 0)
			{
				Bone.Parent = &myskin->Bones[Bone.ParentID];
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

		if (myskin->Version < 2)
		{			
			myskin->BoneIndices.resize(BoneCount);
			for (uint32_t i = 0; i < BoneCount; i++)
				myskin->BoneIndices[i] = i;
		}
		else if (myskin->Version == 2)
		{
			uint32_t BoneIndexCount;
			fread(&BoneIndexCount, sizeof(uint32_t), 1, fp);
			myskin->BoneIndices.resize(BoneIndexCount);
			fread(myskin->BoneIndices.data(), sizeof(uint32_t), BoneIndexCount, fp);
		}
	}
	else if (myskin->Type == Version2)
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

		myskin->Bones.resize(BoneCount);
		for (int i = 0; i < BoneCount; ++i)
		{
			Bone& Bone = myskin->Bones[i];

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
			Bone.Parent = Bone.ParentID >= 0 ? &myskin->Bones[Bone.ParentID] : nullptr;

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
				myskin->Bones[Bone.ParentID].Children.emplace_back(&Bone);
			}
		}

		Offset = BoneIndicesOffset;
		fseek(fp, Offset, 0);

		myskin->BoneIndices.reserve(BoneIndexCount);
		for (uint32_t i = 0; i < BoneIndexCount; i++)
		{
			uint16_t BoneIndex;
			fread(&BoneIndex, sizeof(uint16_t), 1, fp);
			myskin->BoneIndices.emplace_back(BoneIndex);
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
			myskin->Bones[i].Name = Pointer;
			Pointer += strlen(Pointer);
			while (removechar(*Pointer)) 
				Pointer++;
		}

		for (size_t i = 0; i < myskin->Bones.size(); i++)
		{
			if (myskin->Bones[i].ParentID != -1) continue;
			RecursiveInvertGlobalMatrices(glm::identity<glm::mat4>(), &myskin->Bones[i]);
		}
	}
	else
	{
		printf("skl has an unsupported version: %d\n", myskin->Type);
		scanf("press enter to exit.");
		fclose(fp);
		return 1;
	}

	printf("skl version %d was succesfully loaded: Type: %d Bones: %u BoneIndices: %d\n",
		myskin->Version, myskin->Type, myskin->Bones.size(), myskin->BoneIndices.size());
	fclose(fp);
	return 0;
}

void fixbone(Skin* skn, Skeleton* skl)
{
	for (uint32_t i = 0; i < skn->BoneIndices.size(); i++)
	{
		for (uint32_t k = 0; k < 4; k++)
		{
			skn->BoneIndices[i][k] = (float)skl->BoneIndices[(uint32_t)skn->BoneIndices[i][k]];
		}
	}
}