#pragma once
#include "E4Result.h"
#include "ThreadPool.h"
#include <array>
#include <d3d11.h>
#include <filesystem>
#include <wrl/client.h>

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

	inline ThreadPool m_threadPool(8u);

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

	inline E4Result m_vdTempResult;
	inline std::vector<std::filesystem::path> m_bankFiles{};
	inline std::string m_conversionType;
	inline bool m_flipPan = false, m_useConverterSpecificData = true, m_isChickenTranslatorFile = false;
	inline bool m_queueClear = false;
	inline bool m_vdMenuOpen = false;
}