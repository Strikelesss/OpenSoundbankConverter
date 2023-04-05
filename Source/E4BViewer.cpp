#include "Header/E4BViewer.h"
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Header/BinaryReader.h"
#include "Header/BinaryWriter.h"
#include "Header/E4BFunctions.h"
#include "Header/E4Result.h"
#include "Header/Logger.h"
#include "Header/VoiceDefinitions.h"
#include <fstream>
#include <ShlObj_core.h>
#include <tchar.h>

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
	constexpr std::array featureLevelArray{D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0,};
	if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags,
	    featureLevelArray.data(), 2u, D3D11_SDK_VERSION, &sd, m_swapchain.GetAddressOf(),
		m_device.GetAddressOf(), &featureLevel, m_deviceContext.GetAddressOf()) != S_OK) { return false; }

	Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);
	if(SUCCEEDED(m_swapchain->GetBuffer(0u, IID_PPV_ARGS(&pBackBuffer))))
	{
		assert(SUCCEEDED(m_device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_rtv)));
	    
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
		if(!m_vdMenuOpen) { m_vdTempResult.Clear(); }

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

	assert(SUCCEEDED(m_swapchain->Present(0u, 0u)));
}

int WINAPI WinMain(_In_ const HINSTANCE hInstance, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int)
{
    const WNDCLASSEX wc{static_cast<UINT>(sizeof WNDCLASSEX), 0u, E4BViewer::WndProc,
        0, 0, hInstance, LoadIcon(nullptr, IDI_APPLICATION), 
        LoadCursor(nullptr, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), 
        nullptr, E4BViewer::m_windowName.data(), LoadIcon(nullptr, IDI_APPLICATION)};

    if (!RegisterClassEx(&wc)) { MessageBoxA(nullptr, "Window Registration Failed!", "Error!", 
        MB_ICONEXCLAMATION | MB_OK); return false; }

    E4BViewer::m_currentWindowSizeX = 1280.f;
    E4BViewer::m_currentWindowSizeY = 720.f;
    
    RECT wr{0,0,static_cast<LONG>(E4BViewer::m_currentWindowSizeX),static_cast<LONG>(E4BViewer::m_currentWindowSizeY)};
    if(!AdjustWindowRectEx(&wr, WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME, FALSE, 0)) { return false; }
    
    E4BViewer::m_hwnd = CreateWindowEx(0, wc.lpszClassName, E4BViewer::m_windowName.data(), (WS_VISIBLE | (WS_OVERLAPPEDWINDOW ^ WS_THICKFRAME)),
        CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top, nullptr, nullptr, wc.hInstance, nullptr);

    if (!E4BViewer::m_hwnd) { MessageBoxA(nullptr, "Window Creation Failed!", "Error", 
        MB_ICONEXCLAMATION | MB_OK); return false; }
    
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
				assert(SUCCEEDED(m_swapchain->ResizeBuffers(0u, width, height, DXGI_FORMAT_UNKNOWN, 0u)));

				Microsoft::WRL::ComPtr<ID3D11Texture2D> pBackBuffer(nullptr);
				if (SUCCEEDED(m_swapchain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer))))
				{
					assert(SUCCEEDED(m_device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_rtv)));
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

                    const auto popupVDMenuName("##filePopupVDMenu" + std::to_string(index));
                    bool tempOpenVDMenu(false);
                    if (ImGui::BeginPopup(popupRCMenuName.c_str()))
                    {
                        ImGui::BeginDisabled(!strCI(file.extension().string(), ".E4B"));

                        if (ImGui::Button("View Details"))
                        {
                            ImGui::CloseCurrentPopup();

                            BinaryReader reader;
                            if (reader.readFile(file))
                            {
                                if (E4BFunctions::ProcessE4BFile(reader, m_vdTempResult))
                                {
                                    tempOpenVDMenu = true;
                                    m_vdMenuOpen = true;
                                }
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

                    if (tempOpenVDMenu)
                    {
                        ImGui::OpenPopup(popupVDMenuName.c_str());
                        ImGui::SetNextWindowSize(ImVec2(windowSize.x * 0.85f, windowSize.y * 0.85f));
                    }

                    DisplayBankInfoWindow(popupVDMenuName.c_str());

                    ++index;
                }
            }

            ImGui::EndListBox();
        }

        ImGui::SameLine();

        ImGui::Text("Banks In Progress: %d", static_cast<int32_t>(m_threadPool.GetNumTasks()));

        if (ImGui::Button("Add Files"))
        {
            std::vector<TCHAR> szFile{};
            szFile.resize(MAX_FILES);

            OPENFILENAME ofn{};
            ofn.lStructSize = sizeof ofn;
            ofn.hwndOwner = m_hwnd;
            ofn.lpstrFile = szFile.data();
            ofn.nMaxFile = MAX_PATH * MAX_FILES;
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
                        while (*str != 0u)
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
                                    if (!strCI(path.extension().string(), m_bankFiles[0].extension().string()))
                                    {
                                        continue;
                                    }
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
        
        if (!m_bankFiles.empty())
        {
            const auto firstExt(m_bankFiles[0].extension().string());
            if (strCI(firstExt, ".E4B"))
            {
        #ifdef _DEBUG
                if (ImGui::Button("Verify Cords"))
                {
                    for (const auto& file : m_bankFiles)
                    {
                        if (exists(file))
                        {
                            m_threadPool.queueFunc([&]
                            {
                                BinaryReader reader;
                                if (reader.readFile(file))
                                {
                                    E4Result tempResult;
                                    if (E4BFunctions::ProcessE4BFile(reader, tempResult))
                                    {
                                        E4BFunctions::IsAccountingForCords(tempResult);
                                    }
                                }
                            });
                        }
                    }
                }
        #endif

                if (ImGui::Button("Sequence Check"))
                {
                    for (const auto& file : m_bankFiles)
                    {
                        if (exists(file))
                        {
                            m_threadPool.queueFunc([&]
                            {
                                BinaryReader reader;
                                if (reader.readFile(file))
                                {
                                    E4Result tempResult;
                                    if (E4BFunctions::ProcessE4BFile(reader, tempResult))
                                    {
                                        if (!tempResult.GetSequences().empty())
                                        {
                                            const auto str("'" + file.filename().string() + "' Sequence count: " + std::to_string(tempResult.GetSequences().size()));
                                            Logger::LogMessage(str.c_str());
                                        }
                                    }
                                }
                            });
                        }
                    }
                }

                if (ImGui::Button("Sequence Extract"))
                {
                    BROWSEINFO browseInfo{};
                    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
                    browseInfo.lpszTitle = _T("Select a folder to export to");

                    std::filesystem::path saveFolder;
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

                        saveFolder = std::filesystem::path(path.data());
                    }

                    if (!saveFolder.empty())
                    {
                        for (const auto& file : m_bankFiles)
                        {
                            if (exists(file))
                            {
                                m_threadPool.queueFunc([&, saveFolder]
                                {
                                    BinaryReader reader;
                                    if (reader.readFile(file))
                                    {
                                        E4Result tempResult;
                                        if (E4BFunctions::ProcessE4BFile(reader, tempResult))
                                        {
                                            if (!tempResult.GetSequences().empty())
                                            {
                                                for (const auto& seq : tempResult.GetSequences())
                                                {
                                                    std::filesystem::path saveFolderSeq(saveFolder);
                                                    saveFolderSeq.append(seq.GetName());
                                                    saveFolderSeq.replace_extension(".mid");

                                                    BinaryWriter writer(saveFolderSeq);

                                                    if (!writer.writeType(seq.GetMIDISeqData().data(),
                                                        sizeof(char) * seq.GetMIDISeqData().size()))
                                                    {
                                                        Logger::LogMessage("Failed to extract sequence!");
                                                        break;
                                                    }

                                                    if (!writer.finishWriting())
                                                    {
                                                        Logger::LogMessage("Failed to finish writing sequence!");
                                                    }
                                                }
                                            }
                                        }
                                    }
                                });
                            }
                        }
                    }
                }
            }
        }

        ImGui::BeginDisabled(m_conversionType.empty());

        if (ImGui::Button("Convert"))
        {
            if (!m_bankFiles.empty())
            {
                BROWSEINFO browseInfo{};
                browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_USENEWUI;
                browseInfo.lpszTitle = _T("Select a folder to export to");
                
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

                    m_converterOptions.m_saveFolder = std::filesystem::path(path.data());
                }

                for (const auto& file : m_bankFiles)
                {
                    if (exists(file))
                    {
                        const auto bankName(file.filename().replace_extension("").string());
                        
                        const auto ext(file.extension().string());
                        if (strCI(ext, ".E4B"))
                        {
                            if (strCI(m_conversionType, "SF2"))
                            {
                                m_threadPool.queueFunc([&, file, bankName]
                                {
                                    BinaryReader reader;
                                    if (reader.readFile(file))
                                    {
                                        E4Result tempResult;
                                        if (E4BFunctions::ProcessE4BFile(reader, tempResult))
                                        {
                                            constexpr BankConverter converter;
                                            if (converter.ConvertE4BToSF2(tempResult, bankName, m_converterOptions))
                                            {
                                                OutputDebugStringA("Successfully converted to SF2! \n");
                                            }
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
                                    m_threadPool.queueFunc([&, file, bankName]
                                    {
                                        constexpr BankConverter converter;
                                        if (converter.ConvertSF2ToE4B(file, bankName, m_converterOptions))
                                        {
                                            OutputDebugStringA("Successfully converted to E4B! \n");
                                        }
                                    });
                                }
                            }
                        }
                    }
                }
            }

            m_queueClear = true;
        }

        ImGui::EndDisabled();

        ImGui::EndDisabled();

        if (m_queueClear && m_threadPool.GetNumTasks() == 0)
        {
            m_queueClear = false;
            m_bankFiles.clear();
            m_conversionType.clear();
            m_converterOptions = {};
        }

        ImGui::EndTabItem();
    }
}

void E4BViewer::DisplayBankInfoWindow(const std::string_view& popupName)
{
    if (ImGui::BeginPopupModal(popupName.data(), &m_vdMenuOpen))
    {
        if (ImGui::TreeNode("Presets"))
        {
            int32_t presetIndex(0u);
            for (auto& preset : m_vdTempResult.GetPresets())
            {
                ImGui::PushID(presetIndex);
                if (ImGui::TreeNode(preset.GetName().data()))
                {
                    int32_t voiceIndex(1u);
                    for (const auto& voice : preset.GetVoices())
                    {
                        ImGui::PushID(voiceIndex);
                        
                        if (ImGui::TreeNode(std::string("Voice #" + std::to_string(voiceIndex)).c_str()))
                        {
                            const auto& zoneRange(voice.GetKeyZoneRange());
                            const auto& velRange(voice.GetVelocityRange());
                            ImGui::Text("Original Key: %s", VoiceDefinitions::GetMIDINoteFromKey(voice.GetOriginalKey()).data());
                            ImGui::Text("Zone Range: %s-%s",
                                VoiceDefinitions::GetMIDINoteFromKey(zoneRange.GetLow()).data(),
                                VoiceDefinitions::GetMIDINoteFromKey(zoneRange.GetHigh()).data());
                            ImGui::Text("Velocity Range: %u-%u", velRange.GetLow(), velRange.GetHigh());
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
            for (const auto& sample : m_vdTempResult.GetSamples())
            {
                ImGui::PushID(sampleIndex);
                if (ImGui::TreeNode(sample.GetName().c_str()))
                {
                    ImGui::Text("Sample Rate: %u", sample.GetSampleRate());
                    ImGui::Text("Loop Start: %u", sample.GetLoopStart());
                    ImGui::Text("Loop End: %u", sample.GetLoopEnd());
                    ImGui::Text("Loop: %d", sample.IsLooping() ? 1 : 0);
                    ImGui::Text("Release: %d", sample.IsReleasing() ? 1 : 0);
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
            for (const auto& seq : m_vdTempResult.GetSequences())
            {
                ImGui::PushID(seqIndex);

                if (ImGui::TreeNode(seq.GetName().c_str()))
                {
                    if (ImGui::Button("Extract Sequence"))
                    {
                        auto seqPathTemp(seq.GetName());
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

                            const auto& seqData(seq.GetMIDISeqData());
                            if (writer.writeType(seqData.data(), seqData.size()))
                            {
                                if (!writer.finishWriting())
                                {
                                    Logger::LogMessage("Failed to extract sequence");
                                }
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

void E4BViewer::DisplayOptions()
{
    if(ImGui::BeginTabItem("Options"))
    {
        if(ImGui::BeginTabBar("##optionsBar"))
        {
            if(ImGui::BeginTabItem("General"))
            {
                ImGui::Checkbox("Flip Pan", &m_converterOptions.m_flipPan);
                
                ImGui::Checkbox("Use Converter Specific Data", &m_converterOptions.m_useConverterSpecificData);
                if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                {
                    ImGui::SetTooltip("Uses specific conversion data from E4BViewer, allowing for more accurate data.");
                }
                
                if (ImGui::TreeNode("Default Filter Settings"))
                {
                    if(ImGui::Button("Apply Emax II defaults"))
                    {
                        m_converterOptions.m_filterDefaults = ADSR_Envelope(0.f, 18.822f, 0.f, 0.f, 0.435f);
                    }

                    if(ImGui::Button("Apply Emulator IV defaults"))
                    {
                        m_converterOptions.m_filterDefaults = ADSR_Envelope(0.f, 0.f, 0.f, 0.f, 0.661f);
                    }

                    ImGui::SliderFloat("Attack (s)", &m_converterOptions.m_filterDefaults.m_attackTime, ADSR_EnvelopeStatics::MIN_ADSR, ADSR_EnvelopeStatics::MAX_ATTACK_TIME);
                    ImGui::SliderFloat("Decay (s)", &m_converterOptions.m_filterDefaults.m_decayTime, ADSR_EnvelopeStatics::MIN_ADSR, ADSR_EnvelopeStatics::MAX_DECAY_TIME);
                    ImGui::SliderFloat("Hold (s)", &m_converterOptions.m_filterDefaults.m_holdTime, ADSR_EnvelopeStatics::MIN_ADSR, ADSR_EnvelopeStatics::MAX_HOLD_TIME);
                    ImGui::SliderFloat("Sustain (dB)", &m_converterOptions.m_filterDefaults.m_sustainDB, ADSR_EnvelopeStatics::MIN_ADSR, ADSR_EnvelopeStatics::MAX_SUSTAIN_DB);
                    ImGui::SliderFloat("Release (s)", &m_converterOptions.m_filterDefaults.m_releaseTime, ADSR_EnvelopeStatics::MIN_ADSR, ADSR_EnvelopeStatics::MAX_RELEASE_TIME);
                    
                    ImGui::TreePop();
                }
                
                ImGui::EndTabItem();
            }

            if(ImGui::BeginTabItem("E4B"))
            {
                if constexpr(ENABLE_TEMP_SETTINGS)
                {
                    ImGui::Checkbox("Correct Fine Tune (temp)", &m_converterOptions.m_useTempFineTune);
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
                    {
                        ImGui::SetTooltip("Corrects a fine tune issue occurring when converting Emax II to E4B via the Emulator IV.");
                    }
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
            std::wstring exportPath;
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
