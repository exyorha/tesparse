#ifndef TESPARSE_EXPRESSION_H
#define TESPARSE_EXPRESSION_H

#include <vector>
#include <variant>
#include <string>

namespace tesparse {
	enum class ExpressionOperator {
		Add,
		Subtract,
		Multiply,
		Divide,
		Modulo,
		And,
		Or,
		Xor,
		Not,
		LeftShift,
		RightShift,

		// Used by ExpressionParser only
		LeftParenthesis,
		RightParenthesis
	};

	using ExpressionInteger = int32_t;
	using ExpressionToken = std::variant<ExpressionOperator, ExpressionInteger, std::string>;
	using Expression = std::vector<ExpressionToken>;
}

#endif
