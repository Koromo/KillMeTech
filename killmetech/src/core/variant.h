#ifndef _KILLME_VARIANT_H_
#define _KILLME_VARIANT_H_

#include "exception.h"
#include <memory>
#include <utility>
#include <string>

namespace killme
{
    /** Variant exception */
    class VariantException : public Exception
    {
    public:
        VariantException(const std::string& msg) : Exception(msg) {}
    };

    /** Variant type */
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
            TypedHolder(T val) : value(val) {}
            Holder* copy() const { return new TypedHolder<T>(value); }
            TypeId typeId() const noexcept { return TypeId_t<T>::id; }
        };

        std::shared_ptr<Holder> holder_;

    public:
        /** Construct */
        Variant() noexcept = default;

        /** Construct with a value  */
        template <class T>
        Variant(T value)
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
        Variant& operator=(T value)
        {
            holder_ = std::make_shared<TypedHolder<T>>(value);
            return *this;
        }

        /** Copy assignment operator */
        Variant& operator=(const Variant& lhs)
        {
            holder_ = lhs.holder_;
            if (holder_)
            {
                holder_.reset(holder_->copy());
            }
            return *this;
        }

        /** Move assignment operator */
        Variant& operator=(Variant&& rhs)
        {
            holder_ = rhs.holder_;
            return *this;
        }

        /** Cast operator */
        template <class T>
        operator T() const
        {
            enforce<VariantException>(hasValue(), "Variant has not value.");
            enforce<VariantException>(is<T>(), "Not match variant type.");
            return std::dynamic_pointer_cast<TypedHolder<T>>(holder_)->value;
        }

        /** Swap */
        void swap(Variant& that) noexcept
        {
            std::swap(holder_, that.holder_);
        }

        /** Returns true if holded value type is same to the "T" */
        template <class T>
        bool is() const noexcept
        {
            return hasValue() && TypeId_t<T>::id == holder_->typeId();
        }

        /** Returns true if has value */
        bool hasValue() const noexcept
        {
            return !!holder_;
        }
    };

    /** Cast to "T" */
    template <class T>
    T to(const Variant& var)
    {
        return var;
    }
}

/** swap functions */
namespace killme
{
    inline void swap(killme::Variant& a, killme::Variant& b) noexcept
    {
        a.swap(b);
    }
}

namespace std
{
    template<>
    inline void swap(killme::Variant& a, killme::Variant& b) noexcept
    {
        a.swap(b);
    }
}

#endif