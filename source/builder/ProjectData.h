/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include <rmxbase.h>


class ProjectData : public SingleInstance<ProjectData>
{
public:
	std::string mName;
	std::wstring mOutputPath;
	std::wstring mRomFile;
	std::vector<uint32> mSeedAddresses;
	std::multimap<uint32, uint32> mJumpTargets;

public:
	bool loadFromProjectFile(const std::wstring& filename);
};
