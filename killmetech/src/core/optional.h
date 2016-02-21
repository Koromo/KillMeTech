#ifndef _KILLME_OPTIONAL_H_
#define _KILLME_OPTIONAL_H_

#include "exception.h"
#include <utility>
#include <memory>

namespace killme
{
    /** The exception of Optional */
    class OptionalException : public Exception
    {
    public:
        OptionalException(const std::string& msg);
    };

    namespace detail
    {
        class NullOpt;
    }

    /** The Optional */
    /// NOTE: The Optional allocate memory dynamically.
    ///       Not support reference type.
    template <class T>
    class Optional
    {
    private:
        std::unique_ptr<T> value_;

    public:
        /** Constructs */
        Optional() noexcept = default;

        /** Constructs with the null */
        Optional(const detail::NullOpt&) noexcept
            : Optional()
        {
        }

        /** Constructs with a value */
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
        Optional& operator =(Optional&& rhs)
        {
            value_ = std::move(rhs.value_);
            return *this;
        }

        /** Returns true if Optional has a value. Otherwise, false. */
        operator bool() const noexcept
        {
            return !!value_;
        }

        /** Accesses to the value */
        const T* operator ->() const
        {
            enforce<OptionalException>(!!value_, "Optional is null.");
            return value_.get();
        }

        /** ditto */
        T* operator ->()
        {
            return const_cast<T*>(static_cast<const Optional&>(*this).operator->());
        }

        /** Returns refference of the value */
        const T& operator *() const
        {
            return *(this->operator->());
        }

        /** ditto */
        T& operator *()
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

    /** The invalid value */
    extern const detail::NullOpt nullopt;
}

#endif