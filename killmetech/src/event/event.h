#ifndef _KILLME_EVENT_H_
#define _KILLME_EVENT_H_

#include <string>

namespace killme
{
    /** Event abstruction basic class */
    class Event
    {
    private:
        std::string type_;

    public:
        /** Construct with a event type */
        explicit Event(const std::string& type) : type_(type) {}

        /** For drived classes */
        virtual ~Event() = default;

        /** Returns event type */
        std::string getType() const { return type_; }
    };
}

#endif