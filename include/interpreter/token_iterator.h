#ifndef CFG_TOKEN_ITERATOR_H
#define CFG_TOKEN_ITERATOR_H

#include <interpreter/token.h>

namespace cfg
{
	class TokenIterator
	{
	public:
		virtual ~TokenIterator() = default;
		virtual bool hasNext() const = 0;
		virtual Token next() = 0;
	};
}

#endif
