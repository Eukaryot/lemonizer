/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#include "pch.h"
#include "builder/ProjectData.h"


bool ProjectData::loadFromProjectFile(const std::wstring& filename)
{
	Json::Value root = rmx::JsonHelper::loadFile(filename);
	if (root.isNull())
	{
		RMX_ERROR("Failed to load project JSON file '" << *WString(filename).toString() << "', it's either not present or invalid JSON", );
		return false;
	}

	rmx::JsonHelper rootHelper(root);
	rootHelper.tryReadString("Name", mName);
	if (!rootHelper.tryReadString("RomFile", mRomFile))
	{
		RMX_ERROR("Project JSON file '" << *WString(filename).toString() << "', does not include a valid 'RomFile' entry", );
		return false;
	}

	const Json::Value& seedAddressesJson = root["SeedAddresses"];
	if (seedAddressesJson.isArray())
	{
		for (const auto& it : seedAddressesJson)
		{
			const uint32 address = (uint32)rmx::parseInteger(it.asCString());
			mSeedAddresses.push_back(address);
		}
	}

	const Json::Value& jumpTargetsJson = root["JumpTargets"];
	if (jumpTargetsJson.isObject())
	{
		for (auto it = jumpTargetsJson.begin(); it != jumpTargetsJson.end(); ++it)
		{
			const uint32 sourceAddress = (uint32)rmx::parseInteger(it.key().asString());
			for (const Json::Value& entry : *it)
			{
				RMX_CHECK(entry.isString(), "Jump table JSON entries must contain arrays of strings", continue);
				const uint32 destAddress = (uint32)rmx::parseInteger(entry.asString());
				mJumpTargets.emplace(sourceAddress, destAddress);
			}
		}
	}

	return true;
}
