//author https://github.com/autergame
#pragma once

enum FrameDataType : uint8_t
{
	RotationType = 0,
	TranslationType = 64,
	ScaleType = 128
};

typedef struct Boneanm
{
	uint32_t Hash = 0;
	std::vector<std::pair<float, glm::vec3>> Translation;
	std::vector<std::pair<float, glm::quat>> Rotation;
	std::vector<std::pair<float, glm::vec3>> Scale;
} Boneanm;

typedef struct Animation
{
	float FPS = 0.f;
	float Duration = 0.f;
	float FrameDelay = 0.f;
	std::vector<Boneanm> Bones;
} Animation;

typedef struct FrameIndices
{
	uint16_t TranslationIndex;
	uint16_t RotationIndex;
	uint16_t ScaleIndex;
} FrameIndices;

glm::quat UncompressQuaternion(const uint16_t& t_DominantAxis, const uint16_t& a_X, const uint16_t& a_Y, const uint16_t& a_Z)
{
	float fx = sqrt(2.0f) * ((int)a_X - 16384) / 32768.0f;
	float fy = sqrt(2.0f) * ((int)a_Y - 16384) / 32768.0f;
	float fz = sqrt(2.0f) * ((int)a_Z - 16384) / 32768.0f;
	float fw = sqrt(1.0f - fx * fx - fy * fy - fz * fz);

	glm::quat uq = glm::quat(0.f, 0.f, 0.f, 1.f);

	switch (t_DominantAxis)
	{
	case 0:
		uq.x = fw;
		uq.y = fx;
		uq.z = fy;
		uq.w = fz;
		break;

	case 1:
		uq.x = fx;
		uq.y = fw;
		uq.z = fy;
		uq.w = fz;
		break;

	case 2:
		uq.x = -fx;
		uq.y = -fy;
		uq.z = -fw;
		uq.w = -fz;
		break;

	case 3:
		uq.x = fx;
		uq.y = fy;
		uq.z = fz;
		uq.w = fw;
		break;
	}

	return uq;
}

glm::vec3 UncompressVec3(const glm::vec3& a_Min, const glm::vec3& a_Max, const uint16_t& a_X, const uint16_t& a_Y, const uint16_t& a_Z)
{
	glm::vec3 t_Uncompressed;

	t_Uncompressed = a_Max - a_Min;

	t_Uncompressed.x *= (a_X / 65535.0f);
	t_Uncompressed.y *= (a_Y / 65535.0f);
	t_Uncompressed.z *= (a_Z / 65535.0f);

	t_Uncompressed = t_Uncompressed + a_Min;

	return t_Uncompressed;
}

float UncompressTime(const uint16_t& a_CurrentTime, const float& a_AnimationLength)
{
	float t_UncompressedTime;

	t_UncompressedTime = a_CurrentTime / 65535.0f;
	t_UncompressedTime = t_UncompressedTime * a_AnimationLength;

	return t_UncompressedTime;
}

int openanm(Animation *myanm, const char* filename)
{
	uint32_t Offset = 0;
	FILE *fp = fopen(filename, "rb");
	if (fp == NULL)
		printf("Error opening file: %s %d (%s)\n", filename, errno, strerror(errno));

	char* Signature = (char*)calloc(9, sizeof(char));
	fread(Signature, sizeof(uint8_t), 8, fp); Offset += 8;

	if (memcmp(Signature, "r3d2anmd", 8) != 0 && memcmp(Signature, "r3d2canm", 8) != 0)
	{
		printf("anm has signature %s, which is not known by this application\n", Signature);
		scanf("press enter to exit.");
		fclose(fp);
		return 1;
	}

	uint32_t Version;
	fread(&Version, sizeof(uint32_t), 1, fp); Offset += 4;

	if (Version == 1)
	{
		int32_t FileSize;
		fread(&FileSize, sizeof(int32_t), 1, fp); Offset += 4;
		FileSize += 12;

		Offset += 8;
		fseek(fp, Offset, 0);

		uint32_t BoneCount;
		fread(&BoneCount, sizeof(uint32_t), 1, fp); Offset += 4;

		int32_t EntryCount;
		fread(&EntryCount, sizeof(int32_t), 1, fp); Offset += 4;

		Offset += 4;
		fseek(fp, Offset, 0);

		fread(&myanm->Duration, sizeof(float), 1, fp); Offset += 4;
		fread(&myanm->FPS, sizeof(float), 1, fp); Offset += 4;
		myanm->FrameDelay = 1.0f / myanm->FPS;

		uint32_t FrameCount = (uint32_t)(myanm->Duration * myanm->FPS);
		float FrameDelay = 1.0f / myanm->FPS;

		Offset += 24;
		fseek(fp, Offset, 0);

		glm::vec3 TranslationMin;
		fread(&TranslationMin[0], sizeof(float), 1, fp); 
		fread(&TranslationMin[1], sizeof(float), 1, fp);
		fread(&TranslationMin[2], sizeof(float), 1, fp);

		glm::vec3 TranslationMax;
		fread(&TranslationMax[0], sizeof(float), 1, fp);
		fread(&TranslationMax[1], sizeof(float), 1, fp);
		fread(&TranslationMax[2], sizeof(float), 1, fp);

		glm::vec3 ScaleMin;
		fread(&ScaleMin[0], sizeof(float), 1, fp);
		fread(&ScaleMin[1], sizeof(float), 1, fp); 
		fread(&ScaleMin[2], sizeof(float), 1, fp); 

		glm::vec3 ScaleMax;
		fread(&ScaleMax[0], sizeof(float), 1, fp); 
		fread(&ScaleMax[1], sizeof(float), 1, fp); 
		fread(&ScaleMax[2], sizeof(float), 1, fp);

		uint32_t EntriesOffset, IndicesOffset, HashesOffset;
		fread(&EntriesOffset, sizeof(uint32_t), 1, fp);
		fread(&IndicesOffset, sizeof(uint32_t), 1, fp);
		fread(&HashesOffset, sizeof(uint32_t), 1, fp);

		EntriesOffset += 12;
		IndicesOffset += 12;
		HashesOffset += 12;

		fseek(fp, HashesOffset, 0);
		std::vector<uint32_t> HashEntries;
		HashEntries.resize(BoneCount);
		for (uint32_t i = 0; i < BoneCount; ++i)
			fread(&HashEntries[0], sizeof(uint32_t), BoneCount, fp);

		fseek(fp, EntriesOffset, 0);

		std::vector<std::vector<std::pair<uint16_t, std::bitset<48>>>> CompressedRotations, CompressedTranslations, CompressedScales;
		CompressedRotations.resize(BoneCount);
		CompressedTranslations.resize(BoneCount);
		CompressedScales.resize(BoneCount);

		for (int i = 0; i < EntryCount; ++i)
		{
			uint16_t CompressedTime;
			fread(&CompressedTime, sizeof(uint16_t), 1, fp);

			uint8_t HashIndex;
			fread(&HashIndex, sizeof(uint8_t), 1, fp);

			FrameDataType DataType;
			fread(&DataType, sizeof(uint8_t), 1, fp);

			std::bitset<48> CompressedData;
			fread(&CompressedData, sizeof(uint8_t), 6, fp);

			switch (DataType)
			{
				case FrameDataType::RotationType:
				CompressedRotations[HashIndex].emplace_back(CompressedTime, CompressedData);
				break;

			case FrameDataType::TranslationType:
				CompressedTranslations[HashIndex].emplace_back(CompressedTime, CompressedData);
				break;

			case FrameDataType::ScaleType:
				CompressedScales[HashIndex].emplace_back(CompressedTime, CompressedData);
				break;
			}
		}

		for (uint32_t i = 0; i < BoneCount; ++i)
		{
			Boneanm BoneEntry;
			BoneEntry.Hash = HashEntries[i];

			for (auto &CompressedTranslation : CompressedTranslations[i])
			{
				float Time = UncompressTime(CompressedTranslation.first, myanm->Duration);

				std::bitset<48> Mask = 0xFFFF;
				uint16_t X = static_cast<uint16_t>((CompressedTranslation.second & Mask).to_ulong());
				uint16_t Y = static_cast<uint16_t>((CompressedTranslation.second >> 16 & Mask).to_ulong());
				uint16_t Z = static_cast<uint16_t>((CompressedTranslation.second >> 32 & Mask).to_ulong());

				glm::vec3 Translation = UncompressVec3(TranslationMin, TranslationMax, X, Y, Z);

				BoneEntry.Translation.emplace_back(Time, Translation);
			}

			for (auto &CompressedRotation : CompressedRotations[i])
			{
				float Time = UncompressTime(CompressedRotation.first, myanm->Duration);

				std::bitset<48> Mask = 0x7FFF;
				uint16_t DominantAxis = static_cast<uint16_t>((CompressedRotation.second >> 45).to_ulong());
				uint16_t X = static_cast<uint16_t>((CompressedRotation.second >> 30 & Mask).to_ulong());
				uint16_t Y = static_cast<uint16_t>((CompressedRotation.second >> 15 & Mask).to_ulong());
				uint16_t Z = static_cast<uint16_t>((CompressedRotation.second & Mask).to_ulong());

				glm::quat Rotation = UncompressQuaternion(DominantAxis, X, Y, Z);

				BoneEntry.Rotation.emplace_back(Time, Rotation);
			}

			for (auto &CompressedScale : CompressedScales[i])
			{
				float Time = UncompressTime(CompressedScale.first, myanm->Duration);

				std::bitset<48> Mask = 0xFFFF;
				uint16_t X = static_cast<uint16_t>((CompressedScale.second & Mask).to_ulong());
				uint16_t Y = static_cast<uint16_t>((CompressedScale.second >> 16 & Mask).to_ulong());
				uint16_t Z = static_cast<uint16_t>((CompressedScale.second >> 32 & Mask).to_ulong());

				glm::vec3 Scale = UncompressVec3(ScaleMin, ScaleMax, X, Y, Z);
				BoneEntry.Scale.emplace_back(Time, Scale);
			}

			myanm->Bones.emplace_back(BoneEntry);
		}
	}
	else if (Version == 3)
	{	
		Offset += 4;
		fseek(fp, Offset, 0);

		uint32_t BoneCount, FrameCount;
		fread(&BoneCount, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&FrameCount, sizeof(uint32_t), 1, fp); Offset += 4;

		fread(&myanm->FPS, sizeof(float), 1, fp); Offset += 4;
		myanm->FrameDelay = 1.0f / myanm->FPS;
		myanm->Duration = myanm->FrameDelay * FrameCount;

		char Name[32];
		myanm->Bones.resize(BoneCount);
		for (uint32_t i = 0; i < BoneCount; ++i)
		{
			fread(Name, sizeof(char), 32, fp); Offset += 32;
			myanm->Bones[i].Hash = StringToHash(Name, strlen(Name));

			Offset += 4;
			fseek(fp, Offset, 0);

			myanm->Bones[i].Translation.resize(FrameCount);
			myanm->Bones[i].Rotation.resize(FrameCount);
			myanm->Bones[i].Scale.resize(FrameCount);

			float cumulativeFrameDelay = 0.0f;
			for (uint32_t j = 0; j < FrameCount; ++j)
			{
				myanm->Bones[i].Rotation[j].first = cumulativeFrameDelay;
				myanm->Bones[i].Translation[j].first = cumulativeFrameDelay;
				myanm->Bones[i].Scale[j].first = cumulativeFrameDelay;

				fread(&myanm->Bones[i].Rotation[j].second.x, sizeof(float), 1, fp); Offset += 4;
				fread(&myanm->Bones[i].Rotation[j].second.y, sizeof(float), 1, fp); Offset += 4;
				fread(&myanm->Bones[i].Rotation[j].second.z, sizeof(float), 1, fp); Offset += 4;
				fread(&myanm->Bones[i].Rotation[j].second.w, sizeof(float), 1, fp); Offset += 4;

				fread(&myanm->Bones[i].Translation[j].second.x, sizeof(float), 1, fp); Offset += 4;
				fread(&myanm->Bones[i].Translation[j].second.y, sizeof(float), 1, fp); Offset += 4;
				fread(&myanm->Bones[i].Translation[j].second.z, sizeof(float), 1, fp); Offset += 4;

				myanm->Bones[i].Scale[j].second = glm::vec3(1.f);
				cumulativeFrameDelay += myanm->FrameDelay;
			}
		}
	}
	else if (Version == 4)
	{
		Offset += 16;
		fseek(fp, Offset, 0);

		uint32_t BoneCount, FrameCount;
		fread(&BoneCount, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&FrameCount, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&myanm->FrameDelay, sizeof(float), 1, fp); Offset += 4;

		Offset += 12;
		fseek(fp, Offset, 0);

		myanm->Duration = myanm->FrameDelay * FrameCount;
		myanm->FPS = 1.0f / myanm->FrameDelay;

		uint32_t TranslationDataOffset, RotationDataOffset, FrameDataOffset;
		fread(&TranslationDataOffset, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&RotationDataOffset, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&FrameDataOffset, sizeof(uint32_t), 1, fp); Offset += 4;
		TranslationDataOffset += 12;
		RotationDataOffset += 12;
		FrameDataOffset += 12;

		Offset = TranslationDataOffset;
		fseek(fp, Offset, 0);
		std::vector<glm::vec3> TranslationOrScaleEntries;
		while (Offset != RotationDataOffset)
		{
			glm::vec3 Vector;
			fread(&Vector.x, sizeof(float), 1, fp); Offset += 4;
			fread(&Vector.y, sizeof(float), 1, fp); Offset += 4;
			fread(&Vector.z, sizeof(float), 1, fp); Offset += 4;
			TranslationOrScaleEntries.emplace_back(Vector);
		}

		Offset = RotationDataOffset;
		fseek(fp, Offset, 0);
		std::vector<glm::quat> RotationEntries;
		while (Offset != FrameDataOffset)
		{
			glm::quat RotationEntry;
			fread(&RotationEntry.x, sizeof(float), 1, fp); Offset += 4;
			fread(&RotationEntry.y, sizeof(float), 1, fp); Offset += 4;
			fread(&RotationEntry.z, sizeof(float), 1, fp); Offset += 4;
			fread(&RotationEntry.w, sizeof(float), 1, fp); Offset += 4;
			RotationEntries.emplace_back(RotationEntry);
		}

		Offset = FrameDataOffset;
		fseek(fp, Offset, 0);

		std::unordered_map<uint32_t, std::vector<FrameIndices>> BoneMap;
		for (uint32_t i = 0; i < BoneCount; ++i)
		{
			for (uint32_t j = 0; j < FrameCount;++ j)
			{
				uint32_t BoneHash;
				FrameIndices FrameIndexData;

				fread(&BoneHash, sizeof(uint32_t), 1, fp); Offset += 4;
				fread(&FrameIndexData.TranslationIndex, sizeof(uint16_t), 1, fp); Offset += 2;
				fread(&FrameIndexData.ScaleIndex, sizeof(uint16_t), 1, fp); Offset += 2;
				fread(&FrameIndexData.RotationIndex, sizeof(uint16_t), 1, fp); Offset += 2;

				Offset += 2;
				fseek(fp, Offset, 0);

				BoneMap[BoneHash].emplace_back(FrameIndexData);
			}
		}

		for (auto& BoneIndex : BoneMap)
		{
			float CurrentTime = 0.0f;

			Boneanm Bone;
			Bone.Hash = BoneIndex.first;

			for (auto& Frame : BoneIndex.second)
			{
				Bone.Translation.emplace_back(CurrentTime, TranslationOrScaleEntries[Frame.TranslationIndex]);
				Bone.Rotation.emplace_back(CurrentTime, RotationEntries[Frame.RotationIndex]);
				Bone.Scale.emplace_back(CurrentTime, TranslationOrScaleEntries[Frame.ScaleIndex]);
				CurrentTime += myanm->FrameDelay;
			}

			myanm->Bones.emplace_back(Bone);
		}
	}
	else if (Version == 5)
	{
		int32_t FileSize;
		fread(&FileSize, sizeof(uint32_t), 1, fp); Offset += 4;
		FileSize += 12;

		Offset += 12;
		fseek(fp, Offset, 0);

		uint32_t BoneCount, FrameCount;
		fread(&BoneCount, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&FrameCount, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&myanm->FrameDelay, sizeof(float), 1, fp); Offset += 4;

		myanm->Duration = (float)FrameCount * myanm->FrameDelay;
		myanm->FPS = (float)FrameCount / myanm->Duration;

		int32_t TranslationFileOffset, RotationFileOffset, FrameFileOffset, HashesOffset;

		fread(&HashesOffset, sizeof(uint32_t), 1, fp); Offset += 4;
		Offset += 8;
		fseek(fp, Offset, 0);
		fread(&TranslationFileOffset, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&RotationFileOffset, sizeof(uint32_t), 1, fp); Offset += 4;
		fread(&FrameFileOffset, sizeof(uint32_t), 1, fp); Offset += 4;

		TranslationFileOffset += 12;
		RotationFileOffset += 12;
		FrameFileOffset += 12;
		HashesOffset += 12;

		std::vector<glm::vec3> Translations;

		uint32_t TranslationCount = (uint32_t)(RotationFileOffset - TranslationFileOffset) / (sizeof(float) * 3);

		Offset = TranslationFileOffset;
		fseek(fp, Offset, 0);

		for (uint32_t i = 0; i < TranslationCount; ++i)
		{
			glm::vec3 translationEntry;
			fread(&translationEntry, sizeof(uint8_t), 12, fp); Offset += 12;
			Translations.emplace_back(translationEntry);
		}

		std::vector<std::bitset<48>> RotationEntries;

		uint32_t RotationCount = (uint32_t)(HashesOffset - RotationFileOffset) / 6;

		Offset = RotationFileOffset;
		fseek(fp, Offset, 0);

		for (uint32_t i = 0; i < RotationCount; ++i)
		{
			std::bitset<48> RotationEntry;
			fread(&RotationEntry, sizeof(uint8_t), 6, fp); Offset += 6;
			RotationEntries.emplace_back(RotationEntry);
		}

		Offset = HashesOffset;
		fseek(fp, Offset, 0);

		std::vector<uint32_t> HashEntry;
		uint32_t HashCount = (uint32_t)(FrameFileOffset - HashesOffset) / sizeof(uint32_t);
		HashEntry.resize(HashCount);
		for (uint32_t i = 0; i < HashCount; ++i)
		{
			fread(&HashEntry[i], sizeof(uint32_t), 1, fp); Offset += 4;
		}

		myanm->Bones.resize(BoneCount);

		Offset = FrameFileOffset;
		fseek(fp, Offset, 0);

		float CurrentTime = 0.0f;

		for (uint32_t i = 0; i < FrameCount; ++i)
		{
			for (uint32_t j = 0; j < BoneCount; ++j)
			{
				myanm->Bones[j].Hash = HashEntry[j];

				uint16_t TranslationIndex, RotationIndex, ScaleIndex;
				fread(&TranslationIndex, sizeof(uint16_t), 1, fp); Offset += 2;
				fread(&ScaleIndex, sizeof(uint16_t), 1, fp); Offset += 2;
				fread(&RotationIndex, sizeof(uint16_t), 1, fp); Offset += 2;

				std::bitset<48> mask = 0x7FFF;
				uint16_t flag = (uint16_t)(RotationEntries[RotationIndex] >> 45).to_ulong();
				uint16_t sx = (uint16_t)(RotationEntries[RotationIndex] >> 30 & mask).to_ulong();
				uint16_t sy = (uint16_t)(RotationEntries[RotationIndex] >> 15 & mask).to_ulong();
				uint16_t sz = (uint16_t)(RotationEntries[RotationIndex] & mask).to_ulong();

				glm::quat RotationEntry = UncompressQuaternion(flag, sx, sy, sz);

				myanm->Bones[j].Rotation.emplace_back(CurrentTime, RotationEntry);
				myanm->Bones[j].Scale.emplace_back(CurrentTime, Translations[ScaleIndex]);
				myanm->Bones[j].Translation.emplace_back(CurrentTime, Translations[TranslationIndex]);
			}

			CurrentTime += myanm->FrameDelay;
		}
	}
	else
	{
		printf("anm has an unsupported version: %d\n", Version);
		scanf("press enter to exit.");
		fclose(fp);
		return 1;
	}

	printf("anm version %d was succesfully loaded: FPS: %f Duration: %f\n", Version, myanm->FPS, myanm->Duration);
	fclose(fp);
	return 0;
}

struct BoneFrameIndexCache
{
	size_t Translation = 0;
	size_t Rotation = 0;
	size_t Scale = 0;
};

Bone* GetBones(Skeleton *skl, uint32_t NameHash)
{
	for (size_t i = 0; i < skl->Bones.size(); i++)
		if (skl->Bones[i].Hash == NameHash)
			return &skl->Bones[i];
	return nullptr;
}

Boneanm* GetBonea(Animation *anm, uint32_t NameHash)
{
	for (size_t i = 0; i < anm->Bones.size(); i++)
		if (anm->Bones[i].Hash == NameHash)
			return &anm->Bones[i];
	return nullptr;
}

template<typename T>
T Interpolate(T Low, T High, float Progress)
{
	return glm::lerp(Low, High, Progress);
}

template<>
glm::quat Interpolate(glm::quat Low, glm::quat High, float Progress)
{
	glm::quat Return = glm::quat(0, 0, 0, 1);
	float Dot = Low.w * High.w + Low.x * High.x + Low.y * High.y + Low.z * High.z;
	float InvertedBlend = 1.0f - Progress;

	if (Dot < 0)
	{
		Return.w = InvertedBlend * Low.w + Progress * -High.w;
		Return.x = InvertedBlend * Low.x + Progress * -High.x;
		Return.y = InvertedBlend * Low.y + Progress * -High.y;
		Return.z = InvertedBlend * Low.z + Progress * -High.z;
	}
	else
	{
		Return.w = InvertedBlend * Low.w + Progress * High.w;
		Return.x = InvertedBlend * Low.x + Progress * High.x;
		Return.y = InvertedBlend * Low.y + Progress * High.y;
		Return.z = InvertedBlend * Low.z + Progress * High.z;
	}

	return glm::normalize(Return);
}

template<typename T>
T FindNearestTime(std::vector<std::pair<float, T>>& Vector, float Time, size_t& Index)
{
	auto Min = Vector[0];
	auto Max = Vector.back();

	if (Time > Vector.back().first)
	{
		Min = Vector[Vector.size() - 2];
	}
	else
	{
		if (Time < Vector[Index].first)
			Index = 0;

		Index = Index ? Index - 1 : 0;

		for (; Index < Vector.size(); Index++)
		{
			const auto& Current = Vector[Index];

			if (Current.first <= Time)
			{
				Min = Current;
				continue;
			}

			Max = Current;
			break;
		}
	}

	float Div = Max.first - Min.first;
	float LerpValue = (Div == 0) ? 1 : (Time - Min.first) / Div;
	return Interpolate(Min.second, Max.second, LerpValue);
}

void SetupHierarchy(std::vector<glm::mat4>* Bones, Bone* SkeletonBone, glm::mat4 Parent, float Time, std::vector<BoneFrameIndexCache>* CurrentFrame, Animation* anm)
{
	glm::mat4 GlobalTransform = Parent; 

	Boneanm* AnimBone = GetBonea(anm, SkeletonBone->Hash); 
	if (AnimBone != nullptr)
	{
		glm::vec3 Translation = FindNearestTime(AnimBone->Translation, Time, CurrentFrame->at(SkeletonBone->ID).Translation);
		glm::quat Rotation = FindNearestTime(AnimBone->Rotation, Time, CurrentFrame->at(SkeletonBone->ID).Rotation);
		glm::vec3 Scale = FindNearestTime(AnimBone->Scale, Time, CurrentFrame->at(SkeletonBone->ID).Scale);
		GlobalTransform = Parent * glm::translate(glm::mat4(1.f), Translation) * glm::mat4_cast(Rotation) * glm::scale(glm::mat4(1.f), Scale);
	}

	Bones->at(SkeletonBone->ID) = GlobalTransform * SkeletonBone->InverseGlobalMatrix; 

	for (uint32_t i = 0; i < SkeletonBone->Children.size(); i++)
		SetupHierarchy(Bones, SkeletonBone->Children[i], GlobalTransform, Time, CurrentFrame, anm);
};

void SetupAnimation(std::vector<glm::mat4>* BoneTransforms, float Time, Animation *anm, Skeleton *skl)
{
	std::vector<BoneFrameIndexCache> CurrentFrame; 
	CurrentFrame.resize(skl->Bones.size()); 
	for (size_t i = 0; i < anm->Bones.size(); i++)
	{
		Bone* SkellyBone = GetBones(skl, anm->Bones[i].Hash); 
		if (SkellyBone == nullptr || SkellyBone->Parent != nullptr) continue;
		SetupHierarchy(BoneTransforms, SkellyBone, glm::identity<glm::mat4>(), Time, &CurrentFrame, anm);
	}
}