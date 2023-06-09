#pragma once
#include "NPCSwap.h"

class Prank
{
public:
	virtual void StartPrank() = 0;

	virtual void StopPrank() = 0;

	virtual void ProcessTemplateNPC(RE::TESNPC* a_npc) = 0;

	virtual void ProcessFormDelete(RE::FormID a_formID) = 0;

	static Prank* GetBeastPrank();

	static Prank* GetNazeemPrank();
};


class AllBeast : public Prank
{
	std::vector<RE::FormID> uniqueMaleBeastNPCs;

	std::vector<RE::FormID> uniqueFemaleBeastNPCs;

	std::vector<RE::FormID> NPCsToSwap;

	void StartPrank() override;

	void StopPrank() override;

	void ProcessTemplateNPC(RE::TESNPC* a_npc) override;

	void ProcessFormDelete(RE::FormID a_formID) override;
};

class NazeemWhenTalking : public Prank, RE::BSTEventSink<RE::TESActivateEvent>
{
	void StartPrank() override;

	void StopPrank() override;

	void ProcessTemplateNPC(RE::TESNPC* a_npc) override{};

	void ProcessFormDelete(RE::FormID a_formID) override{};

	RE::BSEventNotifyControl ProcessEvent(const RE::TESActivateEvent* a_event, RE::BSTEventSource<RE::TESActivateEvent>* a_eventSource) override
	{
		if (!a_event || !a_event->objectActivated || !a_event->objectActivated.get() || !a_event->objectActivated.get()->As<RE::Character>()) {
			return RE::BSEventNotifyControl::kContinue;
		}

		auto actor = a_event->objectActivated.get()->As<RE::Character>();

		if (!actor->GetRace()->HasKeywordID(constants::ActorTypeNPC)) {
			return RE::BSEventNotifyControl::kContinue;
		}
		auto npcSwapper = NPCSwapper::GetOrPutNPCSwapper(actor);

		if (npcSwapper->currentNPCAppearanceID != constants::Nazeem) {
			logger::info("Swapping to Nazeem: {}", actor->GetDisplayFullName());
			// Apply new NPC aesthetics
			NPCSwapper::ApplySwapToReference(actor, NPCSwapper::GetOrPutNPCSwapper(constants::Nazeem), false, false);

			// Revert after Nazeeming. The reference will reset with Nazeem aesthetics, but next load of their 3D will be normal
			// to fool the player ;)
			//npcSwapper->Revert();
		}
		return RE::BSEventNotifyControl::kContinue;
	}
};
