#ifndef _KILLME_KEYCODE_H_
#define _KILLME_KEYCODE_H_

#include <string>

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
    std::string toString(KeyCode code);
}

#endif