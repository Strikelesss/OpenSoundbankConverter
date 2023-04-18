#pragma once
#include <string>
#include <sf2cute.hpp>

struct BankWriteOptions;
struct BankRealtimeControl;
struct Soundbank;

struct SF2Writer final
{
    [[nodiscard]] bool WriteData(const Soundbank& soundbank, const BankWriteOptions& options) const;
    
protected:
    void WriteModOrGen(sf2cute::SFInstrumentZone& instrumentZone, const BankRealtimeControl& rtControl, const BankWriteOptions& options) const;
    [[nodiscard]] std::string ConvertNameToSFName(const std::string_view& name) const;
    
    bool m_beganWriting = false;
};