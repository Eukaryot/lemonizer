/*
*	Lemonizer -- turns 68K code into lemon script
*	Copyright (C) 2021 by Eukaryot
*
*	Published under the GNU GPLv3 open source software license, see license.txt
*	or https://www.gnu.org/licenses/gpl-3.0.en.html
*/

#pragma once

#include <rmxbase.h>


namespace assembly
{
	struct AssemblyCode;
	struct Parameter;

	class Formatter
	{
	public:
		struct Token
		{
			enum Type
			{
				TEXT,
				JUMP_TARGET,
				TABULATOR
			};

			Type mType = TEXT;
			std::string mText;		// Used for type TEXT and JUMP_TARGET
			uint32 mValue = 0;		// Used for type TABULATOR and JUMP_TARGET (optional there)

			inline Token(const std::string& text) : mText(text) {}
			inline Token(Type type, const std::string& text, uint32 value = 0) : mType(type), mText(text), mValue(value) {}
			inline Token(Type type, uint32 value) : mType(type), mValue(value) {}
		};

		struct Output
		{
			struct Line
			{
				std::vector<Token> mTokens;
			};
			std::vector<Line> mLines;

			inline Output() { newLine(); }
			inline void addToken(Token&& token)								{ mLines.back().mTokens.emplace_back(token); }
			inline void addToken(const std::string& text)					{ addToken(Token(text)); }
			inline void addToken(Token::Type type, uint32 value)			{ addToken(Token(type, value)); }
			inline void addToken(Token::Type type, const std::string& text, uint32 value = 0)	{ addToken(Token(type, text, value)); }
			inline void newLine() { mLines.emplace_back(); }
		};

	public:
		static void formatCode(const AssemblyCode& code, uint32 address, Output& output);
	};
}
