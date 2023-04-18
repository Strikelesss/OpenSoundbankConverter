#include "Header/BankConverter.h"
#include "Header/BankWriteOptions.h"
#include "Header/IO/BinaryWriter.h"
#include "Header/Logger.h"
#include "Header/Data/Soundbank.h"
#include "Header/IO/E4BWriter.h"
#include "Header/IO/SF2Writer.h"

bool BankConverter::CreateSF2(const Soundbank& bank, const BankWriteOptions& options)
{
    if (bank.IsValid())
    {
        constexpr SF2Writer sf2Writer;
        return sf2Writer.WriteData(bank, options);
    }
    
    Logger::LogMessage("Bank was invalid!");
    return false;
}

bool BankConverter::CreateE4B(const Soundbank& bank, const BankWriteOptions& options)
{
    if(bank.IsValid())
    {
        std::filesystem::path filePath(options.m_saveFolder);
        if (!filePath.empty() && std::filesystem::exists(filePath))
        {
            filePath = filePath.append(bank.m_bankName + ".E4B");
        }
        else
        {
            Logger::LogMessage("Path was empty or did not exist.");
            return false;
        }
        
        BinaryWriter writer(filePath);
        
        E4BWriter e4Writer;
        e4Writer.BeginWriting(writer);

        e4Writer.m_presets = bank.m_presets;
        e4Writer.m_samples = bank.m_samples;

        e4Writer.EndWriting(writer);
        return true;
    }
    
    Logger::LogMessage("Bank was invalid!");
    return false;
}