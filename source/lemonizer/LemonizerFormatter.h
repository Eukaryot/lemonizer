/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include "assembly/AssemblyFormatter.h"


namespace lemon
{
	class StatementToken;
}

namespace lemonizer
{
	class Code;

	class Formatter : public assembly::Formatter
	{
	public:
		struct GlobalSettings
		{
			// None yet
		};
		inline static GlobalSettings mGlobalSettings;

	public:
		static void formatCode(const Code& code, uint32 address, Output& output);

	private:
		enum class JumpCallFormatting
		{
			JUMP,
			CALL,
			CALL_RETURN
		};

	private:
		static bool formatAssemblyCode(const assembly::AssemblyCode& code, Output& output);
		static void formatLemonTokenTreeNode(const lemon::StatementToken& token, Output& output);
		static void formatJumpCall(JumpCallFormatting jumpCallFormatting, const assembly::Parameter& paramDest, Output& output);
		static void formatJumpCall(JumpCallFormatting jumpCallFormatting, uint32 destAddress, Output& output);
	};
}
