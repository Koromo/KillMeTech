#ifndef _KILLME_UTILITY_H_
#define _KILLME_UTILITY_H_

namespace killme
{
    namespace detail
    {
        // For the makeRange()
        template <class It>
        class IteratorRange
        {
        private:
            It begin_;
            It end_;

        public:
            IteratorRange(It begin, It end)
                : begin_(begin)
                , end_(end)
            {
            }

            It begin() const
            {
                return begin_;
            }

            It end() const
            {
                return end_;
            }
        };
    }

    /** Create a range from a iterator range*/
    template <class It>
    detail::IteratorRange<It> makeRange(It begin, It end)
    {
        return detail::IteratorRange<It>(begin, end);
    }

    /** Create a range from a container */
    template <class C>
    auto makeRange(const C& c)
        -> decltype(makeRange(std::cbegin(c), std::cend(c)))
    {
        return makeRange(std::cbegin(c), std::cend(c));
    }

    /** The type converter */
    template <class T, class U>
    T to(const U&);
}

#endif
