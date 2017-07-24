#include <tuple>

namespace iterators {

using std::get;
using core::numeric_limits;
using std::move;
using std::tuple;
using core::offset;

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
template<typename _Stream> InputStreamIterator<_Stream>::InputStreamIterator (_Stream &r_stream, size_t bufferCapacity) : window(bufferCapacity), stream(&r_stream) {
}

template<typename _Stream> InputStreamIterator<_Stream>::InputStreamIterator (_Stream &r_stream) : InputStreamIterator(r_stream, BUFSIZ) {
}

template<typename _Stream> InputStreamIterator<_Stream>::InputStreamIterator () noexcept :  window(), stream(nullptr) {
}

template<typename _Stream> void InputStreamIterator<_Stream>::ensureBuffer () {
  if (!window.exhausted()) {
    return;
  }

  DPRE(stream);
  auto v = window.get();
  iu8f *b = get<0>(v);
  size_t capacity = get<1>(v);
  DPRE(b);
  DA(capacity != 0);
  size_t size = stream->read(b, capacity);
  DA(size != 0);
  if (size == numeric_limits<size_t>::max()) {
    window.unset();
    return;
  }
  window.reset(size);
}

template<typename _Stream> iu8f InputStreamIterator<_Stream>::operator* () {
  ensureBuffer();
  return *window;
}

template<typename _Stream> InputStreamIterator<_Stream> &InputStreamIterator<_Stream>::operator++ () {
  ensureBuffer();
  ++window;
  return *this;
}

template<typename _Stream> const iu8f *InputStreamIterator<_Stream>::operator++ (int) {
  ensureBuffer();
  return window++;
}

template<typename _Stream> bool InputStreamIterator<_Stream>::operator== (InputStreamIterator<_Stream> &r_r) {
  ensureBuffer();
  r_r.ensureBuffer();

  bool lEnded = window.ended();
  bool rEnded = r_r.window.ended();
  if (lEnded) {
    return rEnded;
  }
  if (rEnded) {
    return false;
  }

  return this == &r_r;
}

template<typename _Stream> bool InputStreamIterator<_Stream>::operator!= (InputStreamIterator<_Stream> &r_r) {
  return !(*this == r_r);
}

template<typename _Stream> bool InputStreamIterator<_Stream>::operator== (const InputStreamEndIterator<_Stream> &r) {
  ensureBuffer();

  bool lEnded = window.ended();
  return lEnded;
}

template<typename _Stream> bool InputStreamIterator<_Stream>::operator!= (const InputStreamEndIterator<_Stream> &r) {
  return !(*this == r);
}

template<typename _Stream> InputStreamEndIterator<_Stream>::InputStreamEndIterator () noexcept : InputStreamIterator<_Stream>() {
}

template<typename _Stream> bool InputStreamEndIterator<_Stream>::operator== (const InputStreamEndIterator<_Stream> &r) const {
  return true;
}

template<typename _Stream> bool InputStreamEndIterator<_Stream>::operator== (InputStreamIterator<_Stream> &r_r) const {
  return r_r == *this;
}

template<typename _Stream> bool InputStreamEndIterator<_Stream>::operator!= (InputStreamIterator<_Stream> &r_r) const {
  return r_r != *this;
}

template<typename _Stream> OutputStreamIterator<_Stream>::OutputStreamIterator (_Stream &r_stream, size_t bufferCapacity) : window(bufferCapacity), stream(&r_stream) {
  DI(indirectionsMinusIncrements = 0;)
}

template<typename _Stream> OutputStreamIterator<_Stream>::OutputStreamIterator (_Stream &r_stream) : OutputStreamIterator(r_stream, BUFSIZ) {
}

template<typename _Stream> void OutputStreamIterator<_Stream>::ensureBuffer () {
  if (!window.exhausted()) {
    return;
  }

  flushToStream();
}

template<typename _Stream> void OutputStreamIterator<_Stream>::flushToStream () {
  DPRE(stream);
  auto v = window.get();
  iu8f *b = get<0>(v);
  size_t capacity = get<1>(v);
  DPRE(b);
  size_t advancement = window.advancement();
  if (advancement != 0) {
    stream->write(b, advancement);
  }
  window.reset(capacity);
}

template<typename _Stream> iu8f &OutputStreamIterator<_Stream>::operator* () {
  ensureBuffer();
  DPRE(indirectionsMinusIncrements++ <= 0);
  return *window++;
}

template<typename _Stream> OutputStreamIterator<_Stream> &OutputStreamIterator<_Stream>::operator++ () {
  DPRE(indirectionsMinusIncrements-- >= 0);
  return *this;
}

template<typename _Stream> OutputStreamIterator<_Stream> &OutputStreamIterator<_Stream>::operator++ (int) {
  return ++*this;
}

template<
  typename _Class, typename _Reference, typename _Iterator
> RevaluedIterator<_Class, _Reference, _Iterator>::RevaluedIterator (_Iterator &&i) : i(move(i)) {
}

template<
  typename _Class, typename _Reference, typename _Iterator
> RevaluedIterator<_Class, _Reference, _Iterator>::RevaluedIterator () : i() {
}

template<
  typename _Class, typename _Reference, typename _Iterator
> typename RevaluedIterator<_Class, _Reference, _Iterator>::Pointer RevaluedIterator<_Class, _Reference, _Iterator>::operator-> () noexcept(noexcept(*(std::declval<_Class>()))) {
  return &(*(*static_cast<_Class *>(this)));
}

template<
  typename _Class, typename _Reference, typename _Iterator
> _Class &RevaluedIterator<_Class, _Reference, _Iterator>::operator++ () noexcept(noexcept(++i)) {
  ++i;
  return *static_cast<_Class *>(this);
}

template<
  typename _Class, typename _Reference, typename _Iterator
> _Class RevaluedIterator<_Class, _Reference, _Iterator>::operator++ (int) noexcept(std::is_nothrow_copy_constructible<_Class>::value && noexcept(++i)) {
  _Class o(*static_cast<_Class *>(this));
  ++i;
  return o;
}

template<
  typename _Class, typename _Reference, typename _Iterator
> _Class &RevaluedIterator<_Class, _Reference, _Iterator>::operator-- () noexcept(noexcept(--i)) {
  --i;
  return *static_cast<_Class *>(this);
}

template<
  typename _Class, typename _Reference, typename _Iterator
> _Class RevaluedIterator<_Class, _Reference, _Iterator>::operator-- (int) noexcept(std::is_nothrow_copy_constructible<_Class>::value && noexcept(--i)) {
  _Class o(*static_cast<_Class *>(this));
  --i;
  return o;
}

template<
  typename _Class, typename _Reference, typename _Iterator
> _Class &RevaluedIterator<_Class, _Reference, _Iterator>::operator+= (const Distance &r) noexcept(noexcept(i += r)) {
  i += r;
  return *static_cast<_Class *>(this);
}

template<
  typename _Class, typename _Reference, typename _Iterator
> _Class &RevaluedIterator<_Class, _Reference, _Iterator>::operator-= (const Distance &r) noexcept(noexcept(i -= r)) {
  i -= r;
  return *static_cast<_Class *>(this);
}

template<
  typename _Class, typename _Reference, typename _Iterator
> _Reference RevaluedIterator<_Class, _Reference, _Iterator>::operator[] (const Distance &r) noexcept(noexcept(*(std::declval<_Class>() + r))) {
  return *(*static_cast<const _Class *>(this) + r);
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}
