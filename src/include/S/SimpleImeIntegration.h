#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#pragma once

#ifdef SIMPLE_EXPORTS
#	define SIMPLE_API __declspec(dllexport)
#else
#	define SIMPLE_API __declspec(dllimport)
#endif

namespace SimpleIME
{
	enum class SkseImeMessage
	{
		IME_INTEGRATION_INIT = 0x100,
		IME_COMPOSITION_RESULT,
	};

	struct SIMPLE_API IntegrationData
	{
		// Other mod must call this to render IME window
		void (*RenderIme)() = nullptr;

		/// <summary>
		/// Try enable IME.
		/// Must use IsWantCaptureInput to check current state because
		/// IME enabled state is updated asynchronously.
		/// </summary>
		bool (*EnableIme)(bool enable) = nullptr;

		// update IME window position (the candidate and composition window)
		void (*UpdateImeWindowPosition)(float posX, float posY) = nullptr;

		/// <summary>
		//  Check current IME want to capture user keyboard input?
		//  Note: iFly won't update conversion mode value
		/// </summary>
		/// <returns>return true if SimpleIME mod enabled and IME not in alphanumeric mode,
		/// otherwise, return false.
		/// </returns>
		bool (*IsWantCaptureInput)() = nullptr;
	};

	static_assert(sizeof(IntegrationData) == 32);

	class SimpleImeIntegration
	{
		using Message = SKSE::MessagingInterface::Message;

	public:
		enum ImeMessage : std::uint32_t
		{
			IME_INTEGRATION_INIT = 0x100,
			IME_COMPOSITION_RESULT,
		};

		static auto HandleMessage(Message* a_msg) -> void;

		// a shortcut to add enable IME check.
		static void EnableImeOnInputTextWidget()
		{
			auto& instance = SimpleIME::SimpleImeIntegration::GetSingleton();
			if (ImGui::IsItemActivated()) {
				instance.EnableIme(true);
				instance.UpdateImeWindowPosition();
			} else if (ImGui::IsItemDeactivated()) {
				instance.EnableIme(false);
			}
		}

		auto EnableIme(bool enable) -> bool;
		auto RenderIme() -> void;
		auto UpdateImeWindowPosition(float posX, float posY) -> void;
		// Update IME window position by last ImGui item position
		auto UpdateImeWindowPosition() -> void;

		auto IsWantCaptureInput() -> bool
		{
			if (!isIntegrated || integrationData == nullptr) {
				return false;
			}

			return integrationData->IsWantCaptureInput();
		}

		static auto GetSingleton() -> SimpleImeIntegration&
		{
			static SimpleImeIntegration g_instance;
			return g_instance;
		}

	private:
		std::atomic_bool isIntegrated = false;
		std::atomic_bool isEnabled = false;
		IntegrationData* integrationData = nullptr;

		auto OnMessageImeIntegrationInit(Message* a_msg) -> void;
		auto OnMessageImeCompositionResult(Message* a_msg) -> void;
	};
}