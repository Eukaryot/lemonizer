/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "builder/KnowledgeBase.h"


KnowledgeBase::DefineKey::DefineKey(uint32 address, uint8 bytes, bool isSigned) :
	mAddress(address),
	mBytes(bytes),
	mSigned(isSigned),
	mBaseRegister(assembly::ExtRegister::NONE)
{
}

KnowledgeBase::DefineKey::DefineKey(assembly::Register baseRegister, int32 addressOffset, uint8 bytes, bool isSigned) :
	mAddress(addressOffset),
	mBytes(bytes),
	mSigned(isSigned),
	mBaseRegister((assembly::ExtRegister)baseRegister)
{
}

uint64 KnowledgeBase::DefineKey::operator*() const
{
	return mAddress + ((uint64)(mBytes + (mSigned ? 0x08 : 0x00)) << 32) + ((uint64)mBaseRegister << 40);
}


KnowledgeBase::KnowledgeBase()
{
}

void KnowledgeBase::clear()
{
	mFunctions.clear();
	mDefines.clear();
}

KnowledgeBase::FunctionInfo& KnowledgeBase::createFunctionInfo(uint32 address)
{
	RMX_CHECK(mFunctions.count(address) == 0, "Function info already exists for address " + rmx::hexString(address), );

	return mFunctions.emplace(address, address).first->second;
}

const KnowledgeBase::FunctionInfo* KnowledgeBase::getFunctionInfo(uint32 address) const
{
	const auto it = mFunctions.find(address);
	return (it != mFunctions.end()) ? &it->second : nullptr;
}

KnowledgeBase::DefineInfo& KnowledgeBase::createDefineInfo(const DefineKey& key)
{
	//RMX_CHECK(mDefines.count(*key) == 0, "Define info already exists", );		// S3AIR defines e.g. u16[0xfffff606] twice
	return mDefines.emplace(*key, key).first->second;
}

const KnowledgeBase::DefineInfo* KnowledgeBase::getDefineInfo(const DefineKey& key) const
{
	const auto it = mDefines.find(*key);
	return (it != mDefines.end()) ? &it->second : nullptr;
}
