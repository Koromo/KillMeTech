#ifndef _KILLME_TREE_H_
#define _KILLME_TREE_H_

#include "utility.h"
#include <vector>
#include <algorithm>
#include <utility>

namespace killme
{
    /** Node */
    template <class T>
    class TreeNode
    {
    private:
        std::vector<T> children_;

    public:
        virtual ~TreeNode() = default;

        /** Add child */
        template <class... Args>
        void addChild(Args&&... args)
        {
            children_.emplace_back(std::forward<Args>(args)...);
        }

        /** Erase child */
        template <class U>
        void eraseChild(U&& child)
        {
            const auto begin = std::cbegin(children_);
            const auto end = std::cend(children_);
            const auto it = std::find(begin, end, child);
            if (it != end)
            {
                children_.erase(it);
            }
        }

        /** Return children */
        auto getChildren()
            -> decltype(makeRange(children_))
        {
            return makeRange(children_);
        }
    };
}

#endif