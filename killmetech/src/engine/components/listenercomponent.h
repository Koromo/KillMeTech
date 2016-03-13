#ifndef _KILLME_LISTENERCOMPONENT_H_
#define _KILLME_LISTENERCOMPONENT_H_

#include "transformcomponent.h"
#include <memory>

namespace killme
{
    class AudioListener;

    /** The listener component defines listening audio into an actor */
    class ListenerComponent : public TransformComponent
    {
        KILLME_COMPONENT_DEFINE_BEGIN(ListenerComponent)
        KILLME_COMPONENT_DEFINE_END

    private:
        std::shared_ptr<AudioListener> listener_;
        bool isMainListener_;

    public:
        /** Construct */
        ListenerComponent();

        /** If true, set this listener as the main listener */
        void setEnable(bool enable);

    private:
        void onTranslated();
        void onRotated();

        void onActivate();
        void onDeactivate();
    };
}

#endif