/** @file */
/* -----------------------------------------------------------------------------
   Iterators Library
   © Geoff Crossland 2017
----------------------------------------------------------------------------- */
#ifndef ITERATORS_ALREADYINCLUDED
#define ITERATORS_ALREADYINCLUDED

#include <core.hpp>

namespace iterators {

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
extern DC();

// TODO split into Window and BufferedWindow
class BufferedWindow {
  prv static constexpr iu8f *const ENDED = static_cast<iu8f *>(nullptr) + 1;
  prv iu8f *i;
  prv iu8f *end;
  prv std::unique_ptr<iu8f []> b;
  prv size_t capacity;

  pub explicit BufferedWindow (size_t capacity);
  pub BufferedWindow () noexcept;
  BufferedWindow (const BufferedWindow &) = delete;
  BufferedWindow &operator= (const BufferedWindow &) = delete;
  pub BufferedWindow (BufferedWindow &&o) noexcept;
  pub BufferedWindow &operator= (BufferedWindow &&o) noexcept;

  pub std::tuple<iu8f *, size_t> get () noexcept;
  pub size_t advancement () const noexcept;
  pub bool exhausted () const noexcept;
  pub void reset (size_t size) noexcept;
  pub void unset () noexcept;
  pub bool ended () const noexcept;
  pub iu8f &operator* () noexcept;
  pub void operator++ () noexcept;
  pub iu8f *operator++ (int) noexcept;
  // TODO ptr access to the remainder of the buffer (+ ability to 'consume/fill in' part/all thereof)
  // TODO wrapper to read x bytes (view on the buffer if possible, by copying to a new block if necessary)
};

/**
  @interface InputStream

  Reads octets from a sequence of unbounded size.


  @fn size_t read (iu8f *b, size_t size)

  Moves at most {@p size} octets from the head of the stream into {@p b}. This
  will block for an arbitrarily long time only when the stream is exhausted.

  @return {@c numeric_limits<size_t>::max()}, if the stream is empty (in which
  case no octets were moved), or the number of octets moved, which is at least
  {@c 1} (unless {@c size == 0}) and no greater than {@p size}.
*/

template<typename _Stream> class InputStreamEndIterator;

/**
  Wraps an {@c InputStream} in an InputIterator.

  Instances may buffer octets from the underlying {@c InputStream} an arbitrary
  distance ahead of the current iterator position; if you stop reading elements
  from an instance before you reach the end position, there may be a gap
  between the last element read via the instance and the first octet available
  from the underlying {@c InputStream}.
*/
// TODO split into InputStreamBufferWindow and InputWindowIterator
// TODO SimpleInputStreamIterator (move-only, no post inc, expected that inc can cause blocking)
template<typename _Stream> class InputStreamIterator : public std::iterator<std::input_iterator_tag, iu8f, void, iu8f *, iu8f> {
  friend class InputStreamEndIterator<_Stream>;

  prv BufferedWindow window;
  prv _Stream *stream;

  pub InputStreamIterator (_Stream &r_stream, size_t bufferCapacity);
  pub explicit InputStreamIterator (_Stream &r_stream);
  pub InputStreamIterator () noexcept;

  prv void ensureBuffer ();
  pub iu8f operator* ();
  pub InputStreamIterator<_Stream> &operator++ ();
  pub const iu8f *operator++ (int);
  pub bool operator== (InputStreamIterator<_Stream> &r_r);
  pub bool operator!= (InputStreamIterator<_Stream> &r_r);
  pub bool operator== (const InputStreamEndIterator<_Stream> &r);
  pub bool operator!= (const InputStreamEndIterator<_Stream> &r);
};

/**
  End iterator type for InputStreamIterators.
*/
template<typename _Stream> class InputStreamEndIterator : public InputStreamIterator<_Stream> { // TODO drop inheritance
  pub InputStreamEndIterator () noexcept;

  pub bool operator== (const InputStreamEndIterator<_Stream> &r) const;
  pub bool operator== (InputStreamIterator<_Stream> &r_r) const;
  pub bool operator!= (InputStreamIterator<_Stream> &r_r) const;
};

/**
  @interface OutputStream

  Writes octets to form a sequence of unbounded size.


  @fn void write (const iu8f *b, size_t size)

  Copies {@p size} octets from {@p b} to the tail of the stream.
*/

/**
  Wraps an {@c OutputStream} in an OutputIterator.

  Elements written to an instance may be buffered; flushToStream() should be
  called whenever it is necessary that everything written so far should have been
  passed to the underlying {@c OutputStream}.
*/
// TODO split into OutputStreamBufferWindow and OutputWindowIterator
template<typename _Stream> class OutputStreamIterator : public std::iterator<std::output_iterator_tag, iu8f, void> {
  prv BufferedWindow window;
  prv _Stream *stream;
  DI(prv is8f indirectionsMinusIncrements;)

  pub OutputStreamIterator (_Stream &r_stream, size_t bufferCapacity);
  pub explicit OutputStreamIterator (_Stream &r_stream);
  // TODO flush on destruction?

  prv void ensureBuffer ();
  /**
    Ensures that all elements written have been passed to the underlying stream via
    {@c OutputStream::write()}.
  */
  pub void flushToStream ();
  pub iu8f &operator* ();
  pub OutputStreamIterator<_Stream> &operator++ ();
  pub OutputStreamIterator<_Stream> &operator++ (int);
};

/**
  Wraps an iterator so that each element is a subobject of the underlying element
  or (if this is exactly an InputIterator) a value derived from the underlying
  element.

  @tparam _Reference the type returned from indirection, of the form (given value
  type {@c T}) {@c const T &} (in which case instances are non-mutable iterators),
  {@c T &} (in which case instances are mutable iterators) or {@c T} (in which
  case instances are exactly InputIterators).
  @tparam _Iterator the underlying iterator type
*/
// TODO support RevaluedIterators over OutputIterators that handle indirection via a proxy object - just handle super-ForwardIterators seperately?
// TODO remove template arguments that aren't material to the interface (i.e. _Iterator)
template<typename _Class, typename _Reference, typename _Iterator> class RevaluedIterator : public std::iterator<
  typename _Iterator::iterator_category,
  typename std::remove_const<typename std::remove_reference<_Reference>::type>::type,
  typename _Iterator::difference_type,
  typename std::remove_reference<_Reference>::type *,
  _Reference
> {
  pub typedef typename std::remove_reference<_Reference>::type *Pointer;
  prv typedef typename _Iterator::difference_type Distance;
  prt _Iterator i;

  prt RevaluedIterator (_Iterator &&i);
  prt RevaluedIterator ();

  pub bool operator== (const _Class &r) const noexcept_auto_return(
    i == r.i
  )
  pub bool operator< (const _Class &r) const noexcept_auto_return(
    i < r.i
  )
  // TODO CRTP utils
  pub _Reference operator* () noexcept(noexcept(std::declval<_Class>().operator_ind_())) {
    return static_cast<_Class *>(this)->operator_ind_();
  }
  pub Pointer operator-> () noexcept(noexcept(*(std::declval<_Class>()))); // DODGY broken return type when _Reference is not a reference
  pub _Class &operator++ () noexcept(noexcept(++i));
  pub _Class operator++ (int) noexcept(std::is_nothrow_copy_constructible<_Class>::value && noexcept(++i));
  pub _Class &operator-- () noexcept(noexcept(--i));
  pub _Class operator-- (int) noexcept(std::is_nothrow_copy_constructible<_Class>::value && noexcept(--i));
  pub _Class &operator+= (const Distance &r) noexcept(noexcept(i += r));
  friend _Class operator+ (_Class l, const Distance &r) noexcept_auto_return(
    l.i += r,
    l
  )
  friend _Class operator+ (const Distance &l, const _Class &r) noexcept_auto_return(
    r + l
  )
  pub _Class &operator-= (const Distance &r) noexcept(noexcept(i -= r));
  friend _Class operator- (_Class l, const Distance &r) noexcept_auto_return(
    l.i -= r,
    l
  )
  friend Distance operator- (const _Class &l, const _Class &r) noexcept_auto_return(
    l.i - r.i
  )
  pub _Reference operator[] (const Distance &r) noexcept(noexcept(*(std::declval<_Class>() + r)));
};

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}

#include "iterators.ipp"
#endif
