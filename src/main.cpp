#include "Hooks.h"
#include "NPCSwap.h"
#include "Pranks.h"
#include "Settings.h"

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

void MessageInterface(SKSE::MessagingInterface::Message* msg)
{
	switch (msg->type) {
	case SKSE::MessagingInterface::kDataLoaded:
		Debug();
		if (Prank::GetCurrentPrank()) {
			logger::info("Starting prank!");
			Prank::GetCurrentPrank()->StartPrank();
		}
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
	Settings::GetSingleton()->Load();
	auto messaging = SKSE::GetMessagingInterface();
	messaging->RegisterListener(MessageInterface);
	logger::info("Loaded Plugin");
	return true;
}
