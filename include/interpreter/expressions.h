#ifndef CFG_EXPRESSIONS_H
#define CFG_EXPRESSIONS_H

#include <interpreter/token_type.h>
#include <cfg/cfg.h>
#include <ostream>
#include <string>
#include <memory>
#include <list>

namespace cfg
{
namespace expressions
{
	class Context
	{
	public:
		Context(bool allowInterpretationWithQuotes)
				:mAllowInterpretationWithQuotes(allowInterpretationWithQuotes) {}
		bool mAllowInterpretationWithQuotes;
	};

	enum class ExpressionType
	{
		EMPTY,
		PARSE_ERROR,
		ASSIGN,
		CALL,
		CONDITIONAL,
		VALUE,
		OPERATOR,
		POSTFIX,
		PREFIX,
	};

	/**
	 * Interface for all expression AST node classes.
	 */
	class Expression {
	public:
		Expression(ExpressionType expType, const char* className)
				:mExpressionType(expType), mClassName(className) {}
		virtual ~Expression() = default;
		ExpressionType getExpressionType() const { return mExpressionType; }
		const char* getClassName() const { return mClassName; }
		/**
		 * Pretty-print the expression to a string.
		 */
		virtual void print(std::ostream& builder) const = 0;
		virtual void printEx(std::ostream& builder) const = 0;
		virtual bool interpret(Context& /*context*/, cfg::Value& /*result*/,
				std::ostream& errMsg) const {
			errMsg << getClassName() << " not implemented" << std::endl;
			return false;
		}
	private:
		ExpressionType mExpressionType;
		const char* mClassName;
	};

	class EmptyExpression: public Expression {
	public:
		EmptyExpression() :Expression(ExpressionType::EMPTY, __func__) {}
		virtual void print(std::ostream& /*builder*/) const override {
		}
		virtual void printEx(std::ostream& builder) const override {
			builder << "[empty]";
		}
	};

	/**
	 * Used for parsing errors
	 */
	class ParseErrorExpression: public Expression {
	public:
		ParseErrorExpression(const std::string& errorMessage)
				:Expression(ExpressionType::PARSE_ERROR, __func__),
				mErrorMessage(errorMessage) {}

		std::string getErrorMessage() const { return mErrorMessage; }

		virtual void print(std::ostream& builder) const override {
			builder << "ERROR: '" << mErrorMessage << "'";
		}
		virtual void printEx(std::ostream& builder) const override {
			builder << "ERROR: '" << mErrorMessage << "'";
		}

	private:
		const std::string mErrorMessage;
	};

	/**
	 * A function call like "a(b, c, d)".
	 */
	class CallExpression: public Expression {
	public:
		CallExpression(std::unique_ptr<Expression> function, std::list<std::unique_ptr<Expression>>& args)
				:Expression(ExpressionType::CALL, __func__),
				mFunction(std::move(function)),
				mArgs(std::move(args))
		{
		}

		virtual void print(std::ostream& builder) const override {
			mFunction->print(builder);
			builder << "(";
			unsigned int i = 0;
			for (const std::unique_ptr<Expression>& arg : mArgs) {
				arg->print(builder);
				if (i + 1 < mArgs.size()) builder << ", ";
				++i;
			}
			builder << ")";
		}
		virtual void printEx(std::ostream& builder) const override {
			builder << "{";
			mFunction->printEx(builder);
			builder << "(";
			unsigned int i = 0;
			for (const std::unique_ptr<Expression>& arg : mArgs) {
				arg->printEx(builder);
				if (i + 1 < mArgs.size()) builder << ", ";
				++i;
			}
			builder << ")";
			builder << "}";
		}
		virtual bool interpret(Context& context, cfg::Value& result,
				std::ostream& errMsg) const override;

	private:
		const std::unique_ptr<Expression>            mFunction;
		const std::list<std::unique_ptr<Expression>> mArgs;
	};

	/**
	 * A ternary conditional expression like "a ? b : c".
	 */
	class ConditionalExpression: public Expression {
	public:
		ConditionalExpression(std::unique_ptr<Expression> condition,
				std::unique_ptr<Expression> thenArm,
				std::unique_ptr<Expression> elseArm)
				:Expression(ExpressionType::CONDITIONAL, __func__),
				mCondition(std::move(condition)),
				mThenArm(std::move(thenArm)),
				mElseArm(std::move(elseArm))
		{
		}

		virtual void print(std::ostream& builder) const override {
			builder << "(";
			mCondition->print(builder);
			builder << " ? ";
			mThenArm->print(builder);
			builder << " : ";
			mElseArm->print(builder);
			builder << ")";
		}
		virtual void printEx(std::ostream& builder) const override {
			builder << "(";
			mCondition->printEx(builder);
			builder << " ? ";
			mThenArm->printEx(builder);
			builder << " : ";
			mElseArm->printEx(builder);
			builder << ")";
		}

	private:
		const std::unique_ptr<Expression> mCondition;
		const std::unique_ptr<Expression> mThenArm;
		const std::unique_ptr<Expression> mElseArm;
	};

	/**
	 * A simple variable name expression like "abc".
	 */
	class ValueExpression: public Expression {
	public:
		ValueExpression(const cfg::Value& value)
				:Expression(ExpressionType::VALUE, __func__),
				mValue(value) {
		}

		const cfg::Value& getValue() const { return mValue; }

		virtual void print(std::ostream& builder) const override;
		virtual void printEx(std::ostream& builder) const override;

		virtual bool interpret(Context& /*context*/, cfg::Value& result,
				std::ostream& /*errMsg*/) const override {
			result = mValue;
			return true;
		}

	private:
		const cfg::Value mValue;
	};

	/**
	 * A binary arithmetic expression like "a + b" or "c ^ d".
	 */
	class OperatorExpression: public Expression {
	public:
		OperatorExpression(std::unique_ptr<Expression> left,
				TokenType operatorType, std::unique_ptr<Expression> right)
				:Expression(ExpressionType::OPERATOR, __func__),
				mLeft(std::move(left)),
				mOperator(operatorType),
				mRight(std::move(right))
		{
		}

		virtual void print(std::ostream& builder) const override {
			builder << "(";
			mLeft->print(builder);
			builder << " " << tokentype::punctuator(mOperator) << " ";
			mRight->print(builder);
			builder << ")";
		}
		virtual void printEx(std::ostream& builder) const override {
			builder << "(";
			mLeft->printEx(builder);
			builder << " " << tokentype::punctuator(mOperator) << " ";
			mRight->printEx(builder);
			builder << ")";
		}
		virtual bool interpret(Context& context, cfg::Value& result,
				std::ostream& errMsg) const override;

	private:
		const std::unique_ptr<Expression> mLeft;
		const TokenType                   mOperator;
		const std::unique_ptr<Expression> mRight;
	};

	/**
	 * A postfix unary arithmetic expression like "a!".
	 */
	class PostfixExpression: public Expression {
	public:
		PostfixExpression(std::unique_ptr<Expression> left, TokenType operatorType)
				:Expression(ExpressionType::POSTFIX, __func__),
				mLeft(std::move(left)),
				mOperator(operatorType)
		{
		}

		virtual void print(std::ostream& builder) const override {
			builder << "(";
			mLeft->print(builder);
			builder << tokentype::punctuator(mOperator) << ")";
		}
		virtual void printEx(std::ostream& builder) const override {
			builder << "(";
			mLeft->printEx(builder);
			builder << tokentype::punctuator(mOperator) << ")";
		}

	private:
		const std::unique_ptr<Expression> mLeft;
		const TokenType                   mOperator;
	};

	/**
	 * A prefix unary arithmetic expression like "!a" or "-b".
	 */
	class PrefixExpression: public Expression {
	public:
		PrefixExpression(TokenType operatorType, std::unique_ptr<Expression> right)
				:Expression(ExpressionType::PREFIX, __func__),
				mOperator(operatorType),
				mRight(std::move(right))
		{
		}

		virtual void print(std::ostream& builder) const override {
			builder << "(" << tokentype::punctuator(mOperator);
			mRight->print(builder);
			builder << ")";
		}
		virtual void printEx(std::ostream& builder) const override {
			builder << "(" << tokentype::punctuator(mOperator);
			mRight->printEx(builder);
			builder << ")";
		}
		virtual bool interpret(Context& context, cfg::Value& result,
				std::ostream& errMsg) const override;

	private:
		const TokenType mOperator;
		const std::unique_ptr<Expression> mRight;
	};
}
}

#endif
