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
        KILLME_COMPONENT_DEFINE(ListenerComponent)

    private:
        std::shared_ptr<AudioListener> listener_;
        bool setToMainListener_;

    public:
        /** Construct */
        ListenerComponent();

        /** Set this listener as the main listener */
        void enable();

        /** Set this listener as the non main listener */
        void disable();

        void onTranslated();
        void onRotated();

        void onActivate();
        void onDeactivate();
    };
}

#endif