#ifndef _KILLME_OPTIONAL_H_
#define _KILLME_OPTIONAL_H_

#include <utility>
#include <memory>
#include <cassert>

namespace killme
{
    namespace detail
    {
        class NullOpt;
    }

    /** Optional */
    /// NOTE: The Optional allocate memory dynamically.
    ///       Not support reference type.
    template <class T>
    class Optional
    {
    private:
        std::unique_ptr<T> value_;

    public:
        /** Construct */
        Optional() noexcept = default;

        /** Construct as the null */
        Optional(const detail::NullOpt&) noexcept
            : Optional()
        {
        }

        /** Construct with a value */
        Optional(const T& value)
            : value_(std::make_unique<T>(value))
        {
        }

        /** ditto */
        Optional(T&& value)
            : value_(std::make_unique<T>(std::move(value)))
        {
        }

        /** Copy constructor */
        Optional(const Optional& lhs)
            : value_()
        {
            *this = lhs;
        }

        /** Move constructor */
        Optional(Optional&& rhs)
            : value_(std::move(rhs.value_))
        {
        }

        /** Destructs */
        virtual ~Optional() = default;

        /** Assignment operator with the null */
        Optional& operator =(const detail::NullOpt&) noexcept
        {
            value_.reset();
            return *this;
        }

        /** Assignment operator with a value */
        /// TODO:
        template <class U>
        Optional& operator =(U&& value)
        {
            value_ = std::make_unique<T>(std::forward<U>(value));
            return *this;
        }

        /** Copy assignment operator */
        Optional& operator =(const Optional& lhs)
        {
            if (lhs)
            {
                value_ = std::make_unique<T>(*lhs);
            }
            else
            {
                value_.reset();
            }
            return *this;
        }

        /** Move assignment operator */
        Optional& operator =(Optional&& rhs) noexcept
        {
            value_ = std::move(rhs.value_);
            return *this;
        }

        /** Return true if Optional has a value */
        operator bool() const noexcept
        {
            return !!value_;
        }

        /** Access into the value */
        const T* operator ->() const noexcept
        {
            assert(*this && "Invalid access into null optional.");
            return value_.get();
        }

        /** ditto */
        T* operator ->() noexcept
        {
            return const_cast<T*>(static_cast<const Optional&>(*this).operator->());
        }

        /** ditto */
        const T& operator *() const noexcept
        {
            return *(this->operator->());
        }

        /** ditto */
        T& operator *() noexcept
        {
            return *(this->operator->());
        }
    };

    namespace detail
    {
        struct NullOpt_t
        {
        };

        class NullOpt : public Optional<NullOpt_t>
        {
        };
    }

    /** Null optional */
    extern const detail::NullOpt nullopt;
}

#endif