#include "event.h"

namespace killme
{
    Event::Event(const std::string& type)
        : type_(type)
    {
    }

    std::string Event::getType() const
    {
        return type_;
    }
}