#pragma once

#include <lemon/compiler/TokenTypes.h>


namespace lemon
{

	#define DEFINE_LEMON_CUSTOM_TOKEN_TYPE(_class_, _baseClass_) \
		DEFINE_GENERIC_MANAGER_ELEMENT_TYPE(Token, _baseClass_, _class_, assignType(#_class_, true))


	class ConstantTokenExt : public ConstantToken
	{
	public:
		DEFINE_LEMON_CUSTOM_TOKEN_TYPE(ConstantTokenExt, ConstantToken)

	public:
		bool mOutputAsDecimal = false;
		bool mOutputWithDataTypeSize = false;
		bool mOutputAsAddress = false;
	};


	#undef DEFINE_LEMON_CUSTOM_TOKEN_TYPE

}
