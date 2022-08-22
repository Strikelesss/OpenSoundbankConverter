#include "Header/E4BViewer.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Header/BankConverter.h"
#include "Header/BinaryReader.h"
#include "Header/E4BFunctions.h"
#include "Header/E4Result.h"
#include "Header/Logger.h"
#include <ShlObj_core.h>
#include <tchar.h>

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool E4BViewer::CreateResources()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof sd);
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
		if(ImGui::BeginTabBar("##maintabbar"))
		{
			if(ImGui::BeginTabItem("Converter"))
			{
				ImGui::BeginDisabled(m_threadPool.GetNumTasks() > 0);

				if(ImGui::BeginListBox("##banks", ImVec2(windowSize.x * 0.85f, windowSize.y * 0.75f)))
				{
					for(const auto& file : m_bankFiles)
					{
						if(exists(file))
						{
							if(ImGui::Selectable(file.string().c_str(), false, ImGuiSelectableFlags_AllowDoubleClick))
							{
								if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right))
								{
									// TODO: open popup to remove
									break;
								}
							}
						}
					}

					ImGui::EndListBox();
				}

				ImGui::SameLine();

				ImGui::Text("Banks In Progress: %d", m_threadPool.GetNumTasks());

				if(ImGui::Button("Add Files"))
				{
					constexpr auto MAX_FILES(MAX_PATH * 100);

					std::vector<TCHAR> szFile{};
					szFile.resize(MAX_FILES);

					OPENFILENAME ofn{};
					ofn.lStructSize = sizeof ofn;
					ofn.hwndOwner = m_hwnd;
					ofn.lpstrFile = szFile.data();
					ofn.nMaxFile = MAX_FILES;
					ofn.lpstrFilter = _T("Supported Files\0*.e4b;*.sf2");
					ofn.Flags = OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

					if (GetOpenFileName(&ofn))
					{
						auto* str(ofn.lpstrFile);
						if (str != nullptr)
						{
							if (*(str + wcslen(str) + 1) == 0)
							{
								std::filesystem::path path(str);
								if (std::ranges::find(m_bankFiles, path) == m_bankFiles.end())
								{
									if (!m_bankFiles.empty())
									{
										// Skip if extension is not the same
										if (strCI(path.extension().string(), m_bankFiles[0].extension().string()))
										{
											m_bankFiles.emplace_back(std::move(path));
										}
									}
									else
									{
										m_bankFiles.emplace_back(std::move(path));
									}
								}
							}
							else
							{
								const std::wstring_view dir(str);
								str += dir.length() + 1;
								while (*str)
								{
									const std::wstring_view filename(str);
									str += filename.length() + 1;

									std::filesystem::path path(dir);
									path = path.append(filename);

									if (std::ranges::find(m_bankFiles, path) == m_bankFiles.end())
									{
										if (!m_bankFiles.empty())
										{
											// Skip if extension is not the same
											if (!strCI(path.extension().string(), m_bankFiles[0].extension().string())) { continue; }
										}

										m_bankFiles.emplace_back(std::move(path));
									}
								}
							}
						}
					}
				}

				ImGui::BeginDisabled(m_bankFiles.empty());

				ImGui::SameLine();

				if(ImGui::Button("Clear")) { m_conversionType.clear(); m_bankFiles.clear(); }

				ImGui::SetNextItemWidth(ImGui::CalcTextSize(m_conversionType.c_str()).x + 100.f + ImGui::GetStyle().FramePadding.x * 2.f);

				if(ImGui::BeginCombo("Conversion Format", m_conversionType.c_str()))
				{
					if(!m_bankFiles.empty()) { ImGui::BeginDisabled(strCI(m_bankFiles[0].extension().string(), ".SF2")); }

					if(ImGui::Selectable("SF2", strCI(m_conversionType, "SF2"))) { m_conversionType = "SF2"; }

					if(!m_bankFiles.empty()) { ImGui::EndDisabled(); }

					if(!m_bankFiles.empty()) { ImGui::BeginDisabled(strCI(m_bankFiles[0].extension().string(), ".E4B")); }

					if(ImGui::Selectable("E4B", strCI(m_conversionType, "E4B"))) { m_conversionType = "E4B"; }

					if(!m_bankFiles.empty()) { ImGui::EndDisabled(); }

					ImGui::EndCombo();
				}

				ImGui::EndDisabled();

				ImGui::BeginDisabled(m_conversionType.empty());

				if (strCI(m_conversionType, "SF2"))
				{
					// TODO: remove
					if (ImGui::Button("Verify Cords"))
					{
						for (const auto& file : m_bankFiles)
						{
							if (exists(file))
							{
								const auto ext(file.extension().string());
								if (strCI(ext, ".E4B"))
								{
									if (strCI(m_conversionType, "SF2"))
									{
										m_threadPool.queueFunc([&]
										{
											E4Result tempResult;

											BinaryReader reader;
											reader.readFile(file);

											if (E4BFunctions::ProcessE4BFile(reader, tempResult))
											{
												E4BFunctions::IsAccountingForCords(tempResult);
											}
										});
									}
								}
							}
						}
					}
				}

				ImGui::Checkbox("Flip Pan", &m_flipPan);

				if(strCI(m_conversionType, "E4B"))
				{
					ImGui::SameLine();
					ImGui::Checkbox("Is Chicken Translator File", &m_isChickenTranslatorFile);
				}

				ImGui::SameLine();

				if(ImGui::Button("Convert"))
				{
					if (!m_bankFiles.empty())
					{
						BROWSEINFO browseInfo{};
						browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
						browseInfo.lpszTitle = _T("Select a folder to export to");

						ConverterOptions options(m_flipPan, m_useConverterSpecificData, m_isChickenTranslatorFile, true);

						const LPITEMIDLIST pidl(SHBrowseForFolder(&browseInfo));
						if (pidl != nullptr)
						{
							std::array<TCHAR, MAX_PATH> path{};
							SHGetPathFromIDList(pidl, path.data());

							IMalloc* imalloc = nullptr;
							if (SUCCEEDED(SHGetMalloc(&imalloc)))
							{
								imalloc->Free(pidl);
								imalloc->Release();
							}

							options.m_ignoreFileNameSettingSaveFolder = std::filesystem::path(path.data());
						}

						for (const auto& file : m_bankFiles)
						{
							if (exists(file))
							{
								const auto ext(file.extension().string());
								if (strCI(ext, ".E4B"))
								{
									if (strCI(m_conversionType, "SF2"))
									{
										m_threadPool.queueFunc([&, file, options]
										{
											E4Result tempResult;

											BinaryReader reader;
											reader.readFile(file);

											if (E4BFunctions::ProcessE4BFile(reader, tempResult))
											{
												constexpr BankConverter converter;
												if (converter.ConvertE4BToSF2(tempResult, file.filename().replace_extension("").string(), options))
												{
													OutputDebugStringA("Successfully converted to SF2! \n");
												}
											}
										});
									}
								}
								else if(strCI(ext, ".SF2"))
								{
									if (strCI(m_conversionType, "E4B"))
									{
										m_threadPool.queueFunc([&, file, options]
										{
											constexpr BankConverter converter;
											if (converter.ConvertSF2ToE4B(file, file.filename().replace_extension("").string(), options))
											{
												OutputDebugStringA("Successfully converted to E4B! \n");
											}
										});
									}
								}
							}
						}
					}

					m_queueClear = true;
				}

				ImGui::EndDisabled();

				ImGui::EndDisabled();

				if(m_queueClear && m_threadPool.GetNumTasks() == 0)
				{
					m_queueClear = false;
					m_bankFiles.clear();
					m_conversionType.clear();
					m_flipPan = false;
					m_isChickenTranslatorFile = false;
				}

				ImGui::EndTabItem();
			}

			if(ImGui::BeginTabItem("Console"))
			{
				if(ImGui::BeginListBox("##console", {-1.f, -1.f}))
				{
					for(const auto& msg : Logger::GetLogMessages()) 
					{
						ImGui::TextUnformatted(msg.c_str(), msg.data() + msg.length());
					}

					// Auto scroll console
					if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) { ImGui::SetScrollHereY(1.f); }

					ImGui::EndListBox();
				}

				ImGui::EndTabItem();
			}

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	ImGui::Render();
	m_deviceContext->OMSetRenderTargets(1u, m_rtv.GetAddressOf(), nullptr);
	m_deviceContext->ClearRenderTargetView(m_rtv.Get(), CLEAR_COLOR.data());
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	m_swapchain->Present(0u, 0u);
}

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
	const WNDCLASSEX wc{ sizeof(WNDCLASSEX), CS_CLASSDC, E4BViewer::WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, _T("test"), nullptr};
    RegisterClassEx(&wc);

    E4BViewer::m_hwnd = CreateWindow(wc.lpszClassName, _T("E4B Viewer"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 720, NULL, NULL, wc.hInstance, NULL);

    if (!E4BViewer::CreateResources())
    {
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

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