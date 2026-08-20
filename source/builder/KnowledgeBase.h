/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyDefinitions.h"


class KnowledgeBase : public SingleInstance<KnowledgeBase>
{
public:
	struct FunctionInfo
	{
		const uint32 mAddress;
		std::string mName;
		std::vector<std::string> mNotes;
		uint32 mTranslatedToScriptStart = 0xffffffff;
		uint32 mTranslatedToScriptEnd = 0xffffffff;
		std::string mTranslatedScriptFile;
		bool mAutomaticCallReplacement = false;

		inline FunctionInfo(uint32 address) : mAddress(address) {}
	};

	struct DefineKey
	{
		uint32 mAddress = 0;
		uint8 mBytes = 1;
		bool mSigned = false;
		assembly::ExtRegister mBaseRegister = assembly::ExtRegister::NONE;

		DefineKey(uint32 address, uint8 bytes, bool isSigned);
		DefineKey(assembly::Register baseRegister, int32 addressOffset, uint8 bytes, bool isSigned);
		uint64 operator*() const;
	};

	struct DefineInfo
	{
		const DefineKey mKey;
		std::string mName;
		std::vector<std::string> mNotes;

		inline DefineInfo(const DefineKey& key) : mKey(key) {}
	};

public:
	KnowledgeBase();

	void clear();

	FunctionInfo& createFunctionInfo(uint32 address);
	const FunctionInfo* getFunctionInfo(uint32 address) const;

	DefineInfo& createDefineInfo(const DefineKey& key);
	const DefineInfo* getDefineInfo(const DefineKey& key) const;

public:
	std::map<uint32, FunctionInfo> mFunctions;
	std::map<uint64, DefineInfo> mDefines;
};
