#include "header.hpp"
#include <cstring>
#include <vector>

using core::check;
using core::string;
using std::move;
using core::numeric_limits;
using iterators::InputStreamIterator;
using core::InputIterator;
using iterators::InputStreamEndIterator;
using iterators::OutputStreamIterator;
using core::OutputIterator;
using iterators::RevaluedIterator;
using std::vector;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
DC();

int main (int argc, char *argv[]) {
  /*std::shared_ptr<core::debug::Stream> errs(new core::debug::Stream());
  DOPEN(, errs);
  iterators::DOPEN(, errs);*/

  testInputStreamIterator();
  testOutputStreamIterator();
  testRevaluedIterator();

  return 0;
}

struct TestInputStream {
  string<iu8f> data;
  size_t i;

  TestInputStream (string<iu8f> &&data) : data(move(data)), i(0) {
  }

  size_t read (iu8f *b, size_t size) {
    DA(i <= data.size());
    if (i == data.size()) {
      return numeric_limits<size_t>::max();
    }

    size_t end = i + size;
    if (end > data.size()) {
      end = data.size();
      size = end - i;
    }
    memcpy(b, data.data() + i, size);
    i = end;
    return size;
  }
};

struct TestBiInputStream {
  TestInputStream s0;
  TestInputStream s1;
  bool switchedOver;
  DI(size_t s0Count;)

  TestBiInputStream (TestInputStream &&s0, TestInputStream &&s1) : s0(move(s0)), s1(move(s1)), switchedOver(false) {
    DI(s0Count = 0;)
  }

  size_t read (iu8f *b, size_t size) {
    size_t r = s0.read(b, size);
    DA(!switchedOver || r == numeric_limits<size_t>::max());
    if (r != numeric_limits<size_t>::max()) {
      DA((r == 0) == (size == 0));
      DI(s0Count += r;)
      return r;
    }

    DA(s0Count == s0.data.size());
    switchedOver = true;
    return s1.read(b, size);
  }
};

const char *strs[] = {"", "a", "to", "for", "with", "under", "within", "between", "around and about", "this is a longer string"};
const size_t bufferCapacities[] = {1U, 2U, 3U, 5U, 8U, 11U, 4096U};

void testInputStreamIterator () {
  check(InputIterator<InputStreamIterator<TestInputStream>, iu8f>);

  for (const char *str : strs) {
    auto data = reinterpret_cast<const iu8f *>(str);
    for (size_t bufferCapacity : bufferCapacities) {
      TestInputStream stream(data);
      InputStreamIterator<TestInputStream> i(stream, bufferCapacity);

      string<iu8f> r = useInputStream(i, numeric_limits<size_t>::max(), true);
      check(stream.data, r);
    }
  }

  enum SwitchOverTrigger {
    indirection,
    eq
  };
  for (SwitchOverTrigger switchOverTrigger : {indirection, eq}) {
    const char *strs[] = {"this ", "is a string in two parts"};
    auto datas = reinterpret_cast<const iu8f **>(strs);
    for (size_t bufferCapacity : bufferCapacities) {
      TestBiInputStream stream(TestInputStream{datas[0]}, TestInputStream{datas[1]});
      InputStreamIterator<TestBiInputStream> i(stream, bufferCapacity);

      check(!stream.switchedOver);
      string<iu8f> r = useInputStream(i, strlen(strs[0]), false);
      check(!stream.switchedOver);
      check(string<iu8f>(reinterpret_cast<const iu8f *>(strs[0])), r);

      check(!stream.switchedOver);
      switch (switchOverTrigger) {
        case indirection:
          check(*datas[1], *i);
          break;
        case eq:
          check(false, i == InputStreamEndIterator<TestBiInputStream>());
          break;
      }
      check(stream.switchedOver);

      r += useInputStream(i, numeric_limits<size_t>::max(), true);
      check(stream.s0.data + stream.s1.data, r);
    }
  }
}

template<typename _Stream> string<iu8f> useInputStream (InputStreamIterator<_Stream> &r_i, size_t count, bool doFinalEqCheck) {
  static iu c = 0;

  string<iu8f> data;

  const InputStreamEndIterator<_Stream> end0;
  InputStreamIterator<_Stream> end1;

  while (count != 0 && r_i != end0) {
    checkEq(false, r_i, end0, end1);

    iu8f v = 0;
    if (c % 2 == 0) {
      v = *r_i;
      checkEq(false, r_i, end0, end1);
    }

    switch (c % 3) {
      case 0:
        data.push_back(*r_i);
        checkEq(false, r_i, end0, end1);
        ++r_i;
        break;
      case 1:
        data.push_back(*r_i);
        checkEq(false, r_i, end0, end1);
        r_i++;
        break;
      case 2:
        data.push_back(*r_i++);
        break;
    }

    if (c % 2 == 0) {
      check(data.back(), v);
    }

    --count;
    ++c;
  }

  if (doFinalEqCheck) {
    checkEq(!(r_i != end0), r_i, end0, end1);
  }

  return data;
}

template<typename _Stream> void checkEq (bool eq, InputStreamIterator<_Stream> &r_i, const InputStreamEndIterator<_Stream> &end0, InputStreamIterator<_Stream> &r_end1) {
  check(true, r_i == r_i);
  check(eq, r_i == end0);
  check(eq, r_i == r_end1);
  check(eq, end0 == r_i);
  check(true, end0 == end0);
  check(true, end0 == r_end1);
  check(eq, r_end1 == r_i);
  check(true, r_end1 == end0);
  check(true, r_end1 == r_end1);

  check(false, r_i != r_i);
  check(!eq, r_i != end0);
  check(!eq, r_i != r_end1);
  check(!eq, end0 != r_i);
  check(false, end0 != end0);
  check(false, end0 != r_end1);
  check(!eq, r_end1 != r_i);
  check(false, r_end1 != end0);
  check(false, r_end1 != r_end1);
}

struct TestOutputStream {
  string<iu8f> data;

  void write (const iu8f *b, size_t size) {
    data.append(b, b + size);
  }
};

void testOutputStreamIterator () {
  check(OutputIterator<OutputStreamIterator<TestOutputStream>, iu8f>);

  for (const char *str : strs) {
    auto data = reinterpret_cast<const iu8f *>(str);
    for (size_t bufferCapacity : bufferCapacities) {
      TestOutputStream stream;
      OutputStreamIterator<TestOutputStream> i(stream, bufferCapacity);

      useOutputStream(i, data);
      check(strlen(str) == 0 || stream.data.size() < strlen(str));
      i.flushToStream();
      check(stream.data, data);
    }
  }
}

template<typename _Stream> void useOutputStream (OutputStreamIterator<_Stream> &r_i, string<iu8f> data) {
  static iu c = 0;

  for (iu8f b : data) {
    if (c % 7 == 0) {
      r_i.flushToStream();
    }

    switch (c % 3) {
      case 0:
        *r_i = b;
        ++r_i;
        break;
      case 1:
        *r_i = b;
        r_i++;
        break;
      case 2:
        *r_i++ = b;
        break;
    }

    ++c;
  }
}

// DODGY since indirection returns a value (not a ref), should only be an InputIterator
struct CapitalisingIterator : public RevaluedIterator<CapitalisingIterator, char, string<char>::const_iterator> {
  CapitalisingIterator (string<char>::const_iterator &&i) : RevaluedIterator(move(i)) {
  }

  CapitalisingIterator () : RevaluedIterator() {
  }

  char operator_ind_ () noexcept {
    return *i + ('A' - 'a');
  }
};

struct Datum {
  bool dummy;
  char letter;
};

struct LetterIterator : public RevaluedIterator<LetterIterator, char &, vector<Datum>::iterator> {
  LetterIterator (vector<Datum>::iterator &&i) : RevaluedIterator(move(i)) {
  }

  LetterIterator () : RevaluedIterator() {
  }

  char &operator_ind_ () noexcept {
    return i->letter;
  }
};

void testRevaluedIterator () {
  string<char> data0 = "abcdefghijklmnop";
  {
    CapitalisingIterator begin(data0.cbegin());
    CapitalisingIterator end(data0.cend());

    useRevaluedRandomAccessIterator(begin, end, "ABCDEFGHIJKLMNOP");
  }

  vector<Datum> data1 = {
    {false, 'a'},
    {false, 'b'},
    {false, 'c'},
    {false, 'd'},
    {false, 'e'},
    {false, 'f'},
    {false, 'g'},
    {false, 'h'},
    {false, 'i'},
    {false, 'j'},
    {false, 'k'},
    {false, 'l'},
    {false, 'm'},
    {false, 'n'},
    {false, 'o'},
    {false, 'p'}
  };
  {
    LetterIterator begin(data1.begin());
    LetterIterator end(data1.end());

    useRevaluedRandomAccessIterator(begin, end, "abcdefghijklmnop");

    LetterIterator i = begin;
    *i = 'A';
    i += 2;
    *(++i) = 'D';
    i += 3;
    *(i++) = 'G';
    *(i += 2) = 'J';
    *(i + 3) = 'M';
    *(6 + i) = 'P';
    useRevaluedRandomAccessIterator(begin, end, "AbcDefGhiJklMnoP");

    i = end;
    *(--i) = 'p';
    i -= 3;
    *(i--) = 'm';
    *(i -= 2) = 'j';
    *(i - 3) = 'g';
    useRevaluedRandomAccessIterator(begin, end, "AbcDefghijklmnop");
  }

  // TODO revalue an InputStreamIterator
}

template<typename _Iterator> void useRevaluedRandomAccessIterator (_Iterator begin, _Iterator end, const char *expectedData) {
  _Iterator i = begin;

  auto v = *i;
  check(expectedData[0], v);

  auto &i0 = ++i;
  v = *i;
  check(expectedData[1], v);
  check(&i, &i0);

  auto i1 = i++;
  v = *i;
  check(expectedData[2], v);
  check(expectedData[1], *i1);

  v = *i++;
  check(expectedData[2], v);
  v = *i;
  check(expectedData[3], v);

  i += 1;
  v = *i;
  check(expectedData[4], v);

  i += 2;
  v = *i;
  check(expectedData[6], v);

  i = i + 1;
  v = *i;
  check(expectedData[7], v);

  i = i + 3;
  v = *i;
  check(expectedData[10], v);

  i = 4 + i;
  v = *i;
  check(expectedData[14], v);

  check(end, i + 2);
  check(end - 2, i);
  check(2, end - i);

  auto &i2 = --i;
  v = *i;
  check(expectedData[13], v);
  check(&i, &i2);

  auto i3 = i--;
  v = *i;
  check(expectedData[12], v);
  check(expectedData[13], *i3);

  v = *i--;
  check(expectedData[12], v);
  v = *i;
  check(expectedData[11], v);

  i -= 1;
  v = *i;
  check(expectedData[10], v);

  i -= 2;
  v = *i;
  check(expectedData[8], v);

  i = i - 1;
  v = *i;
  check(expectedData[7], v);

  i = i - 3;
  v = *i;
  check(expectedData[4], v);

  check(begin, i - 4);
  check(begin + 4, i);
  check(4, i - begin);

  i -= -1;
  v = *i;
  check(expectedData[5], v);

  i -= -2;
  v = *i;
  check(expectedData[7], v);

  i = i - -1;
  v = *i;
  check(expectedData[8], v);

  i = i - -3;
  v = *i;
  check(expectedData[11], v);

  i += -1;
  v = *i;
  check(expectedData[10], v);

  i += -2;
  v = *i;
  check(expectedData[8], v);

  i = i + -1;
  v = *i;
  check(expectedData[7], v);

  i = i + -3;
  v = *i;
  check(expectedData[4], v);

  i = -4 + i;
  v = *i;
  check(expectedData[0], v);

  size_t dataEnd = strlen(expectedData);
  check(dataEnd, offset(begin, end));
  for (size_t dataI = 0; dataI != dataEnd; ++dataI) {
    check(expectedData[dataI], begin[static_cast<typename _Iterator::difference_type>(dataI)]);
  }
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
