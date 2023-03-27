#pragma once
#include "Hooks.h"
#include "NPCSwap.h"
#include "Pranks.h"

struct GetTESModelHook
{
	static RE::TESModel* GetTESModel(RE::TESNPC* a_npc)
	{
		if (NPCSwapper::GetNPCSwapper(a_npc->formID)) {
			auto NPCSwapper = NPCSwapper::GetNPCSwapper(a_npc->formID);
			if (NPCSwapper->currentNPCAppearanceID != a_npc->formID) {
				return NPCSwapper->newNPCData->skeletonModel;
			}
		}

		// Original logic
		if (!a_npc->race->skeletonModels[a_npc->GetSex()].model.empty()) {
			return &a_npc->race->skeletonModels[a_npc->GetSex()];
		} else {
			return a_npc->race->skeletonModels;
		}
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: AE
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(19322, 0), REL::VariantOffset(0x5F, 0x0, 0x5F) };
		// Fill the gender and skeleton calls with NOP, as we will handle both gender and skeleton access ourselves
		REL::safe_fill(target.address(), REL::NOP, 0xF);

		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);
		trampoline.write_call<5>(target.address(), reinterpret_cast<uintptr_t>(GetTESModel));

		logger::info("GetTESModelHook hooked at address {:x}", target.address());
		logger::info("GetTESModelHook hooked at offset {:x}", target.offset());
	}
};

struct GetFaceRelatedDataHook
{
	static RE::TESRace::FaceRelatedData* GetFaceData(RE::TESNPC* a_npc)
	{
		if (NPCSwapper::GetNPCSwapper(a_npc->formID)) {
			auto NPCSwapper = NPCSwapper::GetNPCSwapper(a_npc->formID);
			if (NPCSwapper->currentNPCAppearanceID != a_npc->formID) {
				return NPCSwapper->newNPCData->faceRelatedData;
			}
		}

		// Original logic
		return a_npc->race->faceRelatedData[a_npc->GetSex()];
	}

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: AE
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24226, 0), REL::VariantOffset(0x8B, 0x0, 0x8B) };
		REL::safe_fill(target.address(), REL::NOP, 0x10);                                     // Fill with NOP
		std::byte newInstructions[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xC3 };  // mov rbx, rax
		REL::safe_write(target.address() + 0x5, newInstructions, 3);
		auto& trampoline = SKSE::GetTrampoline();
		SKSE::AllocTrampoline(14);
		trampoline.write_call<5>(target.address(), reinterpret_cast<uintptr_t>(GetFaceData));

		logger::info("GetFaceRelatedDataHook hooked at address {:x}", target.address());
		logger::info("GetFaceRelatedDataHook hooked at offset {:x}", target.offset());
	}
};

struct GetFaceRelatedDataHook2
{
	static std::uint64_t thunk(std::uint64_t a_unk, std::uint64_t a_unk1, std::uint64_t a_unk2, RE::TESNPC* a_npc)
	{
		// Swap faceRelatedData for the duration of this function call
		auto oldFaceRelatedData = a_npc->race->faceRelatedData[a_npc->GetSex()];
		if (NPCSwapper::GetNPCSwapper(a_npc->formID)) {
			auto NPCSwapper = NPCSwapper::GetNPCSwapper(a_npc->formID);
			if (NPCSwapper->currentNPCAppearanceID != a_npc->formID) {
				auto NPC = NPCSwapper->newNPCData->baseNPC;
				a_npc->race->faceRelatedData[a_npc->GetSex()] = NPC->race->faceRelatedData[NPC->GetSex()];
			}
		}

		auto result = func(a_unk, a_unk1, a_unk2, a_npc);

		a_npc->race->faceRelatedData[a_npc->GetSex()] = oldFaceRelatedData;
		return result;
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: AE
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(26258, 0), REL::VariantOffset(0x95, 0x0, 0x95) };

		stl::write_thunk_call<GetFaceRelatedDataHook2>(target.address());

		logger::info("GetFaceRelatedDataHook2 hooked at address {:x}", target.address());
		logger::info("GetFaceRelatedDataHook2 hooked at offset {:x}", target.offset());
	}
};

struct LoadTESObjectARMOHook
{
	// Based off 1.5.97 TESObjectARMA::ContainsRace_140226D70(v9[i], a_race)
	static std::uint64_t thunk(std::uint64_t a_unk, RE::TESRace* a_npc, std::uint64_t a_unk1, bool isFemale)
	{
		// a_npc WAS a_race, but our asm kept it as RE::TESNPC for our use case
		auto NPC = reinterpret_cast<RE::TESNPC*>(a_npc);
		auto race = NPC->race;
		if (NPCSwapper::GetNPCSwapper(NPC->formID)) {
			auto NPCSwapper = NPCSwapper::GetNPCSwapper(NPC->formID);
			if (NPCSwapper->currentNPCAppearanceID != NPC->formID) {
				race = NPCSwapper->newNPCData->race;
			}
		}
		return func(a_unk, race, a_unk1, isFemale);  // TODO: Test female
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: AE
		REL::Relocation<std::uintptr_t> target{ RELOCATION_ID(24232, 0), REL::VariantOffset(0x302, 0x0, 0x302) };
		std::byte bytes[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xCA };  // mov rdx, rcx
		REL::safe_fill(target.address() - 0xA, REL::NOP, 0x7);                      // NOP previous instruction (mov rdx, [rcx+0x158])
		REL::safe_write(target.address() - 0xA, bytes, 3);
		stl::write_thunk_call<LoadTESObjectARMOHook>(target.address());

		REL::Relocation<std::uintptr_t> target2{ RELOCATION_ID(24233, 0), REL::VariantOffset(0x78, 0x0, 0x78) };
		std::byte bytes2[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xCA };  // mov rdx, rcx
		REL::safe_fill(target2.address() - 0xE, REL::NOP, 0x7);                      // NOP previous instruction (mov rdx, [rcx+0x158])
		REL::safe_write(target2.address() - 0xE, bytes2, 3);
		stl::write_thunk_call<LoadTESObjectARMOHook>(target2.address());

		REL::Relocation<std::uintptr_t> target3{ RELOCATION_ID(24237, 0), REL::VariantOffset(0xEE, 0x0, 0xEE) };
		std::byte bytes3[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xF2 };  // mov rdx, rsi
		REL::safe_fill(target3.address() - 0xB, REL::NOP, 0x7);                      // NOP previous instruction (mov rdx, [rsi+0x158])
		REL::safe_write(target3.address() - 0xB, bytes3, 3);
		stl::write_thunk_call<LoadTESObjectARMOHook>(target3.address());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target.offset());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target2.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target2.offset());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target3.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target3.offset());
	}
};

struct LoadTESObjectARMOHook2
{
	// Based off 1.5.97 TESObjectARMA::ContainsRace_140226D70(v9[i], a_race)
	static std::uint64_t thunk(std::uint64_t a_unk, RE::TESRace* a_npc, std::uint64_t a_unk1, bool isFemale)
	{
		// a_npc WAS a_race, but our asm kept it as RE::TESNPC for our use case
		auto NPC = reinterpret_cast<RE::TESNPC*>(a_npc);
		auto race = NPC->race;
		if (NPCSwapper::GetNPCSwapper(NPC->formID)) {
			auto NPCSwapper = NPCSwapper::GetNPCSwapper(NPC->formID);
			if (NPCSwapper->currentNPCAppearanceID != NPC->formID) {
				race = NPCSwapper->newNPCData->race;
			}
		}

		return func(a_unk, race, a_unk1, isFemale);  // TODO: Test female
	}

	static inline REL::Relocation<decltype(thunk)> func;

	// Install our hook at the specified address
	static inline void Install()
	{
		// TODO: AE
		REL::Relocation<std::uintptr_t> target4{ RELOCATION_ID(24221, 0), REL::VariantOffset(0x189, 0x0, 0x189) };
		std::byte bytes4[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xDA };  // mov rdx, rbx
		REL::safe_fill(target4.address() - 0x7, REL::NOP, 0x7);                      // NOP previous instruction (mov rdx, [rbx+0x158])
		REL::safe_write(target4.address() - 0x7, bytes4, 3);
		stl::write_thunk_call<LoadTESObjectARMOHook2>(target4.address());

		REL::Relocation<std::uintptr_t> target5{ RELOCATION_ID(24237, 0), REL::VariantOffset(0x52, 0x0, 0x52) };
		std::byte bytes5[] = { (std::byte)0x48, (std::byte)0x89, (std::byte)0xCA };  // mov rdx, rcx
		REL::safe_fill(target5.address() - 0xE, REL::NOP, 0x7);                      // NOP previous instruction (mov rdx, [rcx+0x158])
		REL::safe_write(target5.address() - 0xE, bytes5, 3);
		stl::write_thunk_call<LoadTESObjectARMOHook2>(target5.address());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target4.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target4.offset());

		logger::info("LoadTESObjectARMOHook hooked at address {:x}", target5.address());
		logger::info("LoadTESObjectARMOHook hooked at offset {:x}", target5.offset());
	}
};

struct CopyFromTemplate
{
	static void thunk(RE::TESActorBaseData* a_self, RE::TESActorBase* a_template)
	{
		
		auto NPC = skyrim_cast<RE::TESNPC*, RE::TESActorBaseData>(a_self);
		auto templateNPC = skyrim_cast<RE::TESNPC*, RE::TESActorBase>(a_template);

		if (!NPC || !templateNPC || !Prank::GetCurrentPrank() || !templateNPC->GetRace()->HasKeywordID(constants::ActorTypeNPC)) {
			return;
		}

		if (NPCSwapper::GetNPCSwapper(NPC->formID)) {
			// Data already present, since the base actor is being copied from a new template
			// we will just remove the original swapper data to treat this as a new base actor entirely
			NPCSwapper::RemoveNPCSwapper(NPC->formID);
		}

		func(a_self, a_template);
		ProcessBaseNPC(NPC, templateNPC);
		return;

			
	}

	static void ProcessBaseNPC(RE::TESNPC* NPC, RE::TESNPC* templateNPC) {
		logger::info("Handling base NPC: {} {:x}", NPC->GetFullName(), NPC->formID);
		Prank::GetCurrentPrank()->ProcessTemplateNPC(NPC);
	}

	static void ProcessTemplateNPC(RE::TESNPC* NPC, RE::TESNPC* templateNPC)
	{
		logger::info("Handling template NPC: {} {:x}", templateNPC->GetFullName(), templateNPC->formID);
		Prank::GetCurrentPrank()->ProcessTemplateNPC(templateNPC);
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x4;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 1, CopyFromTemplate>();

		logger::info("CopyFromTemplate hook set");

	}
};

struct CopyNPC
{
	// Maintain NPCSwapper data when copying data between NPCs
	static void thunk(RE::TESNPC* a_self, RE::TESForm* a_other)
	{
		if (!a_other->As<RE::TESNPC>()) {
			func(a_self, a_other); // This should just NOP, but invoke to be safe
			return;
		}

		bool otherNPCSwapped = false;
		NPCSwapper* otherNPCSwapper = NPCSwapper::GetNPCSwapper(a_other->formID);
		if (otherNPCSwapper) {
			// NPCSwapper data existed for other NPC, revert to original appearance for the copy
			if (otherNPCSwapper->currentNPCAppearanceID != a_other->formID) {
				otherNPCSwapper->Revert();
				otherNPCSwapped = true;
			}
		}
		
		func(a_self, a_other);

		// Erase NPCSwapper data, and set it up to match other NPCSwapper data if it exists
		NPCSwapper::RemoveNPCSwapper(a_self->formID);
		if (otherNPCSwapper) {
			NPCSwapper::GetOrPutNPCSwapper(a_self);
			if (otherNPCSwapped) {
				NPCSwapper::GetOrPutNPCSwapper(a_self)->SetupNewNPCSwap(otherNPCSwapper->newNPCData);
				NPCSwapper::GetOrPutNPCSwapper(a_self)->Apply();
			}
		}

		// Reapply swap to other NPC if necessary

		if (otherNPCSwapper && otherNPCSwapped) {
			otherNPCSwapper->Apply();
		}	
	}

	static inline REL::Relocation<decltype(thunk)> func;

	static inline std::uint32_t idx = 0x2F;

	// Install our hook at the specified address
	static inline void Install()
	{
		stl::write_vfunc<RE::TESNPC, 0, CopyNPC>();

		logger::info("CopyNPC hook set");
	}
};

class HandleFormDelete : public RE::BSTEventSink<RE::TESFormDeleteEvent>
{
	RE::BSEventNotifyControl ProcessEvent(const RE::TESFormDeleteEvent* a_event, RE::BSTEventSource<RE::TESFormDeleteEvent>* a_eventSource) override
	{
		NPCSwapper::RemoveNPCSwapper(a_event->formID);

		if (Prank::GetCurrentPrank()) {
			Prank::GetCurrentPrank()->ProcessFormDelete(a_event->formID);
		}

		return RE::BSEventNotifyControl::kContinue;
	}
};

void hook::InstallHooks()
{
	GetTESModelHook::Install();
	GetFaceRelatedDataHook::Install();
	GetFaceRelatedDataHook2::Install();
	LoadTESObjectARMOHook::Install();
	LoadTESObjectARMOHook2::Install();
	RE::ScriptEventSourceHolder::GetSingleton()->AddEventSink(new HandleFormDelete());
	CopyFromTemplate::Install();
	CopyNPC::Install();
}
