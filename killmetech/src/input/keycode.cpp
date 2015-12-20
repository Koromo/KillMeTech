#include "keycode.h"
#include <cassert>

// For toString()
#define TO_STR(key) case KeyCode::key: return #key;

namespace killme
{
    std::string toString(KeyCode code)
    {
        switch (code)
        {
            TO_STR(Esc);
            TO_STR(A);
            TO_STR(B);
            TO_STR(C);
            TO_STR(D);
            TO_STR(E);
            TO_STR(F);
            TO_STR(G);
            TO_STR(H);
            TO_STR(I);
            TO_STR(J);
            TO_STR(K);
            TO_STR(L);
            TO_STR(M);
            TO_STR(N);
            TO_STR(O);
            TO_STR(P);
            TO_STR(Q);
            TO_STR(R);
            TO_STR(S);
            TO_STR(T);
            TO_STR(U);
            TO_STR(V);
            TO_STR(W);
            TO_STR(X);
            TO_STR(Y);
            TO_STR(Z);

        default:
            assert(false && "Item not found.");
        }

        return "";
    }
}