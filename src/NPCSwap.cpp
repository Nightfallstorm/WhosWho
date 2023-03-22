#pragma once

#include "NPCSwap.h"
#include "Database.h"

// TODO: Switch name

NPCSwapper::NPCSwapper(RE::TESNPC* a_baseNPC)
{
	this->oldNPCData = new NPCData();
	this->CopyNPCData(this->oldNPCData, a_baseNPC);
	this->CopySkins(this->oldNPCData, a_baseNPC);
	this->CopyStats(this->oldNPCData, a_baseNPC);
	this->CopyTints(this->oldNPCData, a_baseNPC);
	this->CopyFaceData(this->oldNPCData, a_baseNPC);
	this->CopySkeletons(this->oldNPCData, a_baseNPC);
	this->CopyBeastKeyword(this->oldNPCData, a_baseNPC);
	this->oldNPCData->valid = true;
	this->currentNPCAppearanceID = a_baseNPC->formID;
}

void NPCSwapper::SetupNewNPCSwap(NPCData* a_newNPCData)
{
	if (newNPCData && newNPCData->baseNPC->formID == a_newNPCData->baseNPC->formID) {
		logger::info("Already have new NPC data, ignoring");
		return;
	}
	newNPCData = a_newNPCData;
}

void NPCSwapper::Apply()
{
	assert(oldNPCData->valid);
	assert(newNPCData->valid);
	if (currentNPCAppearanceID != newNPCData->baseNPC->formID) {
		logger::info("Apply swap from {:x} to {:x}", currentNPCAppearanceID, newNPCData->baseNPC->formID);
		ApplyNPCData(newNPCData, oldNPCData->baseNPC);		
		currentNPCAppearanceID = newNPCData->baseNPC->formID;
	}
}

void NPCSwapper::Revert()
{
	assert(oldNPCData->valid);
	assert(newNPCData->valid);
	if (this->currentNPCAppearanceID != oldNPCData->baseNPC->formID) {
		logger::info("Reverting swap from {:x} to {:x}", currentNPCAppearanceID, oldNPCData->baseNPC->formID);
		ApplyNPCData(oldNPCData, oldNPCData->baseNPC);
		this->currentNPCAppearanceID = oldNPCData->baseNPC->formID;
	}	
}

void NPCSwapper::ApplyNPCData(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_baseNPC->height = a_data->height;
	a_baseNPC->weight = a_data->weight;
	if (a_data->isFemale) {
		a_baseNPC->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kFemale);
	} else {
		a_baseNPC->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kFemale);
	}
	if (a_data->isBeastRace) {
		a_baseNPC->AddKeyword(RE::TESForm::LookupByID(constants::IsBeastRace)->As<RE::BGSKeyword>());
	} else {
		// TODO: Can't remove beast keyword from race for NPC without hook?
	}
	a_baseNPC->bodyTintColor = a_data->bodyTintColor;
	
	// skeletonModel applied from hooks

	a_baseNPC->skin = a_data->skin;
	a_baseNPC->farSkin = a_data->farSkin;
	a_baseNPC->tintLayers = a_data->tintLayers;

	a_baseNPC->faceNPC = a_data->faceNPC;
	if (a_baseNPC->faceNPC == a_baseNPC) {
		a_baseNPC->faceNPC = nullptr;
	}
	a_baseNPC->faceData = a_data->faceData;
	a_baseNPC->headRelatedData = a_data->headData;
	a_baseNPC->headParts = a_data->headParts;
	a_baseNPC->numHeadParts = a_data->numHeadParts;
}

void NPCSwapper::CopyNPCData(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->baseNPC = a_baseNPC;
	a_data->faceNPC = a_baseNPC->faceNPC;
	a_data->race = a_baseNPC->race;
}

void NPCSwapper::CopySkins(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	auto& newSkin = a_baseNPC->skin ? a_baseNPC->skin : a_baseNPC->race->skin;
	auto& newFarSkin = a_baseNPC->farSkin;

	a_data->skin = newSkin; 
	a_data->farSkin = newFarSkin;
}

// TODO: Unbake this from save? Or just require save unbaker
void NPCSwapper::CopyStats(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->weight = a_baseNPC->weight;
	a_data->height = a_baseNPC->height;
	a_data->isFemale = a_baseNPC->IsFemale();
}

void NPCSwapper::CopyTints(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->bodyTintColor = a_baseNPC->bodyTintColor;
	a_data->tintLayers = a_baseNPC->tintLayers;
}
void NPCSwapper::CopyFaceData(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->faceData = a_baseNPC->faceData;
	a_data->headData = a_baseNPC->headRelatedData;
	a_data->headParts = a_baseNPC->headParts;
	a_data->numHeadParts = a_baseNPC->numHeadParts;
	a_data->faceRelatedData = a_baseNPC->race->faceRelatedData[a_baseNPC->GetSex()];
}
void NPCSwapper::CopySkeletons(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->skeletonModel = &a_baseNPC->race->skeletonModels[a_baseNPC->GetSex()];
}
void NPCSwapper::CopyBeastKeyword(NPCData* a_data, RE::TESNPC* a_baseNPC)
{
	a_data->isBeastRace = a_baseNPC->HasKeywordID(constants::IsBeastRace) ||
	                      a_baseNPC->race->HasKeywordID(constants::IsBeastRace);
}
