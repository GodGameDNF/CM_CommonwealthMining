#include <vector>
#include <iterator> // size
#include <random>

struct MountStruct
{
	RE::MESSAGEBOX_BUTTON button;
	uint32_t iRank;
};

constexpr int buttonCount = 50;
RE::MESSAGEBOX_BUTTON buttonArray[buttonCount];

int iMessageLength = 0;
std::vector<RE::TESObjectREFR*> refList;


float GetRandomPercent(std::monostate)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_real_distribution<float> dist(0, 100);
	float randNum = dist(mt);

	return randNum;
}

uint32_t GetRandomInt(std::monostate, uint32_t i)
{
	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<uint32_t> dist(0, i);
	uint32_t randNum = dist(mt);

	return randNum;
}

void ClearButtonList(RE::BGSMessage* SendMessage)
{
	using func_t = decltype(&ClearButtonList);
	REL::Relocation<func_t> func{ REL::ID(1127748) };
	return func(SendMessage);
}

void DeletePointer(std::monostate, RE::BGSMessage* SendMessage)
{

	//RemoveButton(SendMessage, tete);
	//delete[] ButtonArray;
	iMessageLength = 0;
	//delete tete;
}


std::string ProcessString(std::string_view inputView, RE::TESForm* SendForm)
{
	// 소괄호의 시작 위치 찾기
	size_t openParenthesisPos = inputView.find('(');

	// 소괄호가 없으면 그대로 반환
	if (openParenthesisPos == std::string_view::npos) {
		return std::string(inputView);
	}

	// 소괄호의 끝 위치 찾기
	size_t closeParenthesisPos = inputView.find(')', openParenthesisPos);

	// 소괄호가 없으면 그대로 반환
	if (closeParenthesisPos == std::string_view::npos) {
		return std::string(inputView);
	}

	// 소괄호 안의 내용 추출
	std::string_view contentInsideParentheses = inputView.substr(openParenthesisPos + 1, closeParenthesisPos - openParenthesisPos - 1);

	// 대괄호로 묶어서 반환
	RE::TESDataHandler* g_dataHandler = RE::TESDataHandler::GetSingleton();
	if (g_dataHandler) {
		RE::TESForm* myForm = g_dataHandler->LookupForm(0x000008B6, "CM_CommonwealthMining.esp");
		RE::TESGlobal* myglobal = myForm->As<RE::TESGlobal>();
		if (myglobal->GetValue() != 0) {
			return "[" + std::string(contentInsideParentheses) + "] " + std::string(inputView.substr(0, openParenthesisPos)) + std::string(inputView.substr(closeParenthesisPos + 1));
		} else {
			return std::string(inputView.substr(0, openParenthesisPos)) + std::string(inputView.substr(closeParenthesisPos + 1));
		}
	}
}

using VGAData = RE::BSScript::structure_wrapper<"CM_MiningPropertyScript", "VGAData">;

std::vector<uint32_t> ChangeMessage(std::monostate, RE::BGSMessage* SendMessage, RE::BGSListForm* SendList, std::vector<VGAData> send)
{
	float fCapsMult;

	RE::TESDataHandler* g_dataHandler = RE::TESDataHandler::GetSingleton();
	if (g_dataHandler) {
		RE::TESForm* myForm = g_dataHandler->LookupForm(0x00000822, "CM_CommonwealthMining.esp");
		RE::TESGlobal* myglobal = myForm->As<RE::TESGlobal>();
		fCapsMult = myglobal->GetValue();
	} else {
		fCapsMult = 1;
	}

	uint32_t tempintarray[buttonCount];
	VGAData tempData;
	RE::Actor* PlayerRef = RE::PlayerCharacter::GetSingleton();

	RE::TESForm* tempVGA = nullptr;
	uint32_t ListSize = static_cast<uint32_t>(SendList->arrayOfForms.size());
	
	uint32_t c;
	uint32_t buttonIndex = 0;

	for (int i = 0; i < ListSize; i++) {
		tempVGA = SendList->arrayOfForms[i];
		bool btemp = PlayerRef->GetItemCount(c, tempVGA, false);
		if (c > 0) {
			std::string_view sName = RE::TESFullName::GetFullName(*tempVGA, false);

			sName = ProcessString(sName, tempVGA);

			tempData = send[i];
			std::optional<uint32_t> iScore = tempData.find<uint32_t>("iScore");
			std::optional<uint32_t> iTDP = tempData.find<uint32_t>("iTDP");

			std::string combinedText;
			combinedText += sName;
			combinedText += "(";

			if (iScore) {
				combinedText += std::to_string(static_cast<int> (* iScore * fCapsMult));
			}

			combinedText += "/";

			if (iTDP) {
				combinedText += std::to_string(*iTDP);
			}

			combinedText += ")";
			buttonArray[buttonIndex].text = combinedText.c_str();
			buttonArray[buttonIndex].conditions.head = nullptr;

			tempintarray[buttonIndex] = i;

			buttonIndex++;
		}
	}

	std::vector<uint32_t> returnIndexArray(std::begin(tempintarray), std::begin(tempintarray) + buttonIndex + 1);

	if (buttonIndex > 0) {
		buttonArray[buttonIndex].text = "나가기";
	} else {
		buttonArray[buttonIndex].text = "가지고 있는 그래픽카드가 없습니다";
	}
	buttonArray[buttonIndex].conditions.head = nullptr;



	returnIndexArray[buttonIndex] = 1001;

	// BSSimpleList를 사용하여 역순으로 버튼을 추가. 지금처럼 역순으로 해야 0번부터 메세지창에 보임
	SendMessage->buttonList.clear();

	for (int i = buttonIndex; i >= 0; --i) {
		SendMessage->buttonList.push_front(&buttonArray[i]);
		//SendMessage->buttonList.push_front(&ButtonData[i].button);// & buttonArray[i]);
		//returnIndexArray[buttonIndex] = ButtonData[i].iRank;
	}

	return returnIndexArray;
	
}



bool RegisterPapyrusFunctions(RE::BSScript::IVirtualMachine* a_vm)
{
	REL::IDDatabase::Offset2ID o2i;
	logger::info("0x0x0112670: {}", o2i(0x0112670));

	a_vm->BindNativeMethod("CM_MiningF4SE"sv, "ChangeMessage"sv, ChangeMessage);
	a_vm->BindNativeMethod("CM_MiningF4SE"sv, "DeletePointer"sv, DeletePointer);
	a_vm->BindNativeMethod("CM_MiningF4SE"sv, "GetRandomInt"sv, GetRandomInt);
	a_vm->BindNativeMethod("CM_MiningF4SE"sv, "GetRandomPercent"sv, GetRandomPercent);
	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(const F4SE::QueryInterface* a_f4se, F4SE::PluginInfo* a_info)
{
#ifndef NDEBUG
	auto sink = std::make_shared<spdlog::sinks::msvc_sink_mt>();
#else
	auto path = logger::log_directory();
	if (!path) {
		return false;
	}

	*path /= fmt::format("{}.log", Version::PROJECT);
	auto sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true);
#endif

	auto log = std::make_shared<spdlog::logger>("Global Log"s, std::move(sink));

#ifndef NDEBUG
	log->set_level(spdlog::level::trace);
#else
	log->set_level(spdlog::level::info);
	log->flush_on(spdlog::level::trace);
#endif

	spdlog::set_default_logger(std::move(log));
	spdlog::set_pattern("[%^%l%$] %v"s);

	logger::info("{} v{}", Version::PROJECT, Version::NAME);

	a_info->infoVersion = F4SE::PluginInfo::kVersion;
	a_info->name = Version::PROJECT.data();
	a_info->version = Version::MAJOR;

	if (a_f4se->IsEditor()) {
		logger::critical("loaded in editor");
		return false;
	}

	const auto ver = a_f4se->RuntimeVersion();
	if (ver < F4SE::RUNTIME_1_10_162) {
		logger::critical("unsupported runtime v{}", ver.string());
		return false;
	}

	return true;
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se)
{
	F4SE::Init(a_f4se);

	const F4SE::PapyrusInterface* papyrus = F4SE::GetPapyrusInterface();
	if (papyrus)
		papyrus->Register(RegisterPapyrusFunctions);

	return true;
}
