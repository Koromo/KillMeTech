#ifndef _KILLME_SCENEVISITOR_H_
#define _KILLME_SCENEVISITOR_H_

#include <memory>

namespace killme
{
    class SceneNode;

    namespace detail
    {
        template <class... Args>
        struct BasicVisitor;

        template <>
        struct BasicVisitor<>
        {
            virtual ~BasicVisitor() = default;
            virtual bool operator ()(const std::shared_ptr<SceneNode>&, std::nullptr_t) { return false; }
        };

        template <class T, class... Args>
        struct BasicVisitor<T, Args...> : BasicVisitor<Args...>
        {
            using BasicVisitor<Args...>::operator();
            virtual bool operator ()(const std::shared_ptr<SceneNode>&, const std::shared_ptr<T>&) { return false; }
        };
    }

    class Camera;
    class MeshEntity;

    /** The basic visitor interface */
    class SceneVisitor : public detail::BasicVisitor<
        Camera,
        MeshEntity
    >
    {
    };
}

#endif