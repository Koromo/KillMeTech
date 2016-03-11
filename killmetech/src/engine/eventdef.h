#ifndef _KILLME_EVENTDEF_H_
#define _KILLME_EVENTDEF_H_

#include "../events/event.h"
#include "../events/eventdispatcher.h"
#include "../core/variant.h"
#include <vector>
#include <functional>
#include <string>
#include <type_traits>
#include <cassert>

/** Event define utilities */
#define KILLME_DEFINE_EVENT(name) const std::string name = #name
#define KILLME_DEFINE_LEVEL_EVENT(name, ...) KILLME_DEFINE_EVENT(LEVEL_##name)
#define KILLME_DEFINE_ACTOR_EVENT(name, ...) KILLME_DEFINE_EVENT(ACTOR_##name)
#define KILLME_DEFINE_COMPONENT_EVENT(name, ...) KILLME_DEFINE_EVENT(COMPONENT_##name)

namespace killme
{
    /** Level event definitions */
    /** No parameters */
    KILLME_DEFINE_LEVEL_EVENT(BeginFrame);

    /** Input events
     *  0 bool: Whether key is pressed or not
     *  1 bool: Whether key is released or not
     **/
    KILLME_DEFINE_LEVEL_EVENT(Esc);
    KILLME_DEFINE_LEVEL_EVENT(A);
    KILLME_DEFINE_LEVEL_EVENT(B);
    KILLME_DEFINE_LEVEL_EVENT(C);
    KILLME_DEFINE_LEVEL_EVENT(D);
    KILLME_DEFINE_LEVEL_EVENT(E);
    KILLME_DEFINE_LEVEL_EVENT(F);
    KILLME_DEFINE_LEVEL_EVENT(G);
    KILLME_DEFINE_LEVEL_EVENT(H);
    KILLME_DEFINE_LEVEL_EVENT(I);
    KILLME_DEFINE_LEVEL_EVENT(J);
    KILLME_DEFINE_LEVEL_EVENT(K);
    KILLME_DEFINE_LEVEL_EVENT(L);
    KILLME_DEFINE_LEVEL_EVENT(M);
    KILLME_DEFINE_LEVEL_EVENT(N);
    KILLME_DEFINE_LEVEL_EVENT(O);
    KILLME_DEFINE_LEVEL_EVENT(P);
    KILLME_DEFINE_LEVEL_EVENT(Q);
    KILLME_DEFINE_LEVEL_EVENT(R);
    KILLME_DEFINE_LEVEL_EVENT(S);
    KILLME_DEFINE_LEVEL_EVENT(T);
    KILLME_DEFINE_LEVEL_EVENT(U);
    KILLME_DEFINE_LEVEL_EVENT(V);
    KILLME_DEFINE_LEVEL_EVENT(W);
    KILLME_DEFINE_LEVEL_EVENT(X);
    KILLME_DEFINE_LEVEL_EVENT(Y);
    KILLME_DEFINE_LEVEL_EVENT(Z);

    /** Actor event definitions */
    /**
     *  0 RigidBodyComponent: Collided body of this actor
     *  1 RigidBodyComponent: Collided body of other actor
     */
    KILLME_DEFINE_ACTOR_EVENT(Collided, RigidBodyComponent, RigidBodyComponent);

    /** Component event definitions */


    namespace detail
    {
        template <class C, class R, class... Args>
        R unpackEventCall(R(*fun)(Args...), C&&, const Event& e)
        {
            size_t i = e.num();
            return fun(to<Args>(e[--i])...);
        }

        template <class C, class R, class... Args>
        R unpackEventCall(R(C::*fun)(Args...), C& c, const Event& e)
        {
            size_t i = e.num();
            return (c.*fun)(to<Args>(e[--i])...);
        }

        template <class C, class R, class... Args>
        R unpackEventCall(R(C::*fun)(Args...) const, const C& c, const Event& e)
        {
            size_t i = e.num();
            return (c.*fun)(to<Args>(e[--i])...);
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
#define KILLME_CONNECT_EVENT_HOOKS_IMPL killme_connect_event_hooks_impl
#define KILLME_CONNECT_EVENT_HOOKS_FUNCTION killme_connect_event_hooks_function
#define KILLME_DISCONNECT_EVENT_HOOKS_FUNCTION killme_disconnect_event_hooks_function

/** Begin event hooks definition */
#define KILLME_EVENT_HOOKS_BEGIN \
    private: \
    std::vector<killme::EventConnection> KILLME_EVENT_HOOK_CONNECTIONS; \
    const std::vector<killme::detail::EventHookInfo> KILLME_EVENT_HOOK_INFORMATIONS = {

/** Define a level event hook */
#define KILLME_HOOK_LEVEL_EVENT(type, fun) \
    { true, type, [&](const killme::Event& e) { killme::detail::unpackEventCall(fun, *this, e); } },

/** Define an actor event hook */
#define KILLME_HOOK_ACTOR_EVENT(type, fun) \
    { false, type, [&](const killme::Event& e) { killme::detail::unpackEventCall(fun, *this, e); } },

/** End event hooks definition */
#define KILLME_EVENT_HOOKS_END \
    }; \
    template <class C, typename std::enable_if_t<std::is_base_of<Level, C>::value>*& = killme::detail::enabler> \
    void KILLME_CONNECT_EVENT_HOOKS_IMPL(const C*) { \
        auto it = std::cbegin(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        const auto end = std::cend(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        while (it != end) { \
            if (it->levelEvent) { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->connect(it->type, it->hook)); } \
            else { assert(false && "You can not hook actor events from level by KILLME_CONNECT_EVENT_HOOKS macro."); } \
            ++it; \
        } \
    } \
    template <class C, typename std::enable_if_t<std::is_base_of<Actor, C>::value>*& = killme::detail::enabler> \
    void KILLME_CONNECT_EVENT_HOOKS_IMPL(const C*) { \
        auto it = std::cbegin(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        const auto end = std::cend(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        while (it != end) { \
            if (it->levelEvent) { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->getOwnerLevel().connect(it->type, it->hook)); } \
            else { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->connect(it->type, it->hook)); } \
            ++it; \
        } \
    } \
    template <class C, typename std::enable_if_t<std::is_base_of<ActorComponent, C>::value>*& = killme::detail::enabler> \
    void KILLME_CONNECT_EVENT_HOOKS_IMPL(const C*) { \
        auto it = std::cbegin(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        const auto end = std::cend(this->KILLME_EVENT_HOOK_INFORMATIONS); \
        while (it != end) { \
            if (it->levelEvent) { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->getOwnerLevel().connect(it->type, it->hook)); } \
            else { this->KILLME_EVENT_HOOK_CONNECTIONS.emplace_back(this->getOwnerActor().connect(it->type, it->hook)); } \
            ++it; \
        } \
    } \
    virtual void KILLME_CONNECT_EVENT_HOOKS_FUNCTION() { \
        this->KILLME_CONNECT_EVENT_HOOKS_IMPL(this); \
    } \
    virtual void KILLME_DISCONNECT_EVENT_HOOKS_FUNCTION() { \
        this->KILLME_EVENT_HOOK_CONNECTIONS.clear(); \
    }

/** Connect all defined event hooks */
#define KILLME_CONNECT_EVENT_HOOKS() \
    (this->KILLME_CONNECT_EVENT_HOOKS_FUNCTION())

/** Disconnect all defined event hooks */
#define KILLME_DISCONNECT_EVENT_HOOKS() \
    (this->KILLME_DISCONNECT_EVENT_HOOKS_FUNCTION())

#endif