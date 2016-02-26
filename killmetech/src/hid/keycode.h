#ifndef _KILLME_KEYCODE_H_
#define _KILLME_KEYCODE_H_

#include "../core/utility.h"
#include <string>
#include <cassert>

namespace killme
{
    /** The key code definitions */
    enum class KeyCode
    {
        Esc,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        none
    };

    /** Count of the key code */
    constexpr size_t NUM_KEY_CODES = static_cast<size_t>(KeyCode::none);

    /** Returns the stringed KeyCode */
#define KILLME_SWITCH_STR(key) case KeyCode::key: return #key;

    template <>
    inline std::string to<std::string, KeyCode>(const KeyCode& code)
    {
        switch (code)
        {
            KILLME_SWITCH_STR(Esc);
            KILLME_SWITCH_STR(A);
            KILLME_SWITCH_STR(B);
            KILLME_SWITCH_STR(C);
            KILLME_SWITCH_STR(D);
            KILLME_SWITCH_STR(E);
            KILLME_SWITCH_STR(F);
            KILLME_SWITCH_STR(G);
            KILLME_SWITCH_STR(H);
            KILLME_SWITCH_STR(I);
            KILLME_SWITCH_STR(J);
            KILLME_SWITCH_STR(K);
            KILLME_SWITCH_STR(L);
            KILLME_SWITCH_STR(M);
            KILLME_SWITCH_STR(N);
            KILLME_SWITCH_STR(O);
            KILLME_SWITCH_STR(P);
            KILLME_SWITCH_STR(Q);
            KILLME_SWITCH_STR(R);
            KILLME_SWITCH_STR(S);
            KILLME_SWITCH_STR(T);
            KILLME_SWITCH_STR(U);
            KILLME_SWITCH_STR(V);
            KILLME_SWITCH_STR(W);
            KILLME_SWITCH_STR(X);
            KILLME_SWITCH_STR(Y);
            KILLME_SWITCH_STR(Z);

        default:
            assert(false && "Item not found.");
        }

        return ""; // For warnings
    }

#undef KILLME_SWITCH_STR
}

#endif