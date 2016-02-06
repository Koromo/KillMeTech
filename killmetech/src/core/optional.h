#ifndef _KILLME_OPTIONAL_H_
#define _KILLME_OPTIONAL_H_

#include "exception.h"
#include <utility>
#include <memory>

namespace killme
{
    /** Optional exception */
    class OptionalException : public Exception
    {
    public:
        OptionalException(const std::string& msg) : Exception(msg) {}
    };

    namespace detail
    {
        class NullOpt;
    }

    /// TODO: Dynamic memory
    template <class T>
    class Optional
    {
    private:
        std::unique_ptr<T> value_;

    public:
        Optional() noexcept = default;

        Optional(const detail::NullOpt&) noexcept
            : Optional()
        {
        }

        template <class U>
        Optional(U&& value)
            : value_(std::make_unique<T>(std::forward<U>(value)))
        {
        }

        Optional(const Optional& lhs)
            : value_()
        {
            *this = lhs;
        }

        Optional(Optional&& rhs)
            : value_()
        {
            *this = std::move(rhs);
        }

        virtual ~Optional() = default;

        Optional& operator =(const detail::NullOpt&) noexcept
        {
            value_.reset();
            return *this;
        }

        template <class U>
        Optional& operator =(U&& value)
        {
            value_ = std::make_unique<T>(std::forward<U>(value));
            return *this;
        }

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

        Optional& operator =(Optional&& rhs)
        {
            value_ = std::move(rhs.value_);
            return *this;
        }

        operator bool() const noexcept
        {
            return !!value_;
        }

        const T* operator ->() const noexcept
        {
            enforce<OptionalException>(*this, "Optional is null.");
            return value_.get();
        }

        T* operator ->()  noexcept
        {
            return const_cast<T*>(static_cast<const Optional&>(*this).operator->());
        }

        const T& operator *() const  noexcept
        {
            return *(this->operator->());
        }

        T& operator *()  noexcept
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

    const detail::NullOpt nullopt;
}

#endif