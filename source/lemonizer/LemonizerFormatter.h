/*
*	Lemonizer -- Turns 68K code into lemonscript
*	Copyright (C) 2017-2026 by Eukaryot
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
			bool mPushPopAddressOnCall = false;
			bool mPreCallWithReturnAddress = false;
			bool mOutputFarJumpsAsCallReturn = true;
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
		static void formatLemonTokenTreeNode(const lemon::StatementToken& token, Output& output, bool useCompactNotation = false, bool preferDecimalConstants = false);
		static void formatJumpCall(JumpCallFormatting jumpCallFormatting, const assembly::Parameter& paramDest, uint32 returnAddress, Output& output);
		static void formatJumpCall(JumpCallFormatting jumpCallFormatting, uint32 destAddress, uint32 returnAddress, Output& output);
	};

}
