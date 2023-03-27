#include "Pranks.h"
#include "NPCSwap.h"

static inline Prank* currentPrank;
// Filter for only NPCs this swapping can work on
bool IsNPCValid(RE::TESNPC* a_npc)
{
	return !a_npc->IsPlayer() &&
	       !a_npc->IsPlayerRef() &&
	       !a_npc->IsPreset() &&
	       !a_npc->IsDynamicForm() &&

	       a_npc->race &&
	       a_npc->race->HasKeywordID(constants::ActorTypeNPC);
}

void Prank::SetCurrentPrank(Prank* a_prank) {
	currentPrank = a_prank;
}

Prank* Prank::GetCurrentPrank() {
	return currentPrank;
}

void AllBeast::StartPrank()
{
	std::srand(0x123123);
	auto [map, lock] = RE::TESForm::GetAllForms();
	lock.get().LockForWrite();
	for (auto& [formID, form] : *map) {
		RE::TESNPC* NPC = form->As<RE::TESNPC>();
		if (NPC && IsNPCValid(NPC)) {
			if (NPC->race->HasKeywordID(constants::IsBeastRace)) {
				logger::info("Inserting beast NPC: {:x}", NPC->formID);
				if (NPC->IsFemale()) {
					uniqueFemaleBeastNPCs.emplace_back(NPC->formID);
				} else {
					uniqueMaleBeastNPCs.emplace_back(NPC->formID);
				}
				
			} else {
				NPCsToSwap.emplace_back(NPC->formID);
			}
		}
	}

	for (auto swappableNPC : NPCsToSwap) {
		auto BeastNPCID = 0;
		if (RE::TESForm::LookupByID(swappableNPC)->As<RE::TESNPC>()->IsFemale()) {
			BeastNPCID = uniqueFemaleBeastNPCs[rand() % uniqueFemaleBeastNPCs.size()];
		} else {
			BeastNPCID = uniqueMaleBeastNPCs[rand() % uniqueMaleBeastNPCs.size()];
		}
		
		auto humanSwapper = NPCSwapper::GetOrPutNPCSwapper(swappableNPC);
		auto beastSwapper = NPCSwapper::GetOrPutNPCSwapper(BeastNPCID);
		if (humanSwapper && beastSwapper) {
			humanSwapper->SetupNewNPCSwap(beastSwapper->oldNPCData);
			humanSwapper->Apply();
		}
	}

	lock.get().UnlockForWrite();
}

void AllBeast::StopPrank() {
	for (auto& swappableNPC : NPCsToSwap) {
		if (NPCSwapper::GetNPCSwapper(swappableNPC)) {
			NPCSwapper::GetNPCSwapper(swappableNPC)->Revert();
		}
	}
	NPCsToSwap.clear();
	uniqueMaleBeastNPCs.clear();
	uniqueFemaleBeastNPCs.clear();
}

void AllBeast::ProcessTemplateNPC(RE::TESNPC* a_npc) {
	RE::FormID BeastID = 0;
	if (a_npc->IsFemale()) {
		BeastID = uniqueFemaleBeastNPCs[rand() % uniqueFemaleBeastNPCs.size()];
	} else {
		BeastID = uniqueMaleBeastNPCs[rand() % uniqueMaleBeastNPCs.size()];
	}
	
	logger::info("Form ID: {:x}, Beast ID: {:x}", a_npc->formID, BeastID);
	auto beastNPC = NPCSwapper::GetOrPutNPCSwapper(BeastID);
	if (beastNPC) {
		NPCSwapper::GetOrPutNPCSwapper(a_npc)->SetupNewNPCSwap(beastNPC->oldNPCData);
		NPCSwapper::GetOrPutNPCSwapper(a_npc)->Apply();
	}
}


void AllBeast::ProcessFormDelete(RE::FormID a_formID) {

	// TODO;
}

void NazeemWhenTalking::StartPrank() {
	RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(this);
}

void NazeemWhenTalking::StopPrank()
{
	RE::ScriptEventSourceHolder::GetSingleton()->RemoveEventSink(this);
}
