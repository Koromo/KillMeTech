#ifndef _KILLME_KEYCODE_H_
#define _KILLME_KEYCODE_H_

#include <string>

namespace killme
{
    /** Key code definitions */
    enum class KeyCode
    {
        Esc,
        A, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
        none
    };

    /** KeyCode <-> string converter */
    std::string toString(KeyCode code);
    KeyCode toString(const std::string& str);
}

#endif