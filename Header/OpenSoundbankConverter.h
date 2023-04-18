#pragma once
#include "Header/Data/Soundbank.h"
#include "BankReadOptions.h"
#include "BankWriteOptions.h"
#include "ThreadPool.h"
#include <array>
#include <filesystem>
#include <d3d11.h>
#include <wrl/client.h>

struct ImVec2;

constexpr uint32_t MAX_FILES = 100;
constexpr bool ENABLE_TEMP_SETTINGS = true;
constexpr std::string_view SEQ_QUERY_POPUP_NAME = "Sequence Query Result";

namespace E4BViewer
{
	inline bool strCIPred(const uint8_t a, const uint8_t b) { return std::tolower(a) == std::tolower(b); }
	inline bool strCI(const std::string& a, const std::string& b)
	{
		return a.length() == b.length() && std::equal(b.begin(), b.end(), a.begin(), strCIPred);
	}

	[[nodiscard]] bool CreateResources();
	void Render();

	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	// Other

	void AddFilePath(std::filesystem::path&& path);
	inline ThreadPool m_threadPool(4u);

	// Rendering

	constexpr std::array CLEAR_COLOR{0.f,0.f,0.f,0.f};

	inline HWND m_hwnd;
	inline Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	inline Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
	inline Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapchain;
	inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
	inline float m_currentWindowSizeX = 0.f;
	inline float m_currentWindowSizeY = 0.f;

	// UI Specific

    void DisplayConverter(const ImVec2& windowSize);
    void DisplayBankInfoWindow(const std::string_view& popupName);
    void DisplaySeqQueryPopup();
    
    void DisplayOptions();
    
    void DisplayConsole();
    
    inline std::vector<Soundbank> m_tempResults;
	inline std::vector<std::filesystem::path> m_bankFiles{};
	inline std::string m_conversionType;
    inline BankWriteOptions m_writeOptions{};
    inline BankReadOptions m_readOptions{};
	inline bool m_queueClear = false;
	inline bool m_viewDetailsMenuOpen = false;
    inline bool m_seqQueryMenuOpen = false;
}
