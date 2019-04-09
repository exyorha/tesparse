#ifndef TESPARSE_EXPRESSION_PARSER_H
#define TESPARSE_EXPRESSION_PARSER_H

#include <tesparse/Expression.h>

namespace tesparse {
	class ExpressionParser {
	public:
		ExpressionParser();
		~ExpressionParser();

		ExpressionParser(const ExpressionParser &other) = delete;
		ExpressionParser &operator =(const ExpressionParser &other) = delete;

		void parse(const std::string_view &string);

		inline Expression &&expression() { return std::move(m_expression); }

	private:
		static unsigned int operatorPrecedence(ExpressionOperator op);

		Expression m_expression;
		std::vector<ExpressionOperator> m_operatorStack;
	};
}

#endif
