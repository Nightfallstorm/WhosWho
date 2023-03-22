#pragma once
#include "Database.h"
class NPCSwapper
{
public:
	// Appearance information of an NPC, used to swap and revert
	struct NPCData
	{
		bool valid = false;
		RE::TESNPC* baseNPC;
		RE::TESNPC* faceNPC;
		RE::TESRace* race;
		float height;
		float weight;
		RE::TESModel* skeletonModel;
		bool isFemale;
		bool isBeastRace;
		RE::Color bodyTintColor;
		RE::BSTArray<RE::TESNPC::Layer*>* tintLayers;
		RE::TESObjectARMO* skin;
		RE::TESObjectARMO* farSkin;
		RE::TESNPC::FaceData* faceData;
		RE::TESRace::FaceRelatedData* faceRelatedData;
		RE::TESNPC::HeadRelatedData* headData;
		RE::BGSHeadPart** headParts;
		std::uint8_t numHeadParts;
	};
	NPCData* oldNPCData;
	NPCData* newNPCData;
	RE::FormID currentNPCAppearanceID = 0;

	NPCSwapper(RE::TESNPC* a_baseNPC);

	// Setup new NPC for the current NPC to swap to
	void SetupNewNPCSwap(NPCData* a_newNPCData);

	// Swap NPC appearance to the new NPC
	void Apply();

	// Revert NPC appearance
	void Revert();

private:
	void CopyNPCData(NPCData* a_data, RE::TESNPC* a_baseNPC);
	void CopySkins(NPCData* a_data, RE::TESNPC* a_baseNPC);
	void CopyStats(NPCData* a_data, RE::TESNPC* a_baseNPC);
	void CopyTints(NPCData* a_data, RE::TESNPC* a_baseNPC);
	void CopyFaceData(NPCData* a_data, RE::TESNPC* a_baseNPC);
	void CopySkeletons(NPCData* a_data, RE::TESNPC* a_baseNPC);
	void CopyBeastKeyword(NPCData* a_data, RE::TESNPC* a_baseNPC);
	void ApplyNPCData(NPCData* a_data, RE::TESNPC* a_baseNPC);
};

namespace constants
{
	inline RE::FormID ActorTypeNPC = 0x13794;
	inline RE::FormID IsBeastRace = 0xD61D1;
	inline RE::FormID Maiq = 0x954BF;
	inline RE::FormID Nazeem = 0x13BBF;
	inline RE::FormID MQ101Alduin = 0x32B94;
	inline RE::FormID DefaultRace = 0x19;
	inline RE::FormID DebugNPCToTest = 0x954BF;
}

namespace database
{
	inline std::map<RE::FormID, NPCSwapper*> NPCSwapMap;
	inline RE::BSTArray<RE::TESRace*> allRaces;
	inline std::vector<RE::TESNPC*> allNPCs;
}
