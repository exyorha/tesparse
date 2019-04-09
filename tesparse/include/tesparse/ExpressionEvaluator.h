#ifndef TESPARSE_EXPRESSION_EVALUATOR_H
#define TESPARSE_EXPRESSION_EVALUATOR_H

#include <tesparse/Expression.h>

namespace tesparse {
	struct TESStruct;

	class ExpressionEvaluator {
	public:
		ExpressionEvaluator();
		~ExpressionEvaluator();

		ExpressionEvaluator(const ExpressionEvaluator &other) = delete;
		ExpressionEvaluator &operator =(const ExpressionEvaluator &other) = delete;

		ExpressionInteger evaluate(const Expression &expression, const TESStruct &context);

	private:
		void execute(ExpressionOperator op, const TESStruct &context);
		void execute(ExpressionInteger val, const TESStruct &context);
		void execute(const std::string &variable, const TESStruct &context);

		ExpressionInteger popStack();

		std::vector<uint32_t> m_stack;
	};
}

#endif
