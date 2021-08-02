//author https://github.com/autergame
#pragma once

uint32_t FNV1Hash(char* str, size_t strlen)
{
	size_t Hash = 0x811c9dc5;
	for (size_t i = 0; i < strlen; i++)
		Hash = (Hash ^ tolower(str[i])) * 0x01000193;
	return Hash;
}

typedef struct SubMeshHeader
{
	char* Name = 0;
	uint32_t VertexOffset = 0;
	uint32_t VertexCount = 0;
	uint32_t IndexOffset = 0;
	uint32_t IndexCount = 0;
} SubMeshHeader;

typedef struct Mesh
{
	char* Name = 0;
	GLuint texid = 0;
	uint32_t Hash = 0;
	uint16_t* Indices;
	uint32_t IndexCount = 0;
} Mesh;

typedef struct Skin
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
} Skin;

int openskn(Skin *myskn, char* filename)
{
	uint32_t Offset = 0;
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		printf("Error opening file: %s %d (%s)\n", filename, errno, strerror(errno));

	uint32_t Signature = 0;
	fread(&Signature, sizeof(uint32_t), 1, fp); Offset += 4;
	if (Signature != 0x00112233)
	{
		printf("skn has no valid signature\n");
		scanf("press enter to exit.");
		fclose(fp);
		return 1;
	}

	fread(&myskn->Major, sizeof(uint16_t), 1, fp); Offset += 2;
	fread(&myskn->Minor, sizeof(uint16_t), 1, fp); Offset += 2;

	uint32_t SubMeshHeaderCount = 0;
	std::vector<SubMeshHeader> SubMeshHeaders;
	if (myskn->Major > 0)
	{
		fread(&SubMeshHeaderCount, sizeof(uint32_t), 1, fp); Offset += 4;
		SubMeshHeaders.resize(SubMeshHeaderCount);
		for (uint32_t i = 0; i < SubMeshHeaderCount; i++)
		{
			SubMeshHeader Mesh;
			Mesh.Name = (char*)calloc(64, 1);
			fread(Mesh.Name, sizeof(char), 64, fp); Offset += 64;

			fread(&Mesh.VertexOffset, sizeof(uint32_t), 1, fp); Offset += 4;
			fread(&Mesh.VertexCount, sizeof(uint32_t), 1, fp); Offset += 4;
			fread(&Mesh.IndexOffset, sizeof(uint32_t), 1, fp); Offset += 4;
			fread(&Mesh.IndexCount, sizeof(uint32_t), 1, fp); Offset += 4;

			SubMeshHeaders[i] = Mesh;
		}
	}

	if (myskn->Major == 4)
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
	if (myskn->Major == 4)
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

	myskn->center.x = (BBMax.x + BBMin.x) / 2;
	myskn->center.y = (BBMax.y + BBMin.y) / 2;
	myskn->center.z = (BBMax.z + BBMin.z) / 2;

	myskn->Indices.resize(IndexCount);
	for (uint32_t i = 0; i < IndexCount; i++)
	{
		fread(&myskn->Indices[i], sizeof(uint16_t), 1, fp);
		Offset += 2;
	}

	myskn->Positions.resize(VertexCount);
	myskn->BoneIndices.resize(VertexCount);
	myskn->Weights.resize(VertexCount);
	myskn->Normals.resize(VertexCount);
	myskn->UVs.resize(VertexCount);
	for (uint32_t i = 0; i < VertexCount; i++)
	{
		fread(&myskn->Positions[i].x, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->Positions[i].y, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->Positions[i].z, sizeof(float), 1, fp); Offset += 4;

		for (int j = 0; j < 4; j++)
		{
			uint8_t value = 0;
			fread(&value, sizeof(uint8_t), 1, fp);
			myskn->BoneIndices[i][j] = value;
			Offset += 1;
		}

		fread(&myskn->Weights[i].x, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->Weights[i].y, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->Weights[i].z, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->Weights[i].w, sizeof(float), 1, fp); Offset += 4;

		fread(&myskn->Normals[i].x, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->Normals[i].y, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->Normals[i].z, sizeof(float), 1, fp); Offset += 4;

		fread(&myskn->UVs[i].x, sizeof(float), 1, fp); Offset += 4;
		fread(&myskn->UVs[i].y, sizeof(float), 1, fp); Offset += 4;

		if (HasTangents)
		{
			Offset += 4;
			fseek(fp, Offset, 0);
		}

		float WeightError = fabsf(myskn->Weights[i].x + myskn->Weights[i].y + myskn->Weights[i].z + myskn->Weights[i].w - 1.0f);
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
		myskn->Meshes.resize(SubMeshHeaderCount);
		for (uint32_t i = 0; i < SubMeshHeaderCount; i++)
		{
			myskn->Meshes[i].Name = SubMeshHeaders[i].Name;
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
	fclose(fp);
	return 0;
}