//author https://github.com/autergame
#pragma once

uint32_t FNV1Hash(const std::string& a_String)
{
	size_t Hash = 0x811c9dc5;
	const char* Chars = a_String.c_str();
	for (size_t i = 0; i < a_String.length(); i++)
		Hash = (Hash ^ tolower(Chars[i])) * 0x01000193;
	return Hash;
}

struct SubMeshHeader
{
	std::string Name = "";
	uint32_t VertexOffset = 0;
	uint32_t VertexCount = 0;
	uint32_t IndexOffset = 0;
	uint32_t IndexCount = 0;
};

struct Mesh
{
	GLuint texid = 0;
	std::string Name;
	uint32_t Hash = 0;
	uint16_t* Indices{0};
	size_t IndexCount = 0;
};

class Skin
{
public:
	uint16_t Major = 0;
	uint16_t Minor = 0;
	glm::vec3 center = glm::vec3(0.f);
	std::vector<glm::vec3> Positions;
	std::vector<glm::vec2> UVs;
	std::vector<glm::vec4> Weights;
	std::vector<glm::vec4> BoneIndices;
	std::vector<uint16_t> Indices;
	std::vector<Mesh> Meshes;
};

int openskn(Skin *myskin, const char* filename)
{
	uint32_t Offset = 0;
	FILE *fp = fopen(filename, "rb");

	uint32_t Signature = 0;
	fread(&Signature, sizeof(uint32_t), 1, fp); Offset += 4;
	if (Signature != 0x00112233)
	{
		printf("skn has no valid signature\n");
		scanf("press enter to exit.");
		fclose(fp);
		return 1;
	}

	fread(&myskin->Major, sizeof(uint16_t), 1, fp); Offset += 2;
	fread(&myskin->Minor, sizeof(uint16_t), 1, fp); Offset += 2;

	uint32_t SubMeshHeaderCount = 0;
	std::vector<SubMeshHeader> SubMeshHeaders;
	if (myskin->Major > 0)
	{
		fread(&SubMeshHeaderCount, sizeof(uint32_t), 1, fp); Offset += 4;
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

			SubMeshHeaders.emplace_back(Mesh);
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

	glm::vec3 BBMin = glm::vec3(6e6);
	glm::vec3 BBMax = glm::vec3(-6e6);

	uint32_t HasTangents = 0;
	if (myskin->Major == 4)
	{
		uint32_t VertexSize = 0;
		fread(&VertexSize, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&HasTangents, sizeof(uint32_t), 1, fp); Offset += 4;

		fread(&BBMin.x, sizeof(float), 1, fp); Offset += 4;
		fread(&BBMin.y, sizeof(float), 1, fp); Offset += 4;
		fread(&BBMin.z, sizeof(float), 1, fp); Offset += 4;

		fread(&BBMax.x, sizeof(float), 1, fp); Offset += 4;
		fread(&BBMax.y, sizeof(float), 1, fp); Offset += 4;
		fread(&BBMax.z, sizeof(float), 1, fp); Offset += 4;

		Offset += 16;
		fseek(fp, Offset, 0);
	}

	myskin->center.x = (BBMax.x + BBMin.x) / 2;
	myskin->center.y = (BBMax.y + BBMin.y) / 2;
	myskin->center.z = (BBMax.z + BBMin.z) / 2;

	myskin->Indices.resize(IndexCount);
	for (uint32_t i = 0; i < IndexCount; i++)
	{
		fread(&myskin->Indices[i], sizeof(uint16_t), 1, fp);
		Offset += 2;
	}

	myskin->Positions.resize(VertexCount);
	myskin->UVs.resize(VertexCount);
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

		fread(&myskin->Weights[i].x, sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Weights[i].y, sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Weights[i].z, sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->Weights[i].w, sizeof(float), 1, fp); Offset += 4;

		Offset += 12;
		fseek(fp, Offset, 0);

		fread(&myskin->UVs[i].x, sizeof(float), 1, fp); Offset += 4;
		fread(&myskin->UVs[i].y, sizeof(float), 1, fp); Offset += 4;

		if (HasTangents)
		{
			Offset += 4;
			fseek(fp, Offset, 0);
		}

		float WeightError = fabsf(myskin->Weights[i].x + myskin->Weights[i].y + myskin->Weights[i].z + myskin->Weights[i].w - 1.0f);
		if (WeightError > 0.02f)
		{
			printf("WeightError\n");
			scanf("press enter to exit.");
			fclose(fp);
			return 1;
		}
	}

	if (SubMeshHeaderCount > 0)
	{
		myskin->Meshes.resize(SubMeshHeaderCount);
		for (uint32_t i = 0; i < SubMeshHeaderCount; i++)
		{
			myskin->Meshes[i].Name = SubMeshHeaders[i].Name;
			myskin->Meshes[i].Hash = FNV1Hash(SubMeshHeaders[i].Name);

			myskin->Meshes[i].IndexCount = SubMeshHeaders[i].IndexCount;
			myskin->Meshes[i].Indices = &myskin->Indices[SubMeshHeaders[i].IndexOffset];
		}
	}
	else
	{
		myskin->Meshes.resize(1);
		myskin->Meshes[0].IndexCount = IndexCount;
		myskin->Meshes[0].Indices = &myskin->Indices[0];
	}

	printf("skn version %u %u was succesfully loaded: SubMeshHeaderCount: %d IndexCount: %d VertexCount: %d\n",
		myskin->Major, myskin->Minor, SubMeshHeaderCount, IndexCount, VertexCount);
	fclose(fp);
	return 0;
}