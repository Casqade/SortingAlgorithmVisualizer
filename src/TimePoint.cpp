#include <SortingAlgorithmVisualizer/TimePoint.hpp>

#include <cmath>


const static int64_t NSEC_IN_SEC = 1000000000;


TimePoint
TimePoint::Now()
{
  LARGE_INTEGER currentTime;
  QueryPerformanceCounter( &currentTime );

  return TimePoint(currentTime);
}


TimePoint::TimePoint(
  int64_t seconds,
  int64_t nanoseconds )
  : sec{seconds}
  , nsec{nanoseconds}
{
}

TimePoint::TimePoint(
  double seconds )
  : sec{ static_cast <int64_t> (std::trunc(seconds)) }
  , nsec{ static_cast <int64_t> (seconds * NSEC_IN_SEC) % NSEC_IN_SEC }
{
}

TimePoint::TimePoint(
  const LARGE_INTEGER& time )
{
  static LARGE_INTEGER perfFreq;
  static INIT_ONCE initOnce = INIT_ONCE_STATIC_INIT;

  auto initFrequencyCallback =
  [] ( PINIT_ONCE, PVOID, PVOID* )
  {
    QueryPerformanceFrequency( &perfFreq );
    return TRUE;
  };

  InitOnceExecuteOnce(
    &initOnce,
    initFrequencyCallback,
    NULL, NULL );

//  Note: Windows has 100 ns time stamp resolution
  sec = time.QuadPart / perfFreq.QuadPart;
  nsec = (time.QuadPart % perfFreq.QuadPart) * NSEC_IN_SEC / perfFreq.QuadPart;
}

TimePoint::operator
double () const
{
  return
    sec + nsec / static_cast <double> ( NSEC_IN_SEC );
}

TimePoint::operator
float () const
{
  return
    sec + nsec / static_cast <double> ( NSEC_IN_SEC );
}

bool
TimePoint::operator == (
  const TimePoint& other ) const
{
  return
    this->sec == other.sec &&
    this->nsec == other.nsec;
}

bool
TimePoint::operator != (
  const TimePoint& other ) const
{
  return !( *this == other );
}

bool
TimePoint::operator < (
  const TimePoint& other ) const
{
  if ( this->sec == other.sec )
    return this->nsec < other.nsec;

  return this->sec < other.sec;
}

bool
TimePoint::operator > (
  const TimePoint& other ) const
{
  return other < *this;
}

bool
TimePoint::operator <= (
  const TimePoint& other ) const
{
  return !( *this > other );
}

bool
TimePoint::operator >= (
  const TimePoint& other ) const
{
  return !( *this < other );
}

TimePoint
TimePoint::operator + (
  const TimePoint& other ) const
{
  TimePoint result
  {
    this->sec + other.sec,
    this->nsec + other.nsec,
  };

  while ( result.nsec >= NSEC_IN_SEC )
  {
    ++result.sec;
    result.nsec -= NSEC_IN_SEC;
  }

  return result;
}

TimePoint
TimePoint::operator - (
  const TimePoint& other ) const
{
  TimePoint result
  {
    this->sec - other.sec,
    this->nsec - other.nsec,
  };

  while ( result.nsec < 0 )
  {
    --result.sec;
    result.nsec += NSEC_IN_SEC;
  }

  return result;
}

TimePoint&
TimePoint::operator += (
  const TimePoint& other )
{
  return *this = *this + other;
}

TimePoint&
TimePoint::operator -= (
  const TimePoint& other )
{
  return *this = *this - other;
}
