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
		virtual bool movePosition(unsigned int /*absolutePositionIndex*/) { return false; }
		virtual bool setRangePosition(unsigned int /*beginIndex*/, unsigned int /*outOfRangeIndex*/) { return false; }
		virtual unsigned int getPosition() const { return 0; }
	};
}

#endif
