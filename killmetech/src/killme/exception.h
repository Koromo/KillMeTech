#ifndef _KILLME_EXCEPTION_H_
#define _KILLME_EXCEPTION_H_

#include <exception>
#include <string>
#include <utility>

namespace killme
{
    /** Returns "value" if it is not 0 object, Otherwise throw exception "E" */
    /// TODO: Right value reference about "T"
    template <class E, class T, class... Args>
    T enforce(T value, Args&&... args)
    {
        if (!value)
        {
            throw E(std::forward<Args>(args)...);
        }
        return value;
    }

    /** Root exception class of Kill Me Tech */
    class Exception : public std::exception
    {
    private:
        std::string msg_;

    public:
        /** Construct with a message */
        explicit Exception(const std::string& msg);

        /** Returns message */
        std::string getMessage() const;

        /** Returns message that is same to the Exception::getMessage() */
        const char* what() const;
    };
}

#endif