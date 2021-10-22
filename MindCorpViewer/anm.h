//author https://github.com/autergame
#ifndef _ANM_H_
#define _ANM_H_

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <unordered_map>
#include <bitset>
#include <vector>

#include "MemReader.h"
#include "skl.h"

enum FrameDataType : uint8_t
{
	RotationType = 0,
	TranslationType = 64,
	ScaleType = 128
};

struct BoneAnm
{
	uint32_t Hash = 0;
	std::vector<std::pair<float, glm::vec3>> Translation;
	std::vector<std::pair<float, glm::quat>> Rotation;
	std::vector<std::pair<float, glm::vec3>> Scale;
};

struct Animation
{
	float FPS = 0.f;
	float Duration = 0.f;
	float FrameDelay = 0.f;
	std::vector<BoneAnm> Bones;
};

struct FrameIndices
{
	uint16_t TranslationIndex = 0;
	uint16_t RotationIndex = 0;
	uint16_t ScaleIndex = 0;
};

glm::quat UncompressQuaternion(uint16_t& t_DominantAxis, uint16_t& a_X, uint16_t& a_Y, uint16_t& a_Z)
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

glm::vec3 UncompressVec3(glm::vec3& a_Min, glm::vec3& a_Max, uint16_t& a_X, uint16_t& a_Y, uint16_t& a_Z)
{
	glm::vec3 t_Uncompressed;

	t_Uncompressed = a_Max - a_Min;

	t_Uncompressed.x *= (a_X / 65535.0f);
	t_Uncompressed.y *= (a_Y / 65535.0f);
	t_Uncompressed.z *= (a_Z / 65535.0f);

	t_Uncompressed = t_Uncompressed + a_Min;

	return t_Uncompressed;
}

float UncompressTime(uint16_t& a_CurrentTime, float& a_AnimationLength)
{
	float t_UncompressedTime;

	t_UncompressedTime = a_CurrentTime / 65535.0f;
	t_UncompressedTime = t_UncompressedTime * a_AnimationLength;

	return t_UncompressedTime;
}

int openanm(Animation *myanm, const char* filename)
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

	char signature[8] = { '\0' };
	input.MemRead(signature, 8);

	if (memcmp(signature, "r3d2anmd", 8) != 0 && memcmp(signature, "r3d2canm", 8) != 0)
	{
		printf("Anm has signature %s, which is not known by this application\n", signature);
		fclose(file);
		return 0;
	}

	uint32_t Version = input.MemRead<uint32_t>();

	if (Version == 1)
	{
		int32_t FileSize = input.MemRead<int32_t>();
		FileSize += 12;

		input.m_Pointer += 8;

		uint32_t BoneCount = input.MemRead<uint32_t>();

		int32_t EntryCount = input.MemRead<int32_t>();

		input.m_Pointer += 4;

		input.MemRead(&myanm->Duration, 4);
		input.MemRead(&myanm->FPS, 4);
		myanm->FrameDelay = 1.0f / myanm->FPS;

		input.m_Pointer += 24;

		glm::vec3 TranslationMin;
		input.MemRead(&TranslationMin, 12);

		glm::vec3 TranslationMax;
		input.MemRead(&TranslationMax, 12);

		glm::vec3 ScaleMin;
		input.MemRead(&ScaleMin, 12);

		glm::vec3 ScaleMax;
		input.MemRead(&ScaleMax, 12);

		uint32_t EntriesOffset = input.MemRead<uint32_t>();
		uint32_t IndicesOffset = input.MemRead<uint32_t>();
		uint32_t HashesOffset = input.MemRead<uint32_t>();

		EntriesOffset += 12;
		IndicesOffset += 12;
		HashesOffset += 12;

		input.m_Pointer = HashesOffset;

		std::vector<uint32_t> HashEntries;
		HashEntries.resize(BoneCount);
		input.MemRead(HashEntries.data(), BoneCount * 4);

		input.m_Pointer = EntriesOffset;

		std::vector<std::vector<std::pair<uint16_t, std::bitset<48>>>> CompressedRotations, CompressedTranslations, CompressedScales;
		CompressedRotations.resize(BoneCount);
		CompressedTranslations.resize(BoneCount);
		CompressedScales.resize(BoneCount);

		for (int i = 0; i < EntryCount; ++i)
		{
			uint16_t CompressedTime = input.MemRead<uint16_t>();

			uint8_t HashIndex = input.MemRead<uint8_t>();

			FrameDataType DataType;
			input.MemRead(&DataType, 1);

			std::bitset<48> CompressedData;
			input.MemRead(&CompressedData, 6);

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
			BoneAnm BoneEntry;
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
		input.m_Pointer += 4;

		uint32_t BoneCount = input.MemRead<uint32_t>();
		uint32_t FrameCount = input.MemRead<uint32_t>();

		input.MemRead(&myanm->FPS, 4);
		myanm->FrameDelay = 1.0f / myanm->FPS;
		myanm->Duration = myanm->FrameDelay * FrameCount;

		myanm->Bones.resize(BoneCount);
		for (uint32_t i = 0; i < BoneCount; ++i)
		{
			char Name[32] = { '\0' };
			input.MemRead(Name, 32);
			myanm->Bones[i].Hash = StringToHash(Name, strlen(Name));

			input.m_Pointer += 4;

			myanm->Bones[i].Translation.resize(FrameCount);
			myanm->Bones[i].Rotation.resize(FrameCount);
			myanm->Bones[i].Scale.resize(FrameCount);

			float cumulativeFrameDelay = 0.0f;
			for (uint32_t j = 0; j < FrameCount; ++j)
			{
				myanm->Bones[i].Rotation[j].first = cumulativeFrameDelay;
				myanm->Bones[i].Translation[j].first = cumulativeFrameDelay;
				myanm->Bones[i].Scale[j].first = cumulativeFrameDelay;

				input.MemRead(&myanm->Bones[i].Rotation[j].second, 16);

				input.MemRead(&myanm->Bones[i].Translation[j].second, 12);

				myanm->Bones[i].Scale[j].second = glm::vec3(1.f);
				cumulativeFrameDelay += myanm->FrameDelay;
			}
		}
	}
	else if (Version == 4)
	{
		input.m_Pointer += 16;

		uint32_t BoneCount = input.MemRead<uint32_t>();
		uint32_t FrameCount = input.MemRead<uint32_t>();
		input.MemRead(&myanm->FrameDelay, 4);

		input.m_Pointer += 12;

		myanm->Duration = myanm->FrameDelay * FrameCount;
		myanm->FPS = 1.0f / myanm->FrameDelay;

		uint32_t TranslationDataOffset = input.MemRead<uint32_t>(); 
		uint32_t RotationDataOffset = input.MemRead<uint32_t>();
		uint32_t FrameDataOffset = input.MemRead<uint32_t>();

		TranslationDataOffset += 12;
		RotationDataOffset += 12;
		FrameDataOffset += 12;

		input.m_Pointer = TranslationDataOffset;

		std::vector<glm::vec3> TranslationOrScaleEntries;
		while (input.m_Pointer != RotationDataOffset)
		{
			glm::vec3 Vector;
			input.MemRead(&Vector, 12);
			TranslationOrScaleEntries.emplace_back(Vector);
		}

		input.m_Pointer = RotationDataOffset;

		std::vector<glm::quat> RotationEntries;
		while (input.m_Pointer != FrameDataOffset)
		{
			glm::quat RotationEntry;
			input.MemRead(&RotationEntry, 16);
			RotationEntries.emplace_back(RotationEntry);
		}

		input.m_Pointer = FrameDataOffset;

		std::unordered_map<uint32_t, std::vector<FrameIndices>> BoneMap;
		for (uint32_t i = 0; i < BoneCount; ++i)
		{
			for (uint32_t j = 0; j < FrameCount; ++j)
			{
				uint32_t BoneHash = input.MemRead<uint32_t>();

				FrameIndices FrameIndexData;
				input.MemRead(&FrameIndexData, 6);

				input.m_Pointer += 2;

				BoneMap[BoneHash].emplace_back(FrameIndexData);
			}
		}

		for (auto& BoneIndex : BoneMap)
		{
			float CurrentTime = 0.0f;

			BoneAnm Bone;
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
		int32_t FileSize = input.MemRead<int32_t>();
		FileSize += 12;

		input.m_Pointer += 12;

		uint32_t BoneCount = input.MemRead<uint32_t>();
		uint32_t FrameCount = input.MemRead<uint32_t>();
		input.MemRead(&myanm->FrameDelay, 4);

		myanm->Duration = (float)FrameCount * myanm->FrameDelay;
		myanm->FPS = (float)FrameCount / myanm->Duration;

		int32_t HashesOffset = input.MemRead<int32_t>();
		input.m_Pointer += 8;

		int32_t TranslationFileOffset = input.MemRead<int32_t>();
		int32_t RotationFileOffset = input.MemRead<int32_t>();
		int32_t FrameFileOffset = input.MemRead<int32_t>();

		TranslationFileOffset += 12;
		RotationFileOffset += 12;
		FrameFileOffset += 12;
		HashesOffset += 12;

		std::vector<glm::vec3> Translations;

		uint32_t TranslationCount = (uint32_t)(RotationFileOffset - TranslationFileOffset) / 12;

		input.m_Pointer = TranslationFileOffset;

		for (uint32_t i = 0; i < TranslationCount; ++i)
		{
			glm::vec3 translationEntry;
			input.MemRead(&translationEntry, 12);
			Translations.emplace_back(translationEntry);
		}

		std::vector<std::bitset<48>> RotationEntries;

		uint32_t RotationCount = (uint32_t)(HashesOffset - RotationFileOffset) / 6;

		input.m_Pointer = RotationFileOffset;

		for (uint32_t i = 0; i < RotationCount; ++i)
		{
			std::bitset<48> RotationEntry;
			input.MemRead(&RotationEntry, 6);
			RotationEntries.emplace_back(RotationEntry);
		}

		input.m_Pointer = HashesOffset;

		uint32_t HashCount = (uint32_t)(FrameFileOffset - HashesOffset);
		std::vector<uint32_t> HashEntry;
		HashEntry.resize(HashCount);
		input.MemRead(HashEntry.data(), HashCount);

		myanm->Bones.resize(BoneCount);

		input.m_Pointer = FrameFileOffset;

		float CurrentTime = 0.0f;

		for (uint32_t i = 0; i < FrameCount; ++i)
		{
			for (uint32_t j = 0; j < BoneCount; ++j)
			{
				myanm->Bones[j].Hash = HashEntry[j];

				uint16_t TranslationIndex = input.MemRead<uint16_t>();
				uint16_t ScaleIndex = input.MemRead<uint16_t>();
				uint16_t RotationIndex = input.MemRead<uint16_t>();

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
		scanf_s("press enter to exit.");
		fclose(file);
		return 0;
	}

	printf("anm version %d was succesfully loaded: FPS: %f Duration: %f\n", Version, myanm->FPS, myanm->Duration);
	fclose(file);
	return 1;
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

BoneAnm* GetBonea(Animation *anm, uint32_t NameHash)
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

void SetupHierarchy(std::vector<glm::mat4>* Bones, Bone* SkeletonBone, glm::mat4 Parent, float Time, std::vector<BoneFrameIndexCache>& CurrentFrame, Animation* anm)
{
	glm::mat4 GlobalTransform = Parent; 

	BoneAnm* AnimBone = GetBonea(anm, SkeletonBone->Hash);
	if (AnimBone != nullptr)
	{
		glm::vec3 Translation = FindNearestTime(AnimBone->Translation, Time, CurrentFrame.at(SkeletonBone->ID).Translation);
		glm::quat Rotation = FindNearestTime(AnimBone->Rotation, Time, CurrentFrame.at(SkeletonBone->ID).Rotation);
		glm::vec3 Scale = FindNearestTime(AnimBone->Scale, Time, CurrentFrame.at(SkeletonBone->ID).Scale);
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
		if (SkellyBone == nullptr || SkellyBone->Parent != nullptr) 
			continue;
		SetupHierarchy(BoneTransforms, SkellyBone, glm::identity<glm::mat4>(), Time, CurrentFrame, anm);
	}
}

#endif //_ANM_H_