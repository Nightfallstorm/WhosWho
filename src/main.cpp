#include "Hooks.h"
#include "NPCSwap.h"

struct CEHelper
{
	std::uint64_t CESignature = 0x123456789ABCDEF;
	RE::TESNPC* debugNPC = nullptr;
	RE::TESRace* desiredRace = nullptr;
	RE::TESObjectARMO* armorRaceCheck = nullptr;
} cehelper;

// TODO: REMOVE THIS WHEN DEBUGGING NO LONGER NEEDED
void Debug()
{
	cehelper.debugNPC = RE::TESForm::LookupByID(0x13BBF)->As<RE::TESNPC>();      // Nazeem
	cehelper.desiredRace = RE::TESForm::LookupByID(0x13746)->As<RE::TESRace>();  // Nord
	cehelper.armorRaceCheck = RE::TESForm::LookupByID(0x13746)->As<RE::TESRace>()->skin;
}

bool IsNPCValid(RE::TESNPC* a_npc)
{
	return !a_npc->IsPlayer() &&
	       !a_npc->IsPlayerRef() &&
	       !a_npc->IsPreset() &&
	       !a_npc->IsDynamicForm() &&

	       a_npc->race &&
	       a_npc->race->HasKeywordID(constants::ActorTypeNPC) &&
	       !a_npc->race->IsChildRace();
}

void ObjectReference__Enable(RE::TESObjectREFR* a_self, bool a_abFadeIn, bool a_wait, RE::BSScript::Internal::VirtualMachine* a_vm, RE::VMStackID a_stackID) {
	using func_t = decltype(&ObjectReference__Enable);
	REL::Relocation<func_t> func{ RELOCATION_ID(56038, 0) };  // TODO: AE/VR
	return func(a_self, a_abFadeIn, a_wait, a_vm, a_stackID);
}

class MaiqOnSight : public RE::BSTEventSink<SKSE::CrosshairRefEvent>
{
	std::vector<RE::Character*> swappedNPCs;

	RE::BSEventNotifyControl ProcessEvent(const SKSE::CrosshairRefEvent* a_event, RE::BSTEventSource<SKSE::CrosshairRefEvent>* a_eventSource) override
	{
		if (!a_event || !a_event->crosshairRef || !a_event->crosshairRef.get() || !a_event->crosshairRef.get()->As<RE::Character>()) {
			for (auto& npc : swappedNPCs) {
				if (npc && !npc->IsDisabled()) {
					//database::NPCSwapMap.at(npc->GetBaseObject()->formID)->Revert();
					//npc->Disable();
					//ObjectReference__Enable(npc, false, false, RE::BSScript::Internal::VirtualMachine::GetSingleton(), 0);
				}
			}
			//swappedNPCs.clear();
			return RE::BSEventNotifyControl::kContinue;
		}

		auto actor = a_event->crosshairRef.get()->As<RE::Character>();

		auto baseActor = actor->GetBaseObject()->As<RE::TESNPC>();
		auto extraCreature = actor->extraList.GetByType<RE::ExtraLeveledCreature>();
		auto baseTemplate = extraCreature ? extraCreature->templateBase : nullptr;
		RE::TESNPC* baseTemplateActor = nullptr;
		if (baseTemplate) {
			baseTemplateActor = baseTemplate->As<RE::TESNPC>();
		}

		if (!database::NPCSwapMap.contains(baseActor->formID)) {
			logger::info("Inserting new entry. Base: {:x} Tempalte: {:x}", baseActor->formID, baseTemplateActor ? baseTemplateActor->formID : 0);
			auto npcSwapper = new NPCSwapper(baseActor);
			database::NPCSwapMap.emplace(baseActor->formID, npcSwapper);
			
		}
		auto npcSwapper = database::NPCSwapMap.at(baseActor->formID);

		if (npcSwapper->currentNPCAppearanceID != constants::DebugNPCToTest) {
			// Apply new NPC aesthetics
			npcSwapper->SetupNewNPCSwap(database::NPCSwapMap.at(constants::DebugNPCToTest)->oldNPCData);
			npcSwapper->Apply();

			

			// DEBUG
			RE::TESNPC* Debug = RE::TESForm::LookupByID(constants::DebugNPCToTest)->As<RE::TESNPC>();
			actor->GetRace()->behaviorGraphs[0] = Debug->GetRace()->behaviorGraphs[0];
			actor->GetRace()->behaviorGraphs[1] = Debug->GetRace()->behaviorGraphs[1];

			actor->GetRace()->rootBehaviorGraphNames[0] = Debug->GetRace()->rootBehaviorGraphNames[0];
			actor->GetRace()->rootBehaviorGraphNames[1] = Debug->GetRace()->rootBehaviorGraphNames[1];

			actor->GetRace()->behaviorGraphProjectNames[0] = Debug->GetRace()->behaviorGraphProjectNames[0];
			actor->GetRace()->behaviorGraphProjectNames[1] = Debug->GetRace()->behaviorGraphProjectNames[1];
			//
			// Remove enable state parent if it exists, so we can disable/enable freely to refresh the NPC
			auto enableStateParent = actor->extraList.GetByType<RE::ExtraEnableStateParent>();
			RE::ObjectRefHandle parentHandle;
			if (enableStateParent) {
				parentHandle = enableStateParent->parent;
				enableStateParent->parent = RE::ObjectRefHandle();
			}
			actor->Disable();


			ObjectReference__Enable(actor, false, false, RE::BSScript::Internal::VirtualMachine::GetSingleton(), 0);
			if (enableStateParent) {
				enableStateParent->parent = parentHandle;
			}
			
			swappedNPCs.emplace_back(actor);
		}
		return RE::BSEventNotifyControl::kContinue;
	}
};

void MessageInterface(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		auto [map, lock] = RE::TESForm::GetAllForms();
		//auto sink = new MaiqOnSight();
		//SKSE::GetCrosshairRefEventSource()->AddEventSink(sink);
		if (!database::NPCSwapMap.contains(constants::DebugNPCToTest)) {
			database::NPCSwapMap.insert(std::pair(constants::DebugNPCToTest, new NPCSwapper(RE::TESForm::LookupByID(constants::DebugNPCToTest)->As<RE::TESNPC>())));
		}
		lock.get().LockForWrite();
		for (auto [formID, form] : *map) {
			if (form && form->As<RE::TESNPC>() && IsNPCValid(form->As<RE::TESNPC>())) {
				database::NPCSwapMap.insert(std::pair(formID, new NPCSwapper(form->As<RE::TESNPC>())));
				database::NPCSwapMap.at(formID)->SetupNewNPCSwap(database::NPCSwapMap.at(constants::DebugNPCToTest)->oldNPCData);
				database::NPCSwapMap.at(formID)->Apply();

			}
		}
		lock.get().UnlockForWrite();
		Debug();

		logger::info("Installing hooks!");
		hook::InstallHooks();
		break;
	}
}

void InitializeLog()
{
	auto path = logger::log_directory();
	if (!path) {
		stl::report_and_fail("Failed to find standard logging directory"sv);
	}

	*path /= Version::PROJECT;
	*path += ".log"sv;
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);

	auto log = std::make_shared<spdlog::logger>("global log"s, std::move(sink));

	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::info);

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%H:%M:%S:%e] %v"s);

	logger::info(FMT_STRING("{} v{}"), Version::PROJECT, Version::NAME);
}

extern "C" DLLEXPORT bool SKSEAPI SKSEPlugin_Load(const SKSE::LoadInterface* a_skse)
{
	InitializeLog();
	SKSE::Init(a_skse);
	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageInterface);
	logger::info("Loaded Plugin");
	return true;
}
