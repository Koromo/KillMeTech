#ifndef _KILLME_EVENT_H_
#define _KILLME_EVENT_H_

#include "../core/variant.h"
#include "../core/string.h"
#include <string>
#include <vector>

namespace killme
{
    /** Event abstruction basic class */
    class Event
    {
    private:
        std::string type_;
        std::vector<Variant> params_;

    public:
        /** Construct with a event type */
        explicit Event(const std::string& type, size_t numParams)
            : type_(type)
            , params_(numParams)
        {
        }

        /** Returns event type */
        operator std::string() const
        {
            return type_;
        }

        /** Parameter accessor */
        const Variant& operator[](size_t i) const
        {
            return params_[i];
        }

        Variant& operator[](size_t i)
        {
            return const_cast<Variant&>(static_cast<const Event&>(*this)[i]);
        }
    };

    inline bool operator==(const Event& e, const std::string& s)
    {
        return toLowers(e) == toLowers(s);
    }

    inline bool operator==(const std::string& s, const Event& e)
    {
        return e == s;
    }

    inline std::string eventType(const Event& e)
    {
        return e;
    }
}

#endif