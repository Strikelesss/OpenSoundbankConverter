#pragma once
#include <array>
#include <d3d11.h>
#include <filesystem>
#include <vector>
#include <wrl/client.h>
#include "E4Preset.h"

namespace E4BSampleReplacer
{
	constexpr std::string_view EMU4_FILE_EXT_A = ".E4B";
	constexpr std::string_view EMU4_FILE_EXT_B = ".e4b";
	constexpr std::array CLEAR_COLOR{0.f,0.f,0.f,0.f};

	[[nodiscard]] bool CreateResources();
	void Render();
	void RefreshFiles();

	LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	inline Microsoft::WRL::ComPtr<ID3D11Device> m_device;
	inline Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
	inline Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapchain;
	inline Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
	inline float m_currentWindowSizeX = 0.f;
	inline float m_currentWindowSizeY = 0.f;

	inline HWND m_hwnd;
	inline E4Result m_currentResult;
	inline std::vector<std::filesystem::path> m_bankFiles{};
	inline bool m_isBankOpened = false;
	inline std::filesystem::path m_openedBank;
	inline std::filesystem::path m_currentSearchPath;
}
