#include "keycode.h"
#include <cassert>

// For toString()
#define TO_STRING(key) case KeyCode::key: return #key;

namespace killme
{
    std::string toString(KeyCode code)
    {
        switch (code)
        {
            TO_STRING(Esc);
            TO_STRING(A);
            TO_STRING(B);
            TO_STRING(C);
            TO_STRING(D);
            TO_STRING(E);
            TO_STRING(F);
            TO_STRING(G);
            TO_STRING(H);
            TO_STRING(I);
            TO_STRING(J);
            TO_STRING(K);
            TO_STRING(L);
            TO_STRING(M);
            TO_STRING(N);
            TO_STRING(O);
            TO_STRING(P);
            TO_STRING(Q);
            TO_STRING(R);
            TO_STRING(S);
            TO_STRING(T);
            TO_STRING(U);
            TO_STRING(V);
            TO_STRING(W);
            TO_STRING(X);
            TO_STRING(Y);
            TO_STRING(Z);

        default:
            assert(false && "Item not found.");
        }

        return "";
    }
}