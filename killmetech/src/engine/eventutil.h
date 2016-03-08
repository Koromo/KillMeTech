#ifndef _KILLME_EVENTUTIL_H_
#define _KILLME_EVENTUTIL_H_

#include "actor.h"
#include "level.h"
#include "../events/event.h"
#include <vector>
#include <functional>
#include <string>
#include <type_traits>
#include <cassert>

namespace killme
{
    namespace detail
    {
        template <class C, class R, class... Args>
        void unpackEventCall(R(*fun)(Args...), C&& c, const Event& e)
        {
            killme::unpackEventCall(fun, e);
        }

        template <class C, class R, class... Args>
        void unpackEventCall(R(C::*fun)(Args...), C&& c, const Event& e)
        {
            killme::unpackEventCall(fun, c, e);
        }

        template <class C, class R, class... Args>
        void unpackEventCall(R(C::*fun)(Args...) const, const C& c, const Event& e)
        {
            killme::unpackEventCall(fun, c, e);
        }

        struct EventHookInfo
        {
            bool levelEvent;
            std::string type;
            std::function<void(const Event&)> hook;
        };

        extern void* enabler;
    }
}

// Implementation macros for event hook utility
#define KILLME_EVENT_HOOK_CONNECTIONS killme_event_hook_connections
#define KILLME_EVENT_HOOK_INFORMATIONS killme_event_hook_informations
#define KILLME_CONNECT_EVENT_HOOKS_FUNCTION killme_connect_event_hooks_function

/** Begin event hooks definition */
#define KILLME_EVENT_HOOKS_BEGIN \
    private: \
    std::vector<killme::EventConnection> KILLME_EVENT_HOOK_CONNECTIONS; \
    const std::vector<killme::detail::EventHookInfo> KILLME_EVENT_HOOK_INFORMATIONS = {

/** Define a level event hook */
#define KILLME_LEVEL_EVENT_HOOK(type, fun) \
    { true, type, [&](const killme::Event& e) { killme::detail::unpackEventCall(fun, *this, e); } },

/** Define an actor event hook */
#define KILLME_ACTOR_EVENT_HOOK(type, fun) \
    { false, type, [&](const killme::Event& e) { killme::detail::unpackEventCall(fun, *this, e); } },

/** End event hooks definition */
#define KILLME_EVENT_HOOKS_END \
    }; \
    template <class C, typename std::enable_if_t<std::is_base_of<Level, C>::value>*& = killme::detail::enabler> \
    void KILLME_CONNECT_EVENT_HOOKS_FUNCTION(const C*) { \
        auto it = std::cbegin(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        const auto end = std::cend(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        while (it != end) { \
            if (it->levelEvent) { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->connect(it->type, it->hook)); } \
            else { assert(false && "You can not hook actor events from level by KILLME_CONNECT_EVENT_HOOKS macro."); } \
            ++it; \
        } \
    } \
    template <class C, typename std::enable_if_t<std::is_base_of<Actor, C>::value>*& = killme::detail::enabler> \
    void KILLME_CONNECT_EVENT_HOOKS_FUNCTION(const C*) { \
        auto it = std::cbegin(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        const auto end = std::cend(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        while (it != end) { \
            if (it->levelEvent) { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->getOwnerLevel().connect(it->type, it->hook)); } \
            else { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->connect(it->type, it->hook)); } \
            ++it; \
        } \
    } \

/** Connect all defined event hooks. You need call on Level::onBeginLevel() or Actor::onSpawn(). */
#define KILLME_CONNECT_EVENT_HOOKS() \
    (this->KILLME_CONNECT_EVENT_HOOKS_FUNCTION(this))

#endif