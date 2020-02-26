// synexts.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "json.hpp"
#include <stdio.h>
#include <unordered_map>

using json = nlohmann::json;

// ----------------------------------------------------------------------------

namespace
{

// ----------------------------------------------------------------------------

const CHAR g_szArgumentTokens[] = " \t";

// ----------------------------------------------------------------------------

}

// ----------------------------------------------------------------------------

extern "C"
HRESULT
CALLBACK
help(
    __in IDebugClient4 *pDebugClient,
    __in PCSTR args
)
{
    UNREFERENCED_PARAMETER(args);

    // query required debug engine interface
    ATL::CComQIPtr<IDebugControl> pDebugControl{pDebugClient};

    // print help messafe
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "Synthetic symbols extension\n");
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "    addsymbol   <NAME> <OFFSET> <SIZE>\n");
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "                create artificial symbol in any existing module\n");
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "    addmodule   <NAME> <PATH> <BASE> <SIZE>\n");
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "                create artificial loaded modules in engine's module list\n");
    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "\n");

    return S_OK;
}

// ----------------------------------------------------------------------------

extern "C"
HRESULT
CALLBACK
addsymbol(
    __in IDebugClient4 *pDebugClient,
    __in PCSTR args
)
{
    // query required debug engine interfaces
    ATL::CComQIPtr<IDebugControl> pDebugControl{pDebugClient};
    ATL::CComQIPtr<IDebugSymbols3> pDebugSymbols{pDebugClient};

    // parse command arguments
    const ATL::CStringA csArguments{args};

    int nTokenize{};
    const auto csName{csArguments.Tokenize(g_szArgumentTokens, nTokenize)};
    if (csName.IsEmpty())
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing first parameter (NAME)\n");
        return E_INVALIDARG;
    }

    const auto csOffset{csArguments.Tokenize(g_szArgumentTokens, nTokenize)};
    if (csOffset.IsEmpty())
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing second parameter (OFFSET)\n");
        return E_INVALIDARG;
    }

    DEBUG_VALUE OffsetValue{};
    auto hResult = 
        pDebugControl->Evaluate(
            csOffset,
            DEBUG_VALUE_INVALID,
            &OffsetValue,
            nullptr);
    if (S_OK != hResult)
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Second parameter (OFFSET) cannot be converted to number (0x%x)\n", hResult);
        return hResult;
    }

    const auto csSize{csArguments.Tokenize(g_szArgumentTokens, nTokenize)};
    if (csSize.IsEmpty())
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing third parameter (SIZE)\n");
        return E_INVALIDARG;
    }

    DEBUG_VALUE SizeValue{};
    hResult =
        pDebugControl->Evaluate(
            csSize,
            DEBUG_VALUE_INVALID,
            &SizeValue,
            nullptr);
    if (S_OK != hResult)
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Third parameter (SIZE) cannot be converted to number (0x%x)\n", hResult);
        return hResult;
    }

    // try to create artificial symbol
    hResult =
        pDebugSymbols->AddSyntheticSymbol(
            OffsetValue.I64,
            SizeValue.I32,
            csName,
            DEBUG_ADDSYNTHSYM_DEFAULT,
            nullptr);
    if (S_OK != hResult)
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Cannot add synthetic symbol (0x%x)\n", hResult);
        return hResult;
    }

    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "Synthetic symbol successfully added\n");
    return S_OK;
}

// ----------------------------------------------------------------------------

extern "C"
HRESULT
CALLBACK
addmodule(
    __in IDebugClient4 *pDebugClient,
    __in PCSTR args
)
{
    // query required debug engine interfaces
    ATL::CComQIPtr<IDebugControl> pDebugControl{pDebugClient};
    ATL::CComQIPtr<IDebugSymbols3> pDebugSymbols{pDebugClient};

    // parse command arguments
    const ATL::CStringA csArguments{args};

    int nTokenize{};
    const auto csName{csArguments.Tokenize(g_szArgumentTokens, nTokenize)};
    if (csName.IsEmpty())
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing 1st parameter (NAME)\n");
        return E_INVALIDARG;
    }

    const auto csPath{csArguments.Tokenize(g_szArgumentTokens, nTokenize)};
    if (csPath.IsEmpty())
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing 2th parameter (PATH)\n");
        return E_INVALIDARG;
    }

    const auto csBase{csArguments.Tokenize(g_szArgumentTokens, nTokenize)};
    if (csBase.IsEmpty())
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing 3th parameter (BASE)\n");
        return E_INVALIDARG;
    }

    DEBUG_VALUE BaseValue{};
    auto hResult = 
        pDebugControl->Evaluate(
            csBase,
            DEBUG_VALUE_INVALID,
            &BaseValue,
            nullptr);
    if (S_OK != hResult)
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "BASE (3th parameter) cannot be converted to number (0x%x)\n", hResult);
        return hResult;
    }

    const auto csSize{csArguments.Tokenize(g_szArgumentTokens, nTokenize)};
    if (csSize.IsEmpty())
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing 4th parameter (SIZE)\n");
        return E_INVALIDARG;
    }

    DEBUG_VALUE SizeValue{};
    hResult =
        pDebugControl->Evaluate(
            csSize,
            DEBUG_VALUE_INVALID,
            &SizeValue,
            nullptr);
    if (S_OK != hResult)
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "SIZE (4th) cannot be converted to number (0x%x)\n", hResult);
        return hResult;
    }

    // try to create artificial loaded module
    hResult =
        pDebugSymbols->AddSyntheticModule(
            BaseValue.I64,
            SizeValue.I32,
            csPath,
            csName,
            DEBUG_ADDSYNTHMOD_DEFAULT);
    if (S_OK != hResult)
    {
        pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Cannot add synthetic module (0x%x)\n", hResult);
        return hResult;
    }

    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "Synthetic module successfully added\n");
    return S_OK;
}

// ----------------------------------------------------------------------------

static bool GetFileNameFromPath(const char* filepath, char* outName, size_t outlen)
{
	const size_t namelen = strlen(filepath);
	const char* outStr = filepath;

	if (namelen > 2)
	{
		for (size_t i = namelen - 2; i > 0; --i)
		{
			if (filepath[i] == '\\' || filepath[i] == '/')
			{
				outStr = &filepath[i + 1];
				strcpy_s(outName, outlen, outStr);
				return true;
			}
		}
	}

	return false;
}

extern "C"
HRESULT
CALLBACK
loadjsondd(
	__in IDebugClient4 *pDebugClient,
	__in PCSTR args
)
{
	// query required debug engine interfaces
	ATL::CComQIPtr<IDebugControl> pDebugControl{ pDebugClient };
	ATL::CComQIPtr<IDebugSymbols3> pDebugSymbols{ pDebugClient };

	// parse command arguments
	const ATL::CStringA csArguments{ args };

	int nTokenize{};
	const ATL::CString csJsonFile{ csArguments.Tokenize(g_szArgumentTokens, nTokenize) };
	if (csJsonFile.IsEmpty())
	{
		pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Missing 1st parameter (JSON FILE)\n");
		return E_INVALIDARG;
	}

	char* jsonData = nullptr;
	FILE* fp = nullptr;
	_wfopen_s(&fp, (LPCTSTR)csJsonFile, L"r");
	if(fp)
	{
		fseek(fp, 0, SEEK_END); // seek to end of file
		size_t fileSize = ftell(fp); // get current file pointer
		fseek(fp, 0, SEEK_SET); // seek back to beginning of file
		
		jsonData = new char[fileSize + 1];
		fread(jsonData, fileSize, 1, fp);
		jsonData[fileSize] = 0;

		fclose(fp);
	}
	else
	{
		pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Could not load JSON file %s\n", (LPCTSTR)csJsonFile);
		return E_INVALIDARG;
	}

    int symCount = 0;
	auto debugData = json::parse(jsonData, nullptr, false);

    struct ModInfo
    {
        ULONG64 baseOffset = 0;
    };

    std::unordered_map<std::string, ModInfo> moduleMap;

    // Setup mapping of all modules so we can get base addresses depending on the symbol's relevant module
    HRESULT modNameRes = S_OK;
    ULONG numLoadedMods;
    ULONG numUnloadedMods;
    pDebugSymbols->GetNumberModules(&numLoadedMods, &numUnloadedMods);

    for(ULONG i = 0; i < numLoadedMods; ++i)
    {
        const ULONG nameBuffLen = 1024;
        CHAR modNameBuff[nameBuffLen] = { 0 };
        CHAR imageNameBuff[nameBuffLen] = { 0 };
        CHAR loadedImageNameBuff[nameBuffLen] = { 0 };
        ULONG modNameSize, imgNameSize, loadedImgNameSize;

        modNameRes = pDebugSymbols->GetModuleNames(i, 0, imageNameBuff, nameBuffLen, &imgNameSize, modNameBuff, nameBuffLen, &modNameSize, loadedImageNameBuff, nameBuffLen, &loadedImgNameSize);
    
        ULONG64 modBase = 0;
        char imageFileName[nameBuffLen] = { 0 };
		bool gotFileName = GetFileNameFromPath(imageNameBuff, imageFileName, nameBuffLen);

        // convert image file name to lowercase as x64dbg-ida export does
        for (int j = 0; j < strlen(imageFileName); ++j)
        {
            imageFileName[j] = (char)tolower(imageFileName[j]);
        }

        if (modNameRes == S_OK && gotFileName)
        {
            pDebugSymbols->GetModuleByIndex(i, &modBase);
            moduleMap[imageFileName] = { modBase };
        }

        pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "Trying to track module %s base=%l64x (0x%x)\n", imageFileName, modBase, modNameRes);
    }

	// loop through all json labels to add symbols
	for (const auto& label : debugData["labels"])
	{

		std::string offsetHex = label["address"].get<std::string>();
		std::string symName = label["text"].get<std::string>();

		DEBUG_VALUE OffsetValue{}; // TODO: add module base to offset..? not sure if necessary
		auto hResult =
			pDebugControl->Evaluate(
				offsetHex.c_str(),
				DEBUG_VALUE_INVALID,
				&OffsetValue,
				nullptr);
		if (S_OK != hResult)
		{
			pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Failed to read in address value from JSON label (0x%x)\n", hResult);
			return hResult;
		}

		DEBUG_VALUE SizeValue{}; //TODO - support 32 bit sizes..?
		hResult =
			pDebugControl->Evaluate(
				"8",   
				DEBUG_VALUE_INVALID,
				&SizeValue,
				nullptr);
		if (S_OK != hResult)
		{
			pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Failed to read in size from JSON label (0x%x)\n", hResult);
			return hResult;
		}

		// try to create artificial symbol
        ULONG64 symOffset = OffsetValue.I64;

        // if possible try to lookup module in map and add base to get correct offset
        auto modIt = moduleMap.find(label["module"]);
        if (modIt != moduleMap.end())
        {
            symOffset += modIt->second.baseOffset;
        }

		hResult =
			pDebugSymbols->AddSyntheticSymbol(
				symOffset,
				SizeValue.I32,
				symName.c_str(),
				DEBUG_ADDSYNTHSYM_DEFAULT,
				nullptr);

        if (hResult == S_OK)
        {
            symCount++;
        }
        else
		{
			pDebugControl->Output(DEBUG_OUTPUT_ERROR, "Could not add symbol %s at %l64x from json label (0x%x)\n", symName.c_str(), symOffset, hResult);
		}
	}

    pDebugControl->Output(DEBUG_OUTPUT_NORMAL, "Added %d symbols from %s.\n", symCount, (LPCTSTR)csJsonFile);

	return S_OK;
}
