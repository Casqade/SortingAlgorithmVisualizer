#pragma once

#include <windows.h>

#include <cstdint>


struct TimePoint
{
  int64_t sec {};
  int64_t nsec {};


  TimePoint() = default;
  TimePoint( int64_t seconds, int64_t nanoseconds );

  explicit TimePoint( double seconds );
  explicit TimePoint( const LARGE_INTEGER& time );

  operator LARGE_INTEGER() const;

  explicit operator float() const;
  explicit operator double() const;

  static TimePoint Now();


  bool operator == ( const TimePoint& ) const;
  bool operator != ( const TimePoint& ) const;
  bool operator < ( const TimePoint& ) const;
  bool operator > ( const TimePoint& ) const;
  bool operator <= ( const TimePoint& ) const;
  bool operator >= ( const TimePoint& ) const;

  TimePoint operator + ( const TimePoint& ) const;
  TimePoint operator - ( const TimePoint& ) const;

  TimePoint& operator += ( const TimePoint& );
  TimePoint& operator -= ( const TimePoint& );
};

