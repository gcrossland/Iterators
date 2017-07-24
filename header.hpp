#ifndef HEADER_ALREADYINCLUDED
#define HEADER_ALREADYINCLUDED

#include "libraries/iterators.hpp"

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
int main (int argc, char *argv[]);

void testInputStreamIterator ();
template<typename _Stream> core::string<iu8f> useInputStream (iterators::InputStreamIterator<_Stream> &r_i, size_t count, bool doFinalEqCheck);
template<typename _Stream> void checkEq (bool eq, iterators::InputStreamIterator<_Stream> &r_i, const iterators::InputStreamEndIterator<_Stream> &end0, iterators::InputStreamIterator<_Stream> &r_end1);
void testOutputStreamIterator ();
template<typename _Stream> void useOutputStream (iterators::OutputStreamIterator<_Stream> &r_i, core::string<iu8f> data);
void testRevaluedIterator ();
template<typename _Iterator> void useRevaluedRandomAccessIterator (_Iterator begin, _Iterator end, const char *expectedData);

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
#endif
