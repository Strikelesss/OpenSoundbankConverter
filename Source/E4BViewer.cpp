#include "Header/E4BViewer.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Header/BankConverter.h"
#include "Header/BinaryReader.h"
#include "Header/E4BFunctions.h"
#include "Header/VoiceDefinitions.h"
#include <ShlObj_core.h>
#include <tchar.h>
#include <fstream>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool E4BViewer::CreateResources()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0u;
	sd.BufferDesc.Height = 0u;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60u;
	sd.BufferDesc.RefreshRate.Denominator = 1u;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hwnd;
	sd.SampleDesc.Count = 1u;
	sd.SampleDesc.Quality = 0u;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	//const auto createDeviceFlags(D3D11_CREATE_DEVICE_DEBUG);
	
	D3D_FEATURE_LEVEL featureLevel;
	constexpr D3D_FEATURE_LEVEL featureLevelArray[2]{D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
	if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0u, featureLevelArray, 2u, D3D11_SDK_VERSION, &sd, m_swapchain.GetAddressOf(),
		m_device.GetAddressOf(), &featureLevel, m_deviceContext.GetAddressOf()) != S_OK) { return false; }

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);
	if(!FAILED(m_swapchain->GetBuffer(0u, IID_PPV_ARGS(&pBackBuffer))))
	{
		m_device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_rtv);

		ShowWindow(m_hwnd, SW_SHOWDEFAULT);
		UpdateWindow(m_hwnd);

		IMGUI_CHECKVERSION();
		ImGui::CreateContext();

		ImGui::StyleColorsDark();

		ImGui_ImplWin32_Init(m_hwnd);
		ImGui_ImplDX11_Init(m_device.Get(), m_deviceContext.Get());
		ImGui::GetIO().IniFilename = nullptr;
		return true;
	}

	return false;
}

void E4BViewer::Render()
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	const ImVec2 windowSize(m_currentWindowSizeX, m_currentWindowSizeY);

	ImGui::SetNextWindowPos(ImVec2());
	ImGui::SetNextWindowSize(windowSize);
	if(ImGui::Begin("##main", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar))
	{
		if(m_isBankOpened && exists(m_openedBank))
		{
			ImGui::OpenPopup(m_openedBank.string().c_str());

			ImGui::SetNextWindowSize(ImVec2(windowSize.x / 1.25f, windowSize.y / 1.25f));
			if(ImGui::BeginPopupModal(m_openedBank.string().c_str(), &m_isBankOpened, ImGuiWindowFlags_NoResize))
			{
				/*
				if(ImGui::InputScalar("Startup Preset # (255 = NO PRESET BOUND)", ImGuiDataType_U32, &m_currentResult.m_currentPreset))
				{
					// TODO: change current preset (either write a new file, or save the last 'end' position and copy, then modify)
				}
				*/

				ImGui::Text("Startup Preset #: %u (255 = NO PRESET BOUND)", m_currentResult.GetCurrentPreset());

				if (ImGui::TreeNode("Presets"))
				{
					int32_t presetIndex(0u);
					for(auto& preset : m_currentResult.GetPresets())
					{
						ImGui::PushID(presetIndex);
						if (ImGui::TreeNode(preset.GetName().data()))
						{
							int32_t voiceIndex(1u);
							for(const auto& voice : preset.GetVoices())
							{
								ImGui::PushID(voiceIndex);
								if (ImGui::TreeNode(std::string("Voice #" + std::to_string(voiceIndex)).c_str()))
								{
									const auto& zoneRange(voice.GetZoneRange());
									const auto& velRange(voice.GetVelocityRange());
									ImGui::Text("Original Key: %s", VoiceDefinitions::GetMIDINoteFromKey(voice.GetOriginalKey()).c_str());
									ImGui::Text("Zone Range: %s-%s", VoiceDefinitions::GetMIDINoteFromKey(zoneRange.first).c_str(), VoiceDefinitions::GetMIDINoteFromKey(zoneRange.second).c_str());
									ImGui::Text("Velocity Range: %u-%u", velRange.first, velRange.second);
									ImGui::Text("Filter Type: %s", voice.GetFilterType().data());
									ImGui::Text("Filter Frequency: %d", voice.GetFilterFrequency());
									ImGui::Text("Pan: %d", voice.GetPan());
									ImGui::Text("Volume: %d", voice.GetVolume());
									ImGui::Text("Fine Tune: %f", voice.GetFineTune());

									ImGui::Text("Release Env: %f", voice.GetAmpEnv().GetRelease1Sec());

									ImGui::TreePop();
								}

								ImGui::PopID();
								++voiceIndex;
							}

							ImGui::TreePop();
						}

						ImGui::PopID();
						++presetIndex;
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Samples"))
				{
					int32_t sampleIndex(0u);
					for(const auto& sample : m_currentResult.GetSamples())
					{
						ImGui::PushID(sampleIndex);
						if (ImGui::TreeNode(sample.GetName().c_str()))
						{
							ImGui::Text("Sample Rate: %u", sample.GetSampleRate());
							ImGui::Text("Loop Start: %u", sample.GetLoopStart());
							ImGui::Text("Loop End: %u", sample.GetLoopEnd());
							ImGui::Text("Loop: %d", sample.IsLooping());
							ImGui::Text("Release: %d", sample.IsReleasing());
							ImGui::Text("Sample Size: %zd", sample.GetData().size());

							ImGui::TreePop();
						}

						ImGui::PopID();
						++sampleIndex;
					}

					ImGui::TreePop();
				}

				if (ImGui::TreeNode("Sequences"))
				{
					int32_t seqIndex(0u);
					for(const auto& seq : m_currentResult.GetSequences())
					{
						ImGui::PushID(seqIndex);

						if(ImGui::TreeNode(seq.GetName().c_str()))
						{
							if (ImGui::Button("Extract Sequence"))
							{
								auto seqPath(std::filesystem::path(seq.GetName()).wstring());
								seqPath.resize(MAX_PATH);

								OPENFILENAME ofn{};
								ofn.lStructSize = sizeof(ofn);
								ofn.hwndOwner = nullptr;
								ofn.lpstrFilter = _T(".mid");
								ofn.lpstrFile = seqPath.data();
								ofn.nMaxFile = MAX_PATH;
								ofn.Flags = OFN_EXPLORER;
								ofn.lpstrDefExt = _T("mid");

								if (GetSaveFileName(&ofn))
								{
									std::ofstream ofs(ofn.lpstrFile, std::ios::binary);

									const auto& seqData(seq.GetMIDISeqData());
									ofs.write(seqData.data(), static_cast<std::streamsize>(seqData.size()));
									ofs.close();

									OutputDebugStringA("Successfully extracted sequence! \n");
								}
							}

							ImGui::TreePop();
						}

						ImGui::PopID();
						++seqIndex;
					}

					ImGui::TreePop();
				}

				ImGui::Dummy(ImVec2(0.f, ImGui::GetWindowSize().y - 115.f));
				if(ImGui::Button("Convert To SF2"))
				{
					constexpr BankConverter converter;
					if(converter.ConvertE4BToSF2(m_currentResult, m_openedBank.filename().replace_extension("").string()))
					{
						OutputDebugStringA("Successfully converted to SF2! \n");
					}
				}

                ImGui::EndPopup();
			}
		}

		if(ImGui::BeginListBox("##banks", ImVec2(windowSize.x * 0.85f, windowSize.y * 0.75f)))
		{
			for(const auto& file : m_bankFiles)
			{
				if(exists(file))
				{
					if(ImGui::Selectable(file.string().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
					{
						if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0))
						{
							m_openedBank = file;

							BinaryReader reader;
							reader.readFile(file);

							m_currentResult.Clear();
							if(E4BFunctions::ProcessE4BFile(reader, m_currentResult)) { m_isBankOpened = true; }
							break;
						}
					}
				}
			}

			ImGui::EndListBox();
		}

		if(ImGui::Button("Refresh Files")) { RefreshFiles(); }
		if (ImGui::Button("Change Path"))
		{
			BROWSEINFO bInfo{};
			bInfo.hwndOwner = m_hwnd;
			bInfo.pidlRoot = nullptr;
			bInfo.lpszTitle = _T("Select a folder");
			bInfo.ulFlags = 0;
			bInfo.lpfn = nullptr;
			bInfo.lParam = 0;
			bInfo.iImage = -1;

			const LPITEMIDLIST lpItem(SHBrowseForFolder(&bInfo));
			if (lpItem != nullptr)
			{
				std::array<TCHAR, MAX_PATH> filename{};
				SHGetPathFromIDList(lpItem, filename.data());
				m_currentSearchPath = std::filesystem::path(filename.data());
				RefreshFiles();
			}
		}

		ImGui::End();
	}

	ImGui::Render();
	m_deviceContext->OMSetRenderTargets(1, m_rtv.GetAddressOf(), nullptr);
	m_deviceContext->ClearRenderTargetView(m_rtv.Get(), CLEAR_COLOR.data());
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_swapchain->Present(0u, 0u);
}

void E4BViewer::RefreshFiles()
{
	m_bankFiles.clear();
	if (exists(m_currentSearchPath))
	{
		for (const auto& it : std::filesystem::recursive_directory_iterator(m_currentSearchPath))
		{
			if (it.exists() && it.is_regular_file())
			{
				const auto& path(it.path());
				const auto ext(path.extension().string());
				if (ext.length() == EMU4_FILE_EXT_A.length() && (std::ranges::equal(ext.begin(), ext.end(), EMU4_FILE_EXT_A.begin(), EMU4_FILE_EXT_A.end())
					|| std::ranges::equal(ext.begin(), ext.end(), EMU4_FILE_EXT_B.begin(), EMU4_FILE_EXT_B.end()))) {
					m_bankFiles.emplace_back(path);
				}
			}
		}
	}
}

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
	const WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_CLASSDC, E4BViewer::WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("test"), nullptr};
    RegisterClassEx(&wc);

    E4BViewer::m_hwnd = CreateWindow(wc.lpszClassName, _T("E4B Viewer"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, NULL, NULL, wc.hInstance, NULL);

    if (!E4BViewer::CreateResources())
    {
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

	E4BViewer::m_currentSearchPath = std::filesystem::current_path();
	E4BViewer::RefreshFiles();

	bool keepRunning(true);
    while (keepRunning)
    {
        MSG msg;
        while (PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
			if (msg.message == WM_QUIT) { keepRunning = false; }
        }

		if (!keepRunning) { break; }
        E4BViewer::Render();
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    DestroyWindow(E4BViewer::m_hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
}

LRESULT E4BViewer::WndProc(const HWND hWnd, const UINT msg, const WPARAM wParam, const LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) { return true; }

	switch (msg)
	{
		case WM_SIZE:
		{
			if (m_device != nullptr && wParam != SIZE_MINIMIZED)
			{
				m_rtv = nullptr;

				const UINT width(LOWORD(lParam));
				const UINT height(HIWORD(lParam));

				m_currentWindowSizeX = static_cast<float>(width);
				m_currentWindowSizeY = static_cast<float>(height);
				m_swapchain->ResizeBuffers(0u, width, height, DXGI_FORMAT_UNKNOWN, 0u);

				Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);
				if (!FAILED(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
				{
					m_device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_rtv);
					return true;
				}
			}
			return 0;
		}

		case WM_SYSCOMMAND:
		{
			// Disable ALT application menu
			if ((wParam & 0xfff0) == SC_KEYMENU) { return 0; }
			break;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			return 0;
		}

		default: { break; }
	}

	return DefWindowProc(hWnd, msg, wParam, lParam);
}