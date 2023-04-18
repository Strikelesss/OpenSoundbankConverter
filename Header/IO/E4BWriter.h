#pragma once
#include "Header/Data/Soundbank.h"

struct BinaryWriter;

struct E4BWriter final
{
    void BeginWriting(BinaryWriter& writer);
    void EndWriting(BinaryWriter& writer);

    std::vector<BankPreset> m_presets;
    std::vector<BankSample> m_samples;
    
protected:
    [[nodiscard]] std::string ConvertNameToEmuName(const std::string_view& name) const;
    void WriteTOC(BinaryWriter& writer);
    
    uint32_t m_totalFORMSize = 0u;
    uint32_t m_totalIndexingSize = 0u;
    bool m_beganWriting = false;
};