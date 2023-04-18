#include "Header/OpenSoundbankConverter.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Header/E4B/Helpers/E4VoiceHelpers.h"
#include "Header/IO/E4BReader.h"
#include "Header/IO/SF2Reader.h"
#include "Header/IO/BinaryWriter.h"
#include "Header/Logger.h"
#include "Header/BankConverter.h"
#include <fstream>
#include <ShlObj_core.h>
#include <tchar.h>

#include "Header/Platforms/Windows/WindowsPlatform.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool E4BViewer::CreateResources()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof sd);
	sd.BufferCount = 2U;
	sd.BufferDesc.Width = 1280U;
	sd.BufferDesc.Height = 720U;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60U;
	sd.BufferDesc.RefreshRate.Denominator = 1U;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = m_hwnd;
	sd.SampleDesc.Count = 1U;
	sd.SampleDesc.Quality = 0U;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;

	constexpr uint32_t createDeviceFlags(
#ifdef _DEBUG
	    D3D11_CREATE_DEVICE_DEBUG
#else
	    0u
#endif
	    );
	
	D3D_FEATURE_LEVEL featureLevel(D3D_FEATURE_LEVEL_10_0);
	constexpr std::array featureLevelArray{D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0};
    auto hr(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
        featureLevelArray.data(), 2u, D3D11_SDK_VERSION, &sd, m_swapchain.GetAddressOf(),
        m_device.GetAddressOf(), &featureLevel, m_deviceContext.GetAddressOf()));
    
    assert(SUCCEEDED(hr));
	if (FAILED(hr))
	{
	    // Attempt to create using WARP:
	    hr = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags,
         featureLevelArray.data(), 2u, D3D11_SDK_VERSION, &sd, m_swapchain.GetAddressOf(),
            m_device.GetAddressOf(), &featureLevel, m_deviceContext.GetAddressOf());

	    assert(SUCCEEDED(hr));
	    if(FAILED(hr)) { return false; }
	}

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);
    hr = SUCCEEDED(m_swapchain->GetBuffer(0u, IID_PPV_ARGS(&pBackBuffer)));
    assert(SUCCEEDED(hr));
	if(SUCCEEDED(hr))
	{
	    hr= m_device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_rtv);
	    assert(SUCCEEDED(hr));
	    if(FAILED(hr)) { Logger::LogMessage("CreateRenderTargetView failed!"); return false; }
	    
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
		if(!m_viewDetailsMenuOpen && !m_seqQueryMenuOpen) { m_tempResults.clear(); }

		if(ImGui::BeginTabBar("##maintabbar"))
		{
			DisplayConverter(windowSize);
		    DisplayOptions();
		    DisplayConsole();

			ImGui::EndTabBar();
		}

		ImGui::End();
	}

	ImGui::Render();
	m_deviceContext->OMSetRenderTargets(1u, m_rtv.GetAddressOf(), nullptr);
	m_deviceContext->ClearRenderTargetView(m_rtv.Get(), CLEAR_COLOR.data());
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    const auto hr(m_swapchain->Present(0u, 0u));
    assert(SUCCEEDED(hr));
    if(FAILED(hr)) { Logger::LogMessage("Failed to present!"); }
}

int WINAPI WinMain(_In_ const HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    constexpr auto windowName(_T("OpenSoundbankConverterGUI"));

    const WNDCLASSEX wc{static_cast<UINT>(sizeof WNDCLASSEX), 0u, E4BViewer::WndProc,
        0, 0, hInstance, LoadIcon(nullptr, IDI_APPLICATION), 
        LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), 
        nullptr, windowName, LoadIcon(nullptr, IDI_APPLICATION)};

    if (!RegisterClassEx(&wc)) { MessageBoxA(nullptr, "Window Registration Failed!", "Error!", 
        MB_ICONEXCLAMATION | MB_OK); return 0; }

    E4BViewer::m_currentWindowSizeX = 1280.f;
    E4BViewer::m_currentWindowSizeY = 720.f;
    
    RECT wr{0,0,static_cast<LONG>(E4BViewer::m_currentWindowSizeX),static_cast<LONG>(E4BViewer::m_currentWindowSizeY)};
    const bool adjustWindowRet(AdjustWindowRectEx(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, FALSE, 0));
    assert(adjustWindowRet);
    if(!adjustWindowRet) { return false; }
    
    E4BViewer::m_hwnd = CreateWindowEx(0, wc.lpszClassName, windowName, (WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME)),
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, wc.hInstance, nullptr);

    if (!E4BViewer::m_hwnd) { MessageBoxA(nullptr, "Window Creation Failed!", "Error", 
        MB_ICONEXCLAMATION | MB_OK); return 0; }

    const bool createResources(E4BViewer::CreateResources());
    assert(createResources);
    if (!createResources)
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
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam) != 0) { return 1; }

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

			    auto hr(m_swapchain->ResizeBuffers(0u, width, height, DXGI_FORMAT_UNKNOWN, 0u));
				assert(SUCCEEDED(hr));
			    if(FAILED(hr)) { Logger::LogMessage("ResizeBuffers failed!"); return 1; }

				Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);
				if (SUCCEEDED(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
				{
				    hr = m_device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_rtv);
				    if(FAILED(hr)) { Logger::LogMessage("CreateRenderTargetView failed!"); return 1; }
					assert(SUCCEEDED(hr));
					return 1;
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

void E4BViewer::AddFilePath(std::filesystem::path&& path)
{
	if (!path.empty())
	{
		if (std::ranges::find(m_bankFiles, path) == m_bankFiles.end())
		{
			if (!m_bankFiles.empty())
			{
				// Skip if extension is not the same as the files contained already
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
}

void E4BViewer::DisplayConverter(const ImVec2& windowSize)
{
    if (ImGui::BeginTabItem("Converter"))
    {
        ImGui::BeginDisabled(m_threadPool.GetNumTasks() > 0);

        if (ImGui::BeginListBox("##banks", ImVec2(windowSize.x * 0.85f, windowSize.y * 0.75f)))
        {
            uint32_t index(0u);
            for (const auto& file : m_bankFiles)
            {
                if (exists(file))
                {
                    const auto fileStr(file.string());
                    ImGui::Selectable(fileStr.c_str());

                    const auto popupRCMenuName("##filePopupRCMenu" + std::to_string(index));
                    ImGui::OpenPopupOnItemClick(popupRCMenuName.c_str());

                    bool tempOpenVDMenu(false);
                    if (ImGui::BeginPopup(popupRCMenuName.c_str()))
                    {
                        ImGui::BeginDisabled(!strCI(file.extension().string(), ".E4B"));

                        if (ImGui::Button("View Details"))
                        {
                            ImGui::CloseCurrentPopup();
                            
                            if (m_tempResults.emplace_back(E4BReader::ProcessFile(file)).IsValid())
                            {
                                tempOpenVDMenu = true;
                                m_viewDetailsMenuOpen = true;
                            }
                        }

                        ImGui::EndDisabled();

                        if (ImGui::Button("Remove"))
                        {
                            m_bankFiles.erase(std::ranges::remove(m_bankFiles, file).begin(), m_bankFiles.end());
                            ImGui::CloseCurrentPopup();
                        }

                        ImGui::EndPopup();
                    }

                    const auto filenameStr(file.filename().string());
                    if (tempOpenVDMenu)
                    {
                        ImGui::OpenPopup(filenameStr.c_str());
                        ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.85f, windowSize.y * 0.85f));
                    }

                    DisplayBankInfoWindow(filenameStr.c_str());

                    ++index;
                }
            }

            ImGui::EndListBox();
        }

        ImGui::SameLine();

        ImGui::Text("Banks In Progress: %d", static_cast<int32_t>(m_threadPool.GetNumTasks()));

        if (!m_bankFiles.empty())
        {
            const auto firstExt(m_bankFiles[0].extension().string());
            if (strCI(firstExt, ".E4B"))
            {
                ImGui::SameLine();
                ImGui::Spacing();

                bool openSeqPopup(false);
                if (ImGui::TreeNode("E4B Mass Actions"))
                {
                    if (ImGui::Button("Sequence Query"))
                    {
                        m_tempResults.clear();
                        
                        for(const auto& file : m_bankFiles)
                        {
                            if(std::filesystem::exists(file))
                            {
                                m_tempResults.emplace_back(E4BReader::ProcessFile(file));
                            }
                        }
                        
                        openSeqPopup = true;
                    }

                    ImGui::TreePop();
                }

                if(openSeqPopup)
                {
                    ImGui::OpenPopup(SEQ_QUERY_POPUP_NAME.data());
                    m_seqQueryMenuOpen = true;
                }
                
                ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.85f, windowSize.y * 0.85f));
                DisplaySeqQueryPopup();
            }
        }

        if (ImGui::Button("Add Files"))
        {
            std::vector<TCHAR> szFile{};
            szFile.resize(MAX_PATH * MAX_FILES);

            OPENFILENAME ofn{};
            ofn.lStructSize = sizeof ofn;
            ofn.hwndOwner = m_hwnd;
            ofn.lpstrFile = szFile.data();
            ofn.nMaxFile = MAX_PATH * MAX_FILES;
            ofn.lpstrFilter = _T("Supported Files\0*.e4b;*.sf2");
            ofn.Flags = OFN_ALLOWMULTISELECT | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_EXPLORER;

            if(GetOpenFileName(&ofn))
            {
                TCHAR* str(ofn.lpstrFile);
                std::wstring directory(str);
                str += directory.length() + 1;

                if(*str == 0)
                {
                    // Only one path detected
                    AddFilePath(std::filesystem::path(directory));
                }
                else
                {
                    // Multiple paths detected
                    while (*str != 0) 
                    {
                        const std::wstring filename(str);
                        AddFilePath(std::filesystem::path(directory + _T("\\") + str));
                        str += filename.length() + 1;
                    }
                }
            }
        }

        ImGui::BeginDisabled(m_bankFiles.empty());

        ImGui::SameLine();

        if (ImGui::Button("Clear"))
        {
            m_conversionType.clear();
            m_bankFiles.clear();
        }

        ImGui::SetNextItemWidth(ImGui::CalcTextSize(m_conversionType.c_str()).x + 100.f + ImGui::GetStyle().FramePadding.x * 2.f);

        if (ImGui::BeginCombo("Conversion Format", m_conversionType.c_str()))
        {
            if (!m_bankFiles.empty()) { ImGui::BeginDisabled(strCI(m_bankFiles[0].extension().string(), ".SF2")); }

            if (ImGui::Selectable("SF2", strCI(m_conversionType, "SF2"))) { m_conversionType = "SF2"; }

            if (!m_bankFiles.empty()) { ImGui::EndDisabled(); }

            if (!m_bankFiles.empty()) { ImGui::BeginDisabled(strCI(m_bankFiles[0].extension().string(), ".E4B")); }

            if (ImGui::Selectable("E4B", strCI(m_conversionType, "E4B"))) { m_conversionType = "E4B"; }

            if (!m_bankFiles.empty()) { ImGui::EndDisabled(); }

            ImGui::EndCombo();
        }

        ImGui::EndDisabled();

        ImGui::BeginDisabled(m_conversionType.empty());

        if (ImGui::Button("Convert"))
        {
            if (!m_bankFiles.empty())
            {
                m_writeOptions.m_saveFolder = WindowsPlatform::GetSaveFolder();
                if (!m_writeOptions.m_saveFolder.empty())
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
                                    m_threadPool.queueFunc([&, file]
                                    {
                                        const auto result(E4BReader::ProcessFile(file));
                                        if (result.IsValid())
                                        {
                                            if (BankConverter::CreateSF2(result, m_writeOptions))
                                            {
                                                Logger::LogMessage("Successfully converted to SF2!");
                                            } 
                                        }
                                    });
                                }
                            }
                            else
                            {
                                if (strCI(ext, ".SF2"))
                                {
                                    if (strCI(m_conversionType, "E4B"))
                                    {
                                        m_threadPool.queueFunc([&, file]
                                        {
                                            const auto result(SF2Reader::ProcessFile(file, m_readOptions));
                                            if (result.IsValid())
                                            {
                                                if (BankConverter::CreateE4B(result, m_writeOptions))
                                                {
                                                    Logger::LogMessage("Successfully converted to E4B!");
                                                }
                                            }
                                        });
                                    }
                                }
                            }
                        }
                    }

                    m_queueClear = true;
                }
            }
        }

        ImGui::EndDisabled();

        ImGui::EndDisabled();

        if (m_queueClear && m_threadPool.GetNumTasks() == 0)
        {
            m_queueClear = false;
            m_bankFiles.clear();
            m_conversionType.clear();
            m_readOptions = {};
            m_writeOptions = {};
        }

        ImGui::EndTabItem();
    }
}

void E4BViewer::DisplayBankInfoWindow(const std::string_view& popupName)
{
    if(m_tempResults.empty()) { return; }
    const auto& bank(m_tempResults[0]);
    
    if (ImGui::BeginPopupModal(popupName.data(), &m_viewDetailsMenuOpen))
    {
        if (ImGui::TreeNode("Presets"))
        {
            int32_t presetIndex(0u);
            for (auto& preset : bank.m_presets)
            {
                ImGui::PushID(presetIndex);
                if (ImGui::TreeNode(std::string("P" + std::format("{:03}", preset.m_index) + " " + preset.m_presetName).c_str()))
                {
                    int32_t voiceIndex(1u);
                    for (const auto& voice : preset.m_voices)
                    {
                        ImGui::PushID(voiceIndex);
                        
                        if (ImGui::TreeNode(std::string("Voice #" + std::to_string(voiceIndex)).c_str()))
                        {
                            const auto& zoneRange(voice.m_keyZone);
                            const auto& velRange(voice.m_velocityZone);
                            ImGui::Text("Original Key: %s", E4VoiceHelpers::GetMIDINoteFromKey(voice.m_originalKey).data());
                            ImGui::Text("Zone Range: %s-%s", E4VoiceHelpers::GetMIDINoteFromKey(zoneRange.m_low).data(),
                                E4VoiceHelpers::GetMIDINoteFromKey(zoneRange.m_high).data());
                            ImGui::Text("Velocity Range: %u-%u", velRange.m_low, velRange.m_high);
                            ImGui::Text("Filter Frequency: %d", voice.m_filterFrequency);
                            ImGui::Text("Pan: %d", voice.m_pan);
                            ImGui::Text("Volume: %d", voice.m_volume);
                            ImGui::Text("Fine Tune: %f", voice.m_fineTune);

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
            for (const auto& sample : bank.m_samples)
            {
                ImGui::PushID(sampleIndex);
                if (ImGui::TreeNode(sample.m_sampleName.c_str()))
                {
                    ImGui::Text("Sample Rate: %u", sample.m_sampleRate);
                    ImGui::Text("Loop Start: %u", sample.m_loopStart);
                    ImGui::Text("Loop End: %u", sample.m_loopEnd);
                    ImGui::Text("Loop: %d", sample.m_isLooping ? 1 : 0);
                    ImGui::Text("Release: %d", sample.m_isLoopReleasing ? 1 : 0);
                    ImGui::Text("Sample Size: %zd", sample.m_sampleData.size());

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
            for (const auto& seq : bank.m_sequences)
            {
                ImGui::PushID(seqIndex);

                if (ImGui::TreeNode(seq.m_sequenceName.c_str()))
                {
                    if (ImGui::Button("Extract Sequence"))
                    {
                        auto seqPathTemp(seq.m_sequenceName);
                        seqPathTemp.resize(MAX_PATH);

                        OPENFILENAMEA ofn{};
                        ofn.lStructSize = sizeof ofn;
                        ofn.hwndOwner = nullptr;
                        ofn.lpstrFilter = ".mid";
                        ofn.lpstrFile = seqPathTemp.data();
                        ofn.nMaxFile = MAX_PATH;
                        ofn.Flags = OFN_EXPLORER;
                        ofn.lpstrDefExt = "mid";

                        if (GetSaveFileNameA(&ofn) != 0)
                        {
                            std::filesystem::path seqPath(seqPathTemp);
                            BinaryWriter writer(seqPath);

                            const auto& seqData(seq.m_midiData);
                            writer.writeType(seqData.data(), sizeof(char) * seqData.size());

                            if (!writer.finishWriting())
                            {
                                Logger::LogMessage("Failed to extract sequence '%s'!", seq.m_sequenceName.c_str());
                            }
                            else
                            {
                                Logger::LogMessage("Sequence '%s' saved to '%ws'!", seq.m_sequenceName.c_str(), seqPath.c_str());
                            }
                        }
                    }

                    ImGui::TreePop();
                }

                ImGui::PopID();
                ++seqIndex;
            }

            ImGui::TreePop();
        }

        ImGui::EndPopup();
    }
}

void E4BViewer::DisplaySeqQueryPopup()
{
    if(ImGui::BeginPopupModal(SEQ_QUERY_POPUP_NAME.data(), &m_seqQueryMenuOpen))
    {
        bool foundSequences(false);
        for(const auto& bank : m_tempResults)
        {
            if(bank.IsValid() && !bank.m_sequences.empty())
            {
                if (ImGui::TreeNode(bank.m_bankName.c_str()))
                {
                    if(ImGui::Button(std::string("Extract " + std::to_string(bank.m_sequences.size()) + " Sequences").c_str()))
                    {
                        const auto saveFolder(WindowsPlatform::GetSaveFolder());
                    
                        for(const auto& seq : bank.m_sequences)
                        {
                            const auto path(std::filesystem::path(saveFolder).append(seq.m_sequenceName + ".mid"));
                            BinaryWriter writer(path);

                            const auto& seqData(seq.m_midiData);
                            writer.writeType(seqData.data(), sizeof(char) * seqData.size());

                            if (!writer.finishWriting())
                            {
                                Logger::LogMessage("Failed to extract sequence '%s'!", seq.m_sequenceName.c_str());
                            }
                            else
                            {
                                Logger::LogMessage("Sequence '%s' saved to '%ws'!", seq.m_sequenceName.c_str(), path.c_str());
                            }
                        }
                    }
                    
                    ImGui::TreePop();
                }

                foundSequences = true;
            }
        }

        if(!foundSequences)
        {
            ImGui::TextUnformatted("Found no sequences :(");
        }

        ImGui::EndPopup();
    }
}

void E4BViewer::DisplayOptions()
{
    if(ImGui::BeginTabItem("Options"))
    {
        if(ImGui::BeginTabBar("Reading"))
        {
            if(ImGui::BeginTabItem("General"))
            {
                ImGui::Checkbox("Flip Pan", &m_readOptions.m_flipPan);
                
                ImGui::Checkbox("Use Converter Specific Data", &m_readOptions.m_useConverterSpecificData);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    ImGui::SetTooltip("Uses specific conversion data from E4BViewer, allowing for more accurate data.");
                }
                
                if (ImGui::TreeNode("Default Filter Settings"))
                {
                    if(ImGui::Button("Apply Emax II defaults"))
                    {
                        m_readOptions.m_filterDefaults = ADSR_Envelope(0., 18.822, 0., 0.f, 0.435, 0.);
                    }

                    if(ImGui::Button("Apply Emulator IV defaults"))
                    {
                        m_readOptions.m_filterDefaults = ADSR_Envelope(0., 0., 0., 0.f, 0.661, 0.);
                    }

                    ImGui::SliderScalar("Attack (s)", ImGuiDataType_Double, &m_readOptions.m_filterDefaults.m_attackSec, &ADSR_EnvelopeStatics::MIN_ADSR, &ADSR_EnvelopeStatics::MAX_ATTACK_TIME);
                    ImGui::SliderScalar("Decay (s)", ImGuiDataType_Double, &m_readOptions.m_filterDefaults.m_decaySec, &ADSR_EnvelopeStatics::MIN_ADSR, &ADSR_EnvelopeStatics::MAX_DECAY_TIME);
                    ImGui::SliderScalar("Hold (s)", ImGuiDataType_Double, &m_readOptions.m_filterDefaults.m_holdSec, &ADSR_EnvelopeStatics::MIN_ADSR, &ADSR_EnvelopeStatics::MAX_HOLD_TIME);
                    ImGui::SliderFloat("Sustain (dB)", &m_readOptions.m_filterDefaults.m_sustainDB, ADSR_EnvelopeStatics::MIN_ADSR, ADSR_EnvelopeStatics::MAX_SUSTAIN_DB);
                    ImGui::SliderScalar("Release (s)", ImGuiDataType_Double, &m_readOptions.m_filterDefaults.m_releaseSec, &ADSR_EnvelopeStatics::MIN_ADSR, &ADSR_EnvelopeStatics::MAX_RELEASE_TIME);
                    
                    ImGui::TreePop();
                }
                
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("E4B"))
            {
                if constexpr(ENABLE_TEMP_SETTINGS)
                {
                    ImGui::Checkbox("Correct Fine Tune (temp)", &m_readOptions.m_e4bOptions.m_useFineTuneCorrection);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    {
                        ImGui::SetTooltip("Corrects a fine tune issue occurring when converting Emax II to E4B via the Emulator IV.");
                    }
                }
                
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }

        if(ImGui::BeginTabBar("Writing"))
        {
            if(ImGui::BeginTabItem("General"))
            {
                ImGui::Checkbox("Use Converter Specific Data", &m_writeOptions.m_useConverterSpecificData);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    ImGui::SetTooltip("Uses specific conversion data from E4BViewer, allowing for more accurate data.");
                }
                
                ImGui::EndTabItem();
            }

            ImGui::EndTabBar();
        }
        
        ImGui::EndTabItem();
    }
}

void E4BViewer::DisplayConsole()
{
    if(ImGui::BeginTabItem("Console"))
    {
        if(ImGui::Button("Export"))
        {
            std::vector<TCHAR> exportPath{};
            exportPath.resize(MAX_PATH);

            OPENFILENAME ofn{};
            ofn.lStructSize = sizeof ofn;
            ofn.hwndOwner = nullptr;
            ofn.lpstrFilter = _T(".txt");
            ofn.lpstrFile = exportPath.data();
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_EXPLORER;
            ofn.lpstrDefExt = _T("txt");

            if (GetSaveFileName(&ofn))
            {
                std::ofstream ofs(ofn.lpstrFile, std::ios::binary);
                for(const auto& msg : Logger::GetLogMessages()) 
                {
                    ofs.write(msg.c_str(), static_cast<std::streamsize>(msg.length()));
                    ofs << std::endl;
                }
            }
        }

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
}
