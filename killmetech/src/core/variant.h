#ifndef _KILLME_VARIANT_H_
#define _KILLME_VARIANT_H_

#include "exception.h"
#include <memory>
#include <utility>
#include <string>

namespace killme
{
    /** The exception of Variant */
    class VariantException : public Exception
    {
    public:
        VariantException(const std::string& msg) : Exception(msg) {}
    };

    /** The Variant type */
    /// TODO: l,rvalue
    class Variant
    {
    private:
        // Type id
        using TypeId = void(*)();

        template <class T>
        struct TypeId_t
        {
            static void id() {}
        };

        // Holder
        struct Holder
        {
            virtual ~Holder() noexcept = default;
            virtual Holder* copy() const = 0;
            virtual TypeId typeId() const noexcept = 0;
        };

        template <class T>
        struct TypedHolder : Holder
        {
            T value;

            template <class U>
            TypedHolder(U&& val) : value(std::forward<U>(val)) {}

            Holder* copy() const { return new TypedHolder<T>(value); }
            TypeId typeId() const noexcept { return TypeId_t<T>::id; }
        };

        std::shared_ptr<Holder> holder_;

    public:
        /** Constructs */
        Variant() noexcept = default;

        /** Constructs with a value  */
        template <class T>
        Variant(T&& value)
            : holder_()
        {
            *this = std::forward<T>(value);
        }

        /** Copy constructor */
        Variant(const Variant& lhs)
            : holder_()
        {
            *this = lhs;
        }

        /** Move constructor */
        Variant(Variant&& rhs)
            : holder_()
        {
            *this = std::move(rhs);
        }

        /** Assignment operator with a value */
        template <class T>
        Variant& operator =(T&& value)
        {
            holder_ = std::make_shared<TypedHolder<T>>(std::forward<T>(value));
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
        Variant& operator =(Variant&& rhs)
        {
            holder_ = std::move(rhs.holder_);
            return *this;
        }

        /** Cast operator */
        template <class T>
        operator T() const
        {
            enforce<VariantException>(hasValue(), "Variant has not value.");
            enforce<VariantException>(killme::is<T>(*this), "Not match variant type.");
            return std::dynamic_pointer_cast<TypedHolder<T>>(holder_)->value;
        }

        // For the killme::is()
        template <class T>
        bool is() const noexcept
        {
            return hasValue() && TypeId_t<T>::id == holder_->typeId();
        }

        /** Returns true if Variant has value */
        bool hasValue() const noexcept
        {
            return !!holder_;
        }
    };

    /** Cast to "T" */
    template <class T>
    inline T to(const Variant& var)
    {
        return var;
    }

    /** Returns true if the type of the value holded by Variant is same to the "T" */
    template <class T>
    bool is(const Variant& v) noexcept
    {
        return v.template is<T>();
    }
}

#endif