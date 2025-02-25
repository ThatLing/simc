// ==========================================================================
// Dedmonwakeen's Raid DPS/TPS Simulator.
// Send questions to natehieter@gmail.com
// ==========================================================================

// ==========================================================================
// Collection of generic programming code
// ==========================================================================

#ifndef SC_GENERIC_HPP
#define SC_GENERIC_HPP

#include "config.hpp"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <numeric>
#include <functional>
#include <type_traits>

// iterable enumeration templates ===========================================

/*
 * Enumeration types in C++ implicitly convert to int but not from int (e.g.,
 * "for ( attribute_e i = ATTR_STRENGTH; i < ATTR_MAX; ++i )" won't compile).
 * This is why so much of the code uses int when it really means an enum type.
 * Providing the kind of operations we want to use for enums lets us tighten up
 * our use of the type system and avoid accidentally passing some other thing
 * that converts to int when we really mean an enumeration type.
 *
 * The template functions tell the compiler it can perform prefix and postfix
 * operators ++ and -- on any type by converting it to int and back. The magic
 * with std::enable_if<> restricts those operations to types T for which the
 * type trait "is_iterable_enum<T>" is true. This trait gives us a way to
 * selectively control the functionality for a specific type T by specializing
 * is_iterable_enum<T> as std::true_type or std::false_type.
 */

// All enumerations are iterable by default.
template <typename T>
struct is_iterable_enum : public std::is_enum<T>
{
};

template <typename T>
inline std::enable_if_t<is_iterable_enum<T>::value, T&> operator--( T& s )
{
  return s = static_cast<T>( static_cast<std::underlying_type_t<T>>(s) - 1 );
}

template <typename T>
inline std::enable_if_t<is_iterable_enum<T>::value, T> operator--( T& s, int )
{
  T tmp = s;
  --s;
  return tmp;
}

template <typename T>
inline std::enable_if_t<is_iterable_enum<T>::value, T&> operator++( T& s )
{
  return s = static_cast<T>( static_cast<std::underlying_type_t<T>>(s) + 1 );
}

template <typename T>
inline std::enable_if_t<is_iterable_enum<T>::value, T> operator++( T& s, int )
{
  T tmp = s;
  ++s;
  return tmp;
}

// Generic programming tools ================================================

/**
 * @brief helper class to make a class non-copyable
 *
 * To make your class A non-copyable, define it as such:
 * class A : private noncopyable {}
 */
class noncopyable
{
protected:
  noncopyable() = default;
  noncopyable( noncopyable&& ) = default;
  noncopyable& operator=( noncopyable&& ) = default;

  noncopyable( const noncopyable& ) = delete; // NOLINT(modernize-use-equals-delete)
  noncopyable& operator=( const noncopyable& ) = delete; // NOLINT(modernize-use-equals-delete)
};

/**
 * @brief helper class to make a class non-moveable
 *
 * To make your class A non-moveable, define it as such:
 * class A : private nonmoveable {}
 */
class nonmoveable : private noncopyable
{
protected:
  nonmoveable()                = default;
  nonmoveable( nonmoveable&& ) = delete; // NOLINT(modernize-use-equals-delete)
  nonmoveable& operator=( nonmoveable&& ) = delete; // NOLINT(modernize-use-equals-delete)
};

// Adapted from (read "stolen") boost::checked_deleter
struct delete_disposer_t
{
  template <typename T>
  void operator()( T* t ) const
  {
    using force_T_to_be_complete = int[ sizeof( T ) ? 1 : -1 ]; // NOLINT(modernize-avoid-c-arrays)
    (void)sizeof( force_T_to_be_complete );
    delete t;
  }
};

namespace meta {

// Poor-mans substitute for C++17 inline constexpr variables
// To fully simulate it should be used like:
//   constexpr const T& name = meta::static_const_t<T>::value;
// TODO C++17: replace all usages with direct inline constexpr
template <typename T>
struct static_const_t {
  static constexpr T value {};
};
template <typename T> constexpr T static_const_t<T>::value;

} // namespace meta

// Generic algorithms =======================================================

template <typename I, typename D>
void dispose( I first, I last, D disposer )
{
  while ( first != last )
    disposer( *first++ );
}

template <typename I>
inline void dispose( I first, I last )
{
  dispose( first, last, delete_disposer_t() );
}

// Machinery for range-based generic algorithms =============================

namespace range
{  // ========================================================
namespace detail {

struct begin_ {
  template <typename R>
  auto operator()( R&& r ) const {
    using std::begin;
    return begin( std::forward<R>( r ) );
  }
  template <typename T>
  auto operator()( const std::pair<T, T>& p ) const {
    return p.first;
  }
};

struct end_ {
  template <typename R>
  auto operator()( R&& r ) const {
    using std::end;
    return end( std::forward<R>( r ) );
  }
  template <typename T>
  auto operator()( const std::pair<T, T>& p ) const {
    return p.second;
  }
};

} // namespace detail

template <typename T>
auto begin( T&& t )
{
  return detail::begin_{}( std::forward<T>( t ) );
}

template <typename T>
auto cbegin( const T& t )
{
  return range::begin( t );
}

template <typename T>
auto end( T&& t )
{
  return detail::end_{}( std::forward<T>( t ) );
}

template <typename T>
auto cend( const T& t )
{
  return range::end( t );
}

template <typename R>
using iterator_t = decltype(range::begin(std::declval<R&>()));

template <typename R>
using value_type_t = typename std::iterator_traits<iterator_t<R>>::value_type;

// Default projection for projection-enabled algorithms =====================

struct identity {
  template <typename T>
  constexpr T&& operator()(T&& value) const noexcept {
    return std::forward<T>(value);
  }
  using is_transparent = std::true_type;
};

// Bare-bones std::invoke ===================================================

template <typename Fn, typename... Args,
        std::enable_if_t<std::is_member_pointer<std::decay_t<Fn>>{}, int> = 0 >
decltype(auto) invoke(Fn&& fn, Args&&... args)
{
  return std::mem_fn(fn)(std::forward<Args>(args)...);
}

template <typename Fn, typename... Args,
         std::enable_if_t<!std::is_member_pointer<std::decay_t<Fn>>{}, int> = 0>
decltype(auto) invoke(Fn&& fn, Args&&... args)
{
  return std::forward<Fn>(fn)(std::forward<Args>(args)...);
}

// Range-based generic algorithms ===========================================

template <typename Range, typename Out>
inline Out copy( const Range& r, Out o )
{
  return std::copy( range::begin( r ), range::end( r ), o );
}

template <typename Range, typename Out, typename UnaryPredicate>
inline Out copy_if( const Range& r, Out o, UnaryPredicate pred )
{
  return std::copy_if( range::begin( r ), range::end( r ), o, pred );
}

template <typename Range, typename D>
inline Range& dispose( Range& r, D disposer )
{
  dispose( range::begin( r ), range::end( r ), disposer );
  return r;
}

template <typename Range>
inline Range& dispose( Range& r )
{
  return dispose( r, delete_disposer_t() );
}

template <typename Range>
inline Range& fill( Range& r, value_type_t<Range> const& t )
{
  std::fill( range::begin( r ), range::end( r ), t );
  return r;
}

template <typename Range, typename T>
inline Range& accumulate( Range& r, const T& init )
{
  return std::accumulate( range::begin( r ), range::end( r ), init );
}

template <typename Range, typename T, typename BinaryOperation>
inline Range& accumulate( Range& r, const T& init, BinaryOperation o )
{
  return std::accumulate( range::begin( r ), range::end( r ), init, o );
}

// This could probably be done with some SFINAE magic, for now just add a suffix
template <typename Range, typename T, typename Proj>
inline T accumulate_proj( Range& r, const T& init, Proj proj )
{
  const auto op = [&proj]( const T& current, auto&& v ) {
    return range::invoke( proj, std::forward<decltype(v)>( v ) ) + current;
  };
  return std::accumulate( range::begin( r ), range::end( r ), init, op );
}

#if defined( SC_GCC )
// Workaround for GCC 4.6+ optimization ( -O3 ) issue with filling C-arrays
template <typename T, size_t N>
inline T ( &fill( T ( &r )[ N ], const T& t ) )[ N ]
{
  for ( size_t i = 0; i < N; ++i )
  {
    r[ i ] = t;
  }
  return r;
}
#endif

template <typename Range, typename T>
inline iterator_t<Range> find( Range& r, T const& t )
{
  // Static assert for human-readable error message. Not 100% technically
  // correct, since "comparability" is enough, but for our purposes convertible
  // is good enough.
  static_assert(
      std::is_convertible<T, value_type_t<Range>>::value,
      "Object to find must be convertible to value type of range" );
  return std::find( range::begin( r ), range::end( r ), t );
}

template <typename Range, typename T, typename Proj>
inline iterator_t<Range> find( Range& r, T const& value, Proj proj )
{
  const auto pred = [&value, &proj]( auto&& v ) {
    return range::invoke( proj, std::forward<decltype(v)>( v ) ) == value;
  };
  return std::find_if( range::begin( r ), range::end( r ), pred );
}

template <typename Range, typename UnaryPredicate>
inline iterator_t<Range> find_if( Range& r, UnaryPredicate p )
{
  return std::find_if( range::begin( r ), range::end( r ), p );
}

template <typename Range, typename UnaryPredicate>
inline bool any_of( Range&& r, UnaryPredicate p )
{
  return std::any_of( range::begin( r ), range::end( r ), p );
}

/**
 * Check if a value is contained in a range
 *
 * Equal to std::any_of with equality predicate, or std::find with checking for end of range.
 */
template <typename Range, typename Value, typename Proj = identity>
inline bool contains( Range&& r, const Value& v, Proj proj = Proj{} )
{
  return range::find( r, v, proj ) != range::end( r );
}

template <typename Range, typename F>
inline F for_each( Range&& r, F f )
{
  return std::for_each( range::begin( r ), range::end( r ), f );
}

template <typename Range, typename Out, typename Predicate>
inline Out remove_copy_if( Range& r, Out o, Predicate p )
{
  return std::remove_copy_if( range::begin( r ), range::end( r ), o, p );
}

template <typename Range, typename Out, typename F>
inline Out transform( Range&& r, Out o, F f )
{
  return std::transform( range::begin( r ), range::end( r ), o, f );
}

template <typename Range, typename Range2, typename Out, typename F>
inline Out transform( Range&& r, Range2&& r2, Out o, F f )
{
  return std::transform( range::begin( r ), range::end( r ), range::begin( r2 ),
                         o, f );
}

template <typename Range, typename F>
inline iterator_t<Range> transform_self( Range& r, F f )
{
  return std::transform( range::begin( r ), range::end( r ), range::begin( r ),
                         f );
}

template <typename Range1, typename Range2, typename Out>
inline Out set_difference( const Range1& left, const Range2& right, Out o )
{
  return std::set_difference( range::begin( left ), range::end( left ),
                              range::begin( right ), range::end( right ), o );
}

template <typename Range1, typename Range2, typename Out, typename Compare>
inline Out set_difference( const Range1& left, const Range2& right, Out o,
                           Compare c )
{
  return std::set_difference( range::begin( left ), range::end( left ),
                              range::begin( right ), range::end( right ), o,
                              c );
}

template <typename Range1, typename Range2, typename Out>
inline Out set_intersection( const Range1& left, const Range2& right, Out o )
{
  return std::set_intersection( range::begin( left ), range::end( left ),
                                range::begin( right ), range::end( right ), o );
}

template <typename Range1, typename Range2, typename Out, typename Compare>
inline Out set_intersection( const Range1& left, const Range2& right, Out o,
                             Compare c )
{
  return std::set_intersection( range::begin( left ), range::end( left ),
                                range::begin( right ), range::end( right ), o,
                                c );
}

template <typename Range1, typename Range2, typename Out>
inline Out set_union( const Range1& left, const Range2& right, Out o )
{
  return std::set_union( range::begin( left ), range::end( left ),
                         range::begin( right ), range::end( right ), o );
}

template <typename Range1, typename Range2, typename Out, typename Compare>
inline Out set_union( const Range1& left, const Range2& right, Out o,
                      Compare c )
{
  return std::set_union( range::begin( left ), range::end( left ),
                         range::begin( right ), range::end( right ), o, c );
}

template <typename Range>
inline Range& sort( Range& r )
{
  std::sort( range::begin( r ), range::end( r ) );
  return r;
}

template <typename Range, typename Comp>
inline Range& sort( Range& r, Comp c )
{
  std::sort( range::begin( r ), range::end( r ), c );
  return r;
}

template <typename Range>
inline iterator_t<Range> unique( Range& r )
{
  return std::unique( range::begin( r ), range::end( r ) );
}

template <typename Range, typename Comp>
inline iterator_t<Range> unique( Range& r, Comp c )
{
  return std::unique( range::begin( r ), range::end( r ), c );
}

template <typename Range>
inline iterator_t<Range> max_element( Range& r )
{
  return std::max_element( range::begin( r ), range::end( r ) );
}

template <typename Range>
inline iterator_t<Range> min_element( Range& r )
{
  return std::min_element( range::begin( r ), range::end( r ) );
}

template <typename Range1, typename Range2>
inline void append( Range1& destination, Range2&& source )
{
  destination.insert( destination.end(), source.begin(), source.end() );
}

template <typename Range, typename Predicate>
inline auto count_if( Range&& r, Predicate p ) -> typename std::iterator_traits<decltype(range::begin( r ))>::difference_type
{
  return std::count_if( range::begin( r ), range::end( r ), p );
}

template <typename Range, typename T, typename Compare = std::less<>, typename Proj = identity>
iterator_t<Range> lower_bound( Range& r, const T& value, Compare comp = Compare{}, Proj proj = Proj{} )
{
  const auto pred = [&value, &proj, &comp]( auto&& v ) {
    return range::invoke( comp, range::invoke( proj, std::forward<decltype(v)>( v ) ), value );
  };
  return std::partition_point( range::begin( r ), range::end( r ), pred );
}

template <typename Range, typename T, typename Compare = std::less<>, typename Proj = identity>
iterator_t<Range> upper_bound( Range& r, const T& value, Compare comp = Compare{}, Proj proj = Proj{} )
{
  const auto pred = [&value, &proj, &comp]( auto&& v ) {
    return !range::invoke( comp, value, range::invoke( proj, std::forward<decltype(v)>( v ) ) );
  };
  return std::partition_point( range::begin( r ), range::end( r ), pred );
}

template <typename Range, typename T, typename Compare = std::less<>, typename Proj = identity>
std::pair<iterator_t<Range>, iterator_t<Range>>
  equal_range( Range& r, const T& value, Compare comp = Compare{}, Proj proj = Proj{} )
{
  auto b = lower_bound( r, value, comp, proj );
  if ( b == range::end( r ) )
    return { b, b };
  return { b, upper_bound( r, value, comp, proj ) };
}

template <typename Range, typename Pred, typename Proj = identity>
iterator_t<Range> partition( Range& r, Pred pred_, Proj proj = Proj{} )
{
  auto pred = [&pred_, &proj]( auto&& v ) -> bool {
    return range::invoke( pred_, range::invoke( proj, std::forward<decltype(v)>( v ) ) );
  };
  return std::partition( range::begin( r ), range::end( r ), pred );
}

}  // namespace range ========================================================

// Adapter for container of owned pointers; automatically deletes the
// pointers on destruction.
template <typename Container>
class auto_dispose : public Container
{
private:
  void dispose_()
  {
    range::dispose( *this );
  }

public:
  ~auto_dispose()
  {
    dispose_();
  }
  void dispose()
  {
    dispose_();
    Container::clear();
  }
};

/**
 * Fancy type-casting function to use when we "know" what type an object pointer
 * really is. Makes sure we are right when debugging.
 */
template <typename To, typename From>
inline To debug_cast( From* ptr )
{
#ifdef NDEBUG
  return static_cast<To>( ptr );
#else
  To result = dynamic_cast<To>( ptr );
  if ( ptr )
    assert( result );
  return result;
#endif
}

/**
 * Fancy type-casting function to convert between types of different size and
 * signedness.
 * When debugging, verifies that the value is representable by both types.
 */
template <typename To, typename From>
inline To as( From f )
{
  To t = static_cast<To>( f );
  // Casting between arithmetic types
  static_assert( std::is_arithmetic<To>::value, "Output type is not arithmetic." );
  static_assert( std::is_arithmetic<From>::value, "Input type is not arithmetic." );
  // is "safe" if (a) it's reversible, and
  assert( f == static_cast<From>( t ) );
  // (b) both values have the same sign.
  assert( ( From() < f ) == ( To() < t ) );
  return t;
}

template <typename T>
inline T clamp( T value, T low, T high )
{
  assert( !( high < low ) );
  return ( value < low ? low : ( high < value ? high : value ) );
}

/**
 * Remove in O(1) by copying the last element over the element to be
 * erased and shrinking the sequence. DOES NOT PRESERVE ELEMENT ORDERING.
 */
template <typename Sequence>
void erase_unordered( Sequence& s, typename Sequence::iterator pos )
{
  assert( !s.empty() && pos != s.end() );
  typename Sequence::iterator last = s.end();
  --last;
  if ( pos != last )
    *pos = *last;  // *pos = std::move( *last );
  s.pop_back();
}

// Experimental propagate_const class, which preserves constness for class
// member pointers, as if they were value/reference types.
// Source: http://en.cppreference.com/w/cpp/experimental/propagate_const ,
// simplified adaption by scamille@2017-03-15
template <class T>
class propagate_const
{
public:
  using element_type =
      typename std::remove_reference<decltype( *std::declval<T&>() )>::type;

  propagate_const() = default;

  propagate_const( const propagate_const& p ) = delete;

  //propagate_const( propagate_const&& p ) = default;

  template <typename U>
  propagate_const( U&& u ) : _M_t( std::forward<U>( u ) )
  {
  }

  explicit operator bool() const
  {
    return static_cast<bool>( _M_t );
  }

  const element_type* operator->() const
  {
    return this->get();
  }

  operator const element_type*() const
  {
    return this->get();
  }

  const element_type& operator*() const
  {
    return *this->get();
  }

  const element_type* get() const
  {
    return _M_t;
  }

  element_type*& get_ref()
  {
    return _M_t;
  }

  // [propagate_const.non_const_observers], non-const observers

  element_type* operator->()
  {
    return this->get();
  }

  operator element_type*()
  {
    return this->get();
  }

  element_type& operator*()
  {
    return *this->get();
  }

  element_type* get()
  {
    return _M_t;
  }

  template <typename U>
  propagate_const& operator=( U&& u )
  {
    this->_M_t = std::forward<U>( u );
    return *this;
  }

private:
  T _M_t;  // exposition only
};

#endif  // SC_GENERIC_HPP
