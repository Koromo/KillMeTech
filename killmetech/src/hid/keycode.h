#ifndef _KILLME_KEYCODE_H_
#define _KILLME_KEYCODE_H_

#include "../core/utility.h"
#include <string>
#include <cassert>

namespace killme
{
    /** Key code definitions */
    enum class KeyCode
    {
        Esc,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        none
    };

    /** Count of the key code */
    constexpr size_t NUM_KEY_CODES = static_cast<size_t>(KeyCode::none);

    /** Return the stringed KeyCode */
#define SWITCH_CASE(key) case KeyCode::key: return #key;

    template <>
    inline std::string to<std::string, KeyCode>(const KeyCode& code)
    {
        switch (code)
        {
            SWITCH_CASE(Esc);
            SWITCH_CASE(A);
            SWITCH_CASE(B);
            SWITCH_CASE(C);
            SWITCH_CASE(D);
            SWITCH_CASE(E);
            SWITCH_CASE(F);
            SWITCH_CASE(G);
            SWITCH_CASE(H);
            SWITCH_CASE(I);
            SWITCH_CASE(J);
            SWITCH_CASE(K);
            SWITCH_CASE(L);
            SWITCH_CASE(M);
            SWITCH_CASE(N);
            SWITCH_CASE(O);
            SWITCH_CASE(P);
            SWITCH_CASE(Q);
            SWITCH_CASE(R);
            SWITCH_CASE(S);
            SWITCH_CASE(T);
            SWITCH_CASE(U);
            SWITCH_CASE(V);
            SWITCH_CASE(W);
            SWITCH_CASE(X);
            SWITCH_CASE(Y);
            SWITCH_CASE(Z);

        default:
            assert(false && "Item not found.");
        }

        return ""; // For warnings
    }

#undef SWITCH_CASE
}

#endif