#ifndef _KILLME_EVENT_H_
#define _KILLME_EVENT_H_

#include "../core/variant.h"
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
    };
}

#endif