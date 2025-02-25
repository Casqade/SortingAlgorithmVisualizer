#pragma once

#include <windows.h>


#define AtomicLoadSeqCst(x) \
  InterlockedCompareExchange(&x, 0, 0)

#define AtomicLoadAcquire(x) \
  InterlockedCompareExchangeAcquire(&x, 0, 0)

#define AtomicLoadRelaxed(x) \
  InterlockedCompareExchangeNoFence(&x, 0, 0)

#define AtomicLoad(x) AtomicLoadSeqCst(x)


#define AtomicStoreSeqCst(x, value) \
  InterlockedExchange(&x, value)

#define AtomicStoreAcquire(x, value) \
  InterlockedExchangeAcquire(&x, value)

#define AtomicStoreRelaxed(x, value) \
  InterlockedExchangeNoFence(&x, value)

#define AtomicStore(x) AtomicStoreSeqCst(x)


///
/// NOTE: Returns pre-incremented value
///

#define AtomicIncrementSeqCst(x) \
  InterlockedIncrement(&x)

#define AtomicIncrementAcquire(x) \
  InterlockedIncrementAcquire(&x)

#define AtomicIncrementRelease(x) \
  InterlockedIncrementRelease(&x)

#define AtomicIncrementRelaxed(x) \
  InterlockedIncrementNoFence(&x)

#define AtomicIncrement(x) AtomicIncrementSeqCst(x)


///
/// NOTE: Returns pre-decremented value
///

#define AtomicDecrementSeqCst(x) \
  InterlockedDecrement(&x)

#define AtomicDecrementAcquire(x) \
  InterlockedDecrementAcquire(&x)

#define AtomicDecrementRelease(x) \
  InterlockedDecrementRelease(&x)

#define AtomicDecrementRelaxed(x) \
  InterlockedDecrementNoFence(&x)

#define AtomicDecrement(x) AtomicDecrementSeqCst(x)


#define AtomicCompareExchangeSeqCst(x, expected, desired) \
  InterlockedCompareExchange(&x, desired, expected)

#define AtomicCompareExchangeAcquire(x, expected, desired) \
  InterlockedCompareExchangeAcquire(&x, desired, expected)

#define AtomicCompareExchangeRelease(x, expected, desired) \
  InterlockedCompareExchangeRelease(&x, desired, expected)

#define AtomicCompareExchangeRelaxed(x, expected, desired) \
  InterlockedCompareExchangeNoFence(&x, desired, expected)

#define AtomicCompareExchange(x, expected, desired) \
  AtomicCompareExchangeSeqCst(x, expected, desired)


#define AtomicPointerLoadSeqCst(x) \
  InterlockedCompareExchangePointer( \
    reinterpret_cast <PVOID*> (&x), nullptr, nullptr )

#define AtomicPointerLoadAcquire(x) \
  InterlockedCompareExchangePointerAcquire( \
    reinterpret_cast <PVOID*> (&x), nullptr, nullptr )

#define AtomicPointerLoadRelaxed(x) \
  InterlockedCompareExchangePointerNoFence( \
    reinterpret_cast <PVOID*> (&x), nullptr, nullptr )

#define AtomicPointerLoad(x) AtomicPointerLoadSeqCst(x)


#define AtomicPointerStoreSeqCst(x, value) \
  InterlockedExchangePointer( \
    reinterpret_cast <PVOID*> (&x), value )

#define AtomicPointerStoreAcquire(x, value) \
  InterlockedExchangePointerAcquire( \
    reinterpret_cast <PVOID*> (&x), value )

#define AtomicPointerStoreNoFence(x, value) \
  InterlockedExchangePointerNoFence( \
    reinterpret_cast <PVOID*> (&x), value )

#define AtomicPointerStore(x, value) \
  AtomicPointerStoreSeqCst(x, value)


#define AtomicPointerExchangeSeqCst(x, value) \
  InterlockedExchangePointer( \
    reinterpret_cast <PVOID*> (&x), value )

#define AtomicPointerExchangeAcquire(x, value) \
  InterlockedExchangePointerAcquire( \
    reinterpret_cast <PVOID*> (&x), value )

#define AtomicPointerExchangeRelaxed(x, value) \
  InterlockedExchangePointerNoFence( \
    reinterpret_cast <PVOID*> (&x), value )

#define AtomicPointerExchange(x, value) \
  AtomicPointerExchangeSeqCst(x, value)


#define AtomicPointerCompareExchangeSeqCst(x, expected, desired) \
  InterlockedCompareExchangePointer( \
    reinterpret_cast <PVOID*> (&x), \
    desired, expected )

#define AtomicPointerCompareExchangeAcquire(x, expected, desired) \
  InterlockedCompareExchangePointerAcquire( \
    reinterpret_cast <PVOID*> (&x), \
    desired, expected )

#define AtomicPointerCompareExchangeRelease(x, expected, desired) \
  InterlockedCompareExchangePointerRelease( \
    reinterpret_cast <PVOID*> (&x), \
    desired, expected )

#define AtomicPointerCompareExchangeRelaxed(x, expected, desired) \
  InterlockedCompareExchangePointerNoFence( \
    reinterpret_cast <PVOID*> (&x), \
    desired, expected )

#define AtomicPointerCompareExchange(x, expected, desired) \
  AtomicPointerCompareExchangeSeqCst(x, expected, desired)
