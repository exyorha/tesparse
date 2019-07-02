#include <tesparse/ExpressionEvaluator.h>
#include <tesparse/TESValue.h>

#include <stdexcept>

namespace tesparse {
	ExpressionEvaluator::ExpressionEvaluator() = default;

	ExpressionEvaluator::~ExpressionEvaluator() = default;

	ExpressionInteger ExpressionEvaluator::evaluate(const Expression &expression, const TESStruct &context) {
		for (const auto &op : expression) {
			std::visit([this, &context](const auto &arg) {
				execute(arg, context);
			}, op);
		}

		if (m_stack.size() != 1) {
			throw std::runtime_error("unexpected stack depth at the end of expression");
		}

		return popStack();
	}

	void ExpressionEvaluator::execute(ExpressionOperator op, const TESStruct &context) {
		if (op == ExpressionOperator::Not) {
			auto val = popStack();

			m_stack.push_back(~val);
		}
		else {
			auto left = popStack();
			auto right = popStack();

			ExpressionInteger result;

			switch (op) {
			case ExpressionOperator::Add:
				result = left + right;
				break;

			case ExpressionOperator::Subtract:
				result = left - right;
				break;

			case ExpressionOperator::Multiply:
				result = left * right;
				break;

			case ExpressionOperator::Divide:
				result = left / right;
				break;

			case ExpressionOperator::Modulo:
				result = left % right;
				break;

			case ExpressionOperator::And:
				result = left & right;
				break;

			case ExpressionOperator::Or:
				result = left | right;
				break;

			case ExpressionOperator::Xor:
				result = left ^ right;
				break;

			case ExpressionOperator::LeftShift:
				result = left << right;
				break;

			case ExpressionOperator::RightShift:
				result = left >> right;
				break;

			default:
				throw std::logic_error("unsupported operator");
			}

			m_stack.push_back(result);
		}
	}

	ExpressionInteger ExpressionEvaluator::popStack() {
		if (m_stack.empty())
			throw std::runtime_error("stack underflow");

		auto val = m_stack.back();
		m_stack.pop_back();

		return val;
	}

	void ExpressionEvaluator::execute(ExpressionInteger val, const TESStruct &context) {
		(void)context;

		m_stack.push_back(val);
	}

	void ExpressionEvaluator::execute(const std::string &variable, const TESStruct &context) {
		auto it = context.fields.find(variable);
		if (it == context.fields.end()) {
			throw std::runtime_error("variable is not in context: " + variable);
		}

		auto uval = std::get_if<TESUInt>(&it->second);
		if (uval) {
			if (*uval > std::numeric_limits<ExpressionInteger>::max()) {
				throw std::runtime_error("variable value is not representable in expression");
			}

			m_stack.push_back(static_cast<ExpressionInteger>(*uval));
		}
		else {
			auto ival = std::get_if<TESInt>(&it->second);

			if (ival) {
				if (*ival > std::numeric_limits<ExpressionInteger>::max() || *ival < std::numeric_limits<ExpressionInteger>::min()) {
					throw std::runtime_error("variable value is not representable in expression");
				}

				m_stack.push_back(static_cast<ExpressionInteger>(*ival));
			}
			else {
				throw std::runtime_error("variable value is not representable in expression");
			}
		}
	}
}
