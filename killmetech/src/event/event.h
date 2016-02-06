#ifndef _KILLME_EVENT_H_
#define _KILLME_EVENT_H_

#include "../core/variant.h"
#include "../core/string.h"
#include <string>
#include <vector>

namespace killme
{
    /** The Event */
    class Event
    {
    private:
        std::string type_;
        std::vector<Variant> params_;

    public:
        /** Constructs with a event type */
        explicit Event(const std::string& type, size_t numParams)
            : type_(type)
            , params_(numParams)
        {
        }

        /** Returns event type */
        std::string getType() const
        {
            return type_;
        }

        /** Accesses to the i'th parameter */
        const Variant& operator[](size_t i) const
        {
            return params_[i];
        }

        /** ditto */
        Variant& operator[](size_t i)
        {
            return const_cast<Variant&>(static_cast<const Event&>(*this)[i]);
        }
    };

    /** The event type definitions */
    struct EventType
    {
        /** Input events */
        static const std::string keyPressed;
        static const std::string keyReleased;

        /** Windows events */
        static const std::string win_KeyDown;
        static const std::string win_KeyUp;
    };
}

#endif