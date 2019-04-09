#include <tesparse/ExpressionParser.h>

#include <regex>
#include <unordered_map>

namespace tesparse {
	ExpressionParser::ExpressionParser() = default;

	ExpressionParser::~ExpressionParser() = default;

	void ExpressionParser::parse(const std::string_view &string) {
		static const std::regex spaceRegex("^\\s+");
		static const std::regex variableNameRegex("^[A-Za-z_][A-Za-z0-9_]*");
		static const std::regex numberRegex("^(?:0x[0-9A-Fa-f]+|0[0-7]?|[0-9]+)");
		static const std::regex operatorRegex("^(?:[+\\-*/%&|^~()]|<<|>>)");
		static std::unordered_map<std::string_view, ExpressionOperator> operatorMap{
			{ "+", ExpressionOperator::Add },
			{ "-", ExpressionOperator::Subtract },
			{ "*", ExpressionOperator::Multiply },
			{ "/", ExpressionOperator::Divide },
			{ "%", ExpressionOperator::Modulo },
			{ "&", ExpressionOperator::And },
			{ "|", ExpressionOperator::Or },
			{ "^", ExpressionOperator::Not },
			{ "(", ExpressionOperator::LeftParenthesis },
			{ ")", ExpressionOperator::RightParenthesis },
			{ "<<", ExpressionOperator::LeftShift },
			{ ">>", ExpressionOperator::RightShift }
		};

		auto pos = string.cbegin();

		while (pos != string.end()) {
			auto sliceStart = pos;

			std::string_view slice(&*pos, string.end() - pos);

			std::match_results<std::string_view::const_iterator> results;

			if (std::regex_search(slice.begin(), slice.end(), results, spaceRegex)) {
				pos += results[0].length();
			}
			else if (std::regex_search(slice.begin(), slice.end(), results, variableNameRegex)) {
				pos += results[0].length();

				m_expression.emplace_back(std::string(sliceStart, pos));
			} else if(std::regex_search(slice.begin(), slice.end(), results, numberRegex)) {
				pos += results[0].length();

				m_expression.emplace_back(std::stoi(std::string(sliceStart, pos), nullptr, 0));
			}
			else if (std::regex_search(slice.begin(), slice.end(), results, operatorRegex)) {
				pos += results[0].length();

				auto it = operatorMap.find(std::string_view(&*sliceStart, (pos - sliceStart)));
				if (it == operatorMap.end())
					throw std::logic_error("operator is not in map");

				auto op = it->second;

				if (op == ExpressionOperator::LeftParenthesis) {
					m_operatorStack.push_back(op);
				}
				else if (op == ExpressionOperator::RightParenthesis) {
					while (!m_operatorStack.empty() && m_operatorStack.back() != ExpressionOperator::LeftParenthesis) {

						m_expression.push_back(m_operatorStack.back());
						m_operatorStack.pop_back();
					}

					if (m_operatorStack.empty())
						throw std::runtime_error("mismatched parentheses");
				}
				else {
					auto prec = operatorPrecedence(op);

					while (!m_operatorStack.empty() &&
						m_operatorStack.back() != ExpressionOperator::LeftParenthesis &&
						operatorPrecedence(m_operatorStack.back()) <= prec) {

						m_expression.push_back(m_operatorStack.back());
						m_operatorStack.pop_back();
					}

					m_operatorStack.push_back(op);
				}

			} else {
				throw std::runtime_error("syntax error: " + std::string(slice));
			}
		}

		while (!m_operatorStack.empty()) {
			if(m_operatorStack.back() == ExpressionOperator::LeftParenthesis)
				throw std::runtime_error("mismatched parentheses");

			m_expression.push_back(m_operatorStack.back());
			m_operatorStack.pop_back();
		}
	}

	unsigned int ExpressionParser::operatorPrecedence(ExpressionOperator op) {
		switch (op) {
		case ExpressionOperator::Add:
		case ExpressionOperator::Subtract:
			return 2;

		case ExpressionOperator::Multiply:
		case ExpressionOperator::Divide:
		case ExpressionOperator::Modulo:
			return 3;

		case ExpressionOperator::And:
			return 8;

		case ExpressionOperator::Or:
			return 10;

		case ExpressionOperator::Xor:
			return 9;

		case ExpressionOperator::Not:
			return 2;

		case ExpressionOperator::LeftShift:
		case ExpressionOperator::RightShift:
			return 5;

		default:
			throw std::logic_error("unexpected operator");
		}
	}
}

