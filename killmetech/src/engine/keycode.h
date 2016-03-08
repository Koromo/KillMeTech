#ifndef _KILLME_KEYCODE_H_
#define _KILLME_KEYCODE_H_

#include "../core/utility.h"
#include <string>
#include <cassert>

namespace killme
{
    /** Keycode definitions */
    enum class Keycode
    {
        Esc,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        none
    };

    /** Count of the keycode */
    constexpr size_t NUM_KEY_CODES = static_cast<size_t>(Keycode::none);

#define KILLME_KEY_CASE(key) case Keycode::key: return #key;

    /** Return the stringed Keycode */
    template <>
    inline std::string to<std::string, Keycode>(const Keycode& code)
    {
        switch (code)
        {
            KILLME_KEY_CASE(Esc);
            KILLME_KEY_CASE(A);
            KILLME_KEY_CASE(B);
            KILLME_KEY_CASE(C);
            KILLME_KEY_CASE(D);
            KILLME_KEY_CASE(E);
            KILLME_KEY_CASE(F);
            KILLME_KEY_CASE(G);
            KILLME_KEY_CASE(H);
            KILLME_KEY_CASE(I);
            KILLME_KEY_CASE(J);
            KILLME_KEY_CASE(K);
            KILLME_KEY_CASE(L);
            KILLME_KEY_CASE(M);
            KILLME_KEY_CASE(N);
            KILLME_KEY_CASE(O);
            KILLME_KEY_CASE(P);
            KILLME_KEY_CASE(Q);
            KILLME_KEY_CASE(R);
            KILLME_KEY_CASE(S);
            KILLME_KEY_CASE(T);
            KILLME_KEY_CASE(U);
            KILLME_KEY_CASE(V);
            KILLME_KEY_CASE(W);
            KILLME_KEY_CASE(X);
            KILLME_KEY_CASE(Y);
            KILLME_KEY_CASE(Z);
            KILLME_KEY_CASE(none);

        default:
            assert(false && "Item not found.");
        }

        return ""; // For warnings
    }

#undef KILLME_KEY_CASE
}

#endif