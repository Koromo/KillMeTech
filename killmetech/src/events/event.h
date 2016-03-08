#ifndef _KILLME_EVENT_H_
#define _KILLME_EVENT_H_

#include "../core/variant.h"
#include "../core/string.h"
#include <string>
#include <vector>

namespace killme
{
    namespace detail
    {
        inline void variantArray(std::vector<Variant>& arr)
        {
        }

        template <class Var, class... Vars>
        void variantArray(std::vector<Variant>& arr, Var&& var, Vars&&... vars)
        {
            arr.emplace_back(std::forward<Var>(var));
            variantArray(arr, std::forward<Vars>(vars)...);
        }
    }

    /** Event */
    class Event
    {
    private:
        std::string type_;
        std::vector<Variant> params_;

    public:
        /** Construct */
        template <class... Params>
        Event(const std::string& type, Params&&... params)
            : type_(type)
            , params_()
        {
            params_.reserve(sizeof...(Params));
            detail::variantArray(params_, std::forward<Params>(params)...);
        }

        /** Return the event type */
        std::string getType() const
        {
            return type_;
        }

        /** Accesses to i'th parameter */
        const Variant& operator [](size_t i) const
        {
            return params_[i];
        }

        /** Return count of parameters */
        size_t num() const
        {
            return params_.size();
        }
    };

    /** Call event hook with unpack event parameters */
    /// TODO: Not support lumbda expression and functor
    /** For function */
    template <class R, class... Args>
    R unpackEventCall(R(*fun)(Args...), const Event& e)
    {
        size_t i = e.num();
        return fun(to<Args>(e[--i])...);
    }

    /** For member function */
    template <class C, class R, class... Args>
    R unpackEventCall(R(C::*fun)(Args...), C&& c, const Event& e)
    {
        size_t i = e.num();
        return (c.*fun)(to<Args>(e[--i])...);
    }

    /** For const member function */
    template <class C, class R, class... Args>
    R unpackEventCall(R(C::*fun)(Args...) const, const C& c, const Event& e)
    {
        size_t i = e.num();
        return (c.*fun)(to<Args>(e[--i])...);
    }
}

#endif