#ifndef _KILLME_VARIANT_H_
#define _KILLME_VARIANT_H_

#include "utility.h"
#include <memory>
#include <utility>
#include <string>
#include <type_traits>
#include <cassert>

namespace killme
{
    /** Variant type */
    /// TODO: Not support hold reference value
    class Variant
    {
    private:
        template <class T>
        using FixedType = std::remove_const_t<std::remove_reference_t<T>>;

        // Holder
        struct Holder
        {
            virtual ~Holder() noexcept = default;
            virtual Holder* copy() const = 0;
            virtual TypeNumber type() const noexcept = 0;
            virtual size_t sizeOf() const noexcept = 0;
            virtual const void* ptr() const noexcept = 0;
        };

        template <class T>
        struct TypedHolder : Holder
        {
            T value;

            template <class U>
            TypedHolder(U&& val) : value(std::forward<U>(val)) {}

            Holder* copy() const { return new TypedHolder<T>(value); }
            TypeNumber type() const noexcept { return typeNumber<T>(); }
            size_t sizeOf() const noexcept { return sizeof(T); }
            const void* ptr() const noexcept { return &value; }
        };

        std::shared_ptr<Holder> holder_;

    public:
        /** Construct */
        Variant() noexcept = default;

        /** Construct with a value  */
        /// TOOD: We does not use explicit
        template <class T, class U = FixedType<T>>
        explicit Variant(T&& value)
            : holder_(std::make_shared<TypedHolder<U>>(std::forward<T>(value)))
        {
        }

        /** Copy constructor */
        Variant(const Variant& lhs) noexcept
            : holder_()
        {
            *this = lhs;
        }

        /** Move constructor */
        Variant(Variant&& rhs) noexcept
            : holder_(std::move(rhs.holder_))
        {
        }

        /** Assignment operator with a value */
        template <class T, class U = FixedType<T>>
        Variant& operator =(T&& value)
        {
            holder_ = std::make_shared<TypedHolder<U>>(std::forward<T>(value));
            return *this;
        }

        /** Copy assignment operator */
        Variant& operator =(const Variant& lhs)
        {
            holder_ = lhs.holder_;
            if (holder_)
            {
                holder_.reset(holder_->copy());
            }
            return *this;
        }

        /** Move assignment operator */
        Variant& operator =(Variant&& rhs) noexcept
        {
            holder_ = std::move(rhs.holder_);
            return *this;
        }

        /** Equivalent test */
        template <class T, class U = FixedType<T>>
        bool operator ==(const T& a) const noexcept
        {
            if (!hasValue() || !killme::is<U>(*this))
            {
                return false;
            }
            return std::dynamic_pointer_cast<TypedHolder<U>>(holder_)->value == a;
        }

        // Cast
        template <class T, class U = FixedType<T>>
        T cast() const noexcept
        {
            assert(hasValue() && "Variant has not value.");
            assert(killme::is<U>(*this) && "Variant type not match.");
            return static_cast<const U&>(std::dynamic_pointer_cast<TypedHolder<U>>(holder_)->value);
        }

        /** ditto */
        template <class T, class U = FixedType<T>>
        T cast() noexcept
        {
            return const_cast<U&>(static_cast<const Variant&>(*this).template cast<const U&>());

        }
        /** Cast operator */
        template <class T, class U = FixedType<T>>
        operator T() const noexcept
        {
            return this->template cast<T>();
        }

        /** ditto */
        template <class T, class U = FixedType<T>>
        operator T() noexcept
        {
            return this->template cast<T>();
        }

        /** Return true if Variant has a value */
        bool hasValue() const noexcept
        {
            return !!holder_;
        }

        /** Return size of the hold value */
        size_t sizeOf() const noexcept
        {
            if (!hasValue())
            {
                return 0;
            }
            return holder_->sizeOf();
        }

        /** Return pointer of the hold value */
        const void* ptr() const noexcept
        {
            if (!hasValue())
            {
                return nullptr;
            }
            return holder_->ptr();
        }

        // For killme::is()
        template <class T>
        bool isSame() const noexcept
        {
            return hasValue() && typeNumber<T>() == holder_->type();
        }
    };

    /** Cast to "T" */
    template <class T>
    T to(const Variant& var) noexcept
    {
        return var.template cast<T>();
    }

    /** ditto */
    template <class T>
    T to(Variant& var) noexcept
    {
        return var.template cast<T>();
    }

    /** ditto */
    template <class T>
    T to(Variant&& var) noexcept
    {
        return var.template cast<T>();
    }

    /** Return true if the type of a variant value is same to the "T" */
    template <class T>
    bool is(const Variant& v) noexcept
    {
        return v.template isSame<T>();
    }

    /** Equivalent tests */
    template <class T>
    bool operator==(const T& a, const Variant& v) noexcept
    {
        return v == a;
    }

    template <class T>
    bool operator!=(const Variant& v, const T& a) noexcept
    {
        return !(v == a);
    }

    template <class T>
    bool operator!=(const T& a, const Variant& v) noexcept
    {
        return !(v == a);
    }
}

#endif