#include "iterators.hpp"

LIB_DEPENDENCIES

namespace iterators {

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
DC();

iu8f *const BufferedWindow::endedMarker = static_cast<iu8f *>(nullptr) + 1;

BufferedWindow::BufferedWindow (size_t capacity_) {
  DPRE(capacity_ != 0);
  b.reset(new iu8f[capacity_]);
  capacity = capacity_;
  i = b.get();
  end = i;
}

BufferedWindow::BufferedWindow () noexcept {
  capacity = 0;
  i = endedMarker;
  end = i + 1;
}

BufferedWindow::BufferedWindow (BufferedWindow &&o) noexcept {
  *this = move(o);
}

BufferedWindow &BufferedWindow::operator= (BufferedWindow &&o) noexcept {
  if (this != &o) {
    auto oI = o.i - o.b.get();
    auto oEnd = o.end - o.b.get();
    b.swap(o.b);
    o.b.release();
    capacity = o.capacity;
    i = b.get() + oI;
    end = b.get() + oEnd;
    o.i = nullptr;
    o.end = nullptr;
  }
  return *this;
}

tuple<iu8f *, size_t> BufferedWindow::get () noexcept {
  return tuple<iu8f *, size_t>(b.get(), capacity);
}

size_t BufferedWindow::advancement () const noexcept {
  return offset(b.get(), i);
}

bool BufferedWindow::exhausted () const noexcept {
  return i == end;
}

void BufferedWindow::reset (size_t size) noexcept {
  DA(size != 0);
  DA(size <= capacity);
  i = b.get();
  end = i + size;
}

void BufferedWindow::unset () noexcept {
  b.release();
  i = endedMarker;
  end = i + 1;
}

bool BufferedWindow::ended () const noexcept {
  return i == endedMarker;
}

iu8f &BufferedWindow::operator* () noexcept {
  DPRE(!ended(), "this must not have ended");
  DPRE(!exhausted(), "this must not be exhausted");
  return *i;
}

void BufferedWindow::operator++ () noexcept {
  DPRE(!ended(), "this must not have ended");
  DPRE(!exhausted(), "this must not be exhausted");
  ++i;
}

iu8f *BufferedWindow::operator++ (int) noexcept {
  DPRE(!ended(), "this must not have ended");
  DPRE(!exhausted(), "this must not be exhausted");
  return i++;
}

/* -----------------------------------------------------------------------------
----------------------------------------------------------------------------- */
}
