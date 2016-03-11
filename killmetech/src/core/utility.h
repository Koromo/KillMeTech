#ifndef _KILLME_UTILITY_H_
#define _KILLME_UTILITY_H_

#include <utility>
#include <type_traits>

/** Generate unique id */
#ifdef __COUNTER__
#define KILLME_ID __COUNTER__
#elif
#define KILLME_ID __LINE__
#endif

/** Cat string */
#define KILLME_CAT(a, b) (a ## b)

namespace killme
{
    namespace detail
    {
        template <class It>
        struct IteratorRange
        {
            It begin_;
            It end_;

            IteratorRange() = default;

            IteratorRange(It begin, It end)
                : begin_(begin)
                , end_(end)
            {
            }

            IteratorRange(const IteratorRange&) = default;
            IteratorRange(IteratorRange&&) = default;

            IteratorRange& operator =(const IteratorRange&) = default;
            IteratorRange& operator =(IteratorRange&&) = default;

            It begin() const
            {
                return begin_;
            }

            It end() const
            {
                return end_;
            }
        };

        template <class C>
        struct EmplaceRange
        {
            C c_;

            EmplaceRange() = default;

            EmplaceRange(C&& c)
                : c_(std::move(c))
            {
            }

            EmplaceRange(EmplaceRange&& rhs)
                : c_(std::move(rhs.c_))
            {
            }

            EmplaceRange(const EmplaceRange&) = delete;

            EmplaceRange& operator =(EmplaceRange&& rhs)
            {
                c_ = std::move(rhs.c_);
                return *this;
            }

            EmplaceRange& operator =(const EmplaceRange&) = delete;

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
        };
    }

    /** Create a range from an iterator range */
    template <class It>
    auto iteratorRange(It begin, It end)
        -> decltype(detail::IteratorRange<It>(begin, end))
    {
        return detail::IteratorRange<It>(begin, end);
    }

    /** Create a range from a container */
    template <class C>
    auto constRange(const C& c)
        -> decltype(iteratorRange(std::cbegin(c), std::cend(c)))
    {
        return iteratorRange(std::cbegin(c), std::cend(c));
    }

    /** ditto */
    template <class C>
    auto mutableRange(C& c)
        -> decltype(iteratorRange(std::begin(c), std::end(c)))
    {
        return iteratorRange(std::begin(c), std::end(c));
    }

    /** ditto */
    template <class C>
    auto emplaceRange(C&& c)
        -> decltype(detail::EmplaceRange<C>(std::move(c)))
    {
        return detail::EmplaceRange<C>(std::move(c));
    }

    namespace detail
    {
        template <class T>
        struct TypeNumberGen
        {
            static void id() {}
        };
    }

    /** Unique type number */
    using TypeNumber = void(*)();

    /** Return the type number */
    template <class T>
    TypeNumber typeNumber() noexcept
    {
        return detail::TypeNumberGen<T>::id;
    }

    /** Index sequence */
    template <size_t... Indices>
    struct IndexSequence
    {
        static constexpr size_t length = sizeof...(Indices);
    };

    namespace detail
    {
        // Sequence {0...M-1, Indices...}
        template <size_t M, size_t... Indices>
        struct IndexSequenceImpl : IndexSequenceImpl<M - 1, M - 1, Indices...>
        {
        };

        template <size_t... Indices>
        struct IndexSequenceImpl<0, Indices...>
        {
            using Seq = IndexSequence<Indices...>;
        };
    }

    /** Create [0, N) sequence */
    template <size_t N>
    auto makeIndexSequence()
        -> typename detail::IndexSequenceImpl<N>::Seq
    {
        return typename detail::IndexSequenceImpl<N>::Seq();
    }

    namespace detail
    {
        template <size_t N, class Arg, class... Args>
        struct NthTypeImpl : NthTypeImpl<N - 1, Args...>
        {
        };

        template <class Arg, class... Args>
        struct NthTypeImpl<0, Arg, Args...>
        {
            using Type = Arg;
        };
    }

    /** N'th type in argments */
    template <size_t N, class... Args>
    using NthType = typename detail::NthTypeImpl<N, Args...>::Type;

    /** Unique id generator */
    template <class T>
    class UniqueCounter
    {
    private:
        T cnt_;

    public:
        UniqueCounter() : cnt_(0) {}
        T operator ()() { return cnt_++; }
    };

    /** Type converter */
    template <class T, class U>
    T to(const U& u);
}

#endif
