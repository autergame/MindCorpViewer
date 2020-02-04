#pragma once
#include <windows.h>

uint32_t FNV1Hash(const std::string& a_String)
{
	size_t Hash = 0x811c9dc5;
	const char* Chars = a_String.c_str();
	for (size_t i = 0; i < a_String.length(); i++)
		Hash = ((Hash ^ tolower(Chars[i])) * 0x01000193) % 0x100000000;
	return Hash;
}

struct SubMeshHeader
{
	std::string Name = "";
	uint32_t VertexOffset;
	uint32_t VertexCount;
	uint32_t IndexOffset;
	uint32_t IndexCount;
};

struct Mesh
{
	std::string Name;
	uint32_t Hash;
	size_t VertexCount;

	glm::vec3* Positions;
	glm::vec2* UVs;
	glm::vec4* Tangents;
	glm::vec4* Weights;
	glm::vec4* BoneIndices;

	uint16_t* Indices;
	size_t IndexCount;
};

struct Skin
{
	uint16_t Major;
	uint16_t Minor;

	std::vector<glm::vec3> Positions;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec4> Tangents;
	std::vector<glm::vec4> Weights;
	std::vector<uint16_t> Indices;
	std::vector<glm::vec4> BoneIndices;
	std::vector<Mesh> Meshes;
};

int openskn(Skin *myskin, const char* filename)
{
	FILE *fp;
	fopen_s(&fp, filename, "rb");
	uint32_t Offset = 0;

	uint32_t Signature = 0;
	fread(&Signature, sizeof(uint32_t), 1, fp); Offset += 4;
	if (Signature != 0x00112233)
	{
		printf("skn has no valid signature\n");
		return 1;
	}

	fread(&myskin->Major, sizeof(uint16_t), 1, fp); Offset += 2;
	fread(&myskin->Minor, sizeof(uint16_t), 1, fp); Offset += 2;

	uint32_t SubMeshHeaderCount = 0;
	fread(&SubMeshHeaderCount, sizeof(uint32_t), 1, fp); Offset += 4;

	std::vector<SubMeshHeader> SubMeshHeaders;
	if (myskin->Major > 0)
	{
		for (uint32_t i = 0; i < SubMeshHeaderCount; i++)
		{
			SubMeshHeader Mesh;

			char Name[64];
			fread(Name, sizeof(char), 64, fp); Offset += 64;
			Mesh.Name = Name;

			fread(&Mesh.VertexOffset, sizeof(uint32_t), 1, fp); Offset += 4;
			fread(&Mesh.VertexCount, sizeof(uint32_t), 1, fp); Offset += 4;
			fread(&Mesh.IndexOffset, sizeof(uint32_t), 1, fp); Offset += 4;
			fread(&Mesh.IndexCount, sizeof(uint32_t), 1, fp); Offset += 4;

			SubMeshHeaders.push_back(Mesh);
		}
	}

	if (myskin->Major == 4)
	{
		Offset += 4;
		fseek(fp, Offset, 0);
	}

	uint32_t IndexCount = 0, VertexCount = 0;
	fread(&IndexCount, sizeof(uint32_t), 1, fp); Offset += 4;
	fread(&VertexCount, sizeof(uint32_t), 1, fp); Offset += 4;

	uint32_t HasTangents = 0;
	if (myskin->Major == 4)
	{
		uint32_t VertexSize = 0;
		fread(&VertexSize, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&HasTangents, sizeof(uint32_t), 1, fp); Offset += 4;

		Offset += 40;
		fseek(fp, Offset, 0);
	}

	myskin->Indices.resize(IndexCount);
	for (uint32_t i = 0; i < IndexCount; i++)
	{
		fread(&myskin->Indices[i], sizeof(uint16_t), 1, fp);
		Offset += 2;
	}

	myskin->Positions.resize(VertexCount);
	myskin->UVs.resize(VertexCount);
	myskin->Tangents.resize(VertexCount);
	myskin->Weights.resize(VertexCount);
	myskin->BoneIndices.resize(VertexCount);
	for (uint32_t i = 0; i < VertexCount; i++)
	{
		fread(&myskin->Positions[i].x, sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Positions[i].y, sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Positions[i].z, sizeof(float), 1, fp); Offset += 4;

		for (int j = 0; j < 4; j++)
		{
			uint8_t value = 0;
			fread(&value, sizeof(uint8_t), 1, fp);
			myskin->BoneIndices[i][j] = value;
			Offset += 1;
		}

		fread(&myskin->Weights[i][0], sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Weights[i][1], sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Weights[i][2], sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Weights[i][3], sizeof(float), 1, fp); Offset += 4;

		Offset += 12;
		fseek(fp, Offset, 0);

		fread(&myskin->UVs[i][0], sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->UVs[i][1], sizeof(float), 1, fp); Offset += 4;

		if (HasTangents)
		{
			Offset += 4;
			fseek(fp, Offset, 0);
		}

		float WeightError = fabsf(myskin->Weights[i][0] + myskin->Weights[i][1] + myskin->Weights[i][2] + myskin->Weights[i][3] - 1.0f);
		if (WeightError > 0.02f)
		{
			printf("WeightError\n");
			return 1;
		}
	}

	myskin->Meshes.resize(SubMeshHeaderCount);
	for (uint32_t i = 0; i < SubMeshHeaderCount; i++)
	{
		myskin->Meshes[i].Hash = FNV1Hash(SubMeshHeaders[i].Name);
		myskin->Meshes[i].VertexCount = SubMeshHeaders[i].VertexCount;
		myskin->Meshes[i].IndexCount = SubMeshHeaders[i].IndexCount;

		myskin->Meshes[i].Positions = &myskin->Positions[SubMeshHeaders[i].VertexOffset];
		myskin->Meshes[i].UVs = &myskin->UVs[SubMeshHeaders[i].VertexOffset];
		myskin->Meshes[i].Tangents = &myskin->Tangents[SubMeshHeaders[i].VertexOffset];
		myskin->Meshes[i].Weights = &myskin->Weights[SubMeshHeaders[i].VertexOffset];
		myskin->Meshes[i].BoneIndices = &myskin->BoneIndices[SubMeshHeaders[i].VertexOffset];
		myskin->Meshes[i].Indices = &myskin->Indices[SubMeshHeaders[i].IndexOffset];
	}

	printf("skn version %u %u was succesfully loaded: SubMeshHeaderCount: %d IndexCount: %d VertexCount: %d\n",
		myskin->Major, myskin->Minor, SubMeshHeaderCount, IndexCount, VertexCount);
	return 0;
}