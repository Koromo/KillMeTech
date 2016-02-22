#ifndef _KILLME_UTILITY_H_
#define _KILLME_UTILITY_H_

#include <utility>
#include <type_traits>

namespace killme
{
    namespace detail
    {
        template <class It>
        struct RangeFromIterator
        {
            It begin_;
            It end_;

            RangeFromIterator() = default;

            RangeFromIterator(It begin, It end)
                : begin_(begin)
                , end_(end)
            {
            }

            RangeFromIterator(const RangeFromIterator&) = default;
            RangeFromIterator(RangeFromIterator&&) = default;

            RangeFromIterator& operator =(const RangeFromIterator&) = default;
            RangeFromIterator& operator =(RangeFromIterator&&) = default;

            It begin() const
            {
                return begin_;
            }

            It end() const
            {
                return end_;
            }

            size_t length() const
            {
                return end_ - begin_;
            }
        };

        template <class C>
        struct RangeWithMove
        {
            C c_;

            RangeWithMove() = default;

            RangeWithMove(C&& c)
                : c_(std::move(c))
            {
            }

            RangeWithMove(RangeWithMove&& rhs)
                : c_(std::move(rhs.c_))
            {
            }

            RangeWithMove(const RangeWithMove&) = delete;

            RangeWithMove& operator =(RangeWithMove&& rhs)
            {
                c_ = std::move(rhs.c_);
                return *this;
            }

            RangeWithMove& operator =(const RangeWithMove&) = delete;

            auto begin()
                -> decltype(std::begin(c_))
            {
                return std::begin(c_);
            }

            auto end()
                -> decltype(std::end(c_))
            {
                return std::end(c_);
            }

            auto begin() const
                -> decltype(std::cbegin(c_))
            {
                return std::cbegin(c_);
            }

            auto end() const
                -> decltype(std::cend(c_))
            {
                return std::cend(c_);
            }

            size_t length() const
            {
                return std::cend(c_) - std::cbegin(c_);
            }
        };
    }

    /** Create the range from an iterator range */
    template <class It>
    detail::RangeFromIterator<It> makeRange(It begin, It end)
    {
        return detail::RangeFromIterator<It>(begin, end);
    }

    /** Create the range from a container */
    template <class C>
    auto makeRange(const C& c)
        -> decltype(makeRange(std::cbegin(c), std::cend(c)))
    {
        return makeRange(std::cbegin(c), std::cend(c));
    }

    /** ditto */
    template <class C>
    auto makeRange(C& c)
        -> decltype(makeRange(std::begin(c), std::end(c)))
    {
        return makeRange(std::begin(c), std::end(c));
    }

    /** Create the range with move a right value container */
    template <class C>
    detail::RangeWithMove<C> makeRange(C&& c)
    {
        return detail::RangeWithMove<C>(std::move(c));
    }

    /** The alias of decltype(makeRange(C)) */
    template <class C>
    using Range_t = decltype(makeRange(std::forward<C>(std::declval<C>())));

    /** The type converter */
    template <class T, class U>
    T to(const U& u);
}

#endif
