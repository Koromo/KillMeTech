#include "actor.h"
#include "level.h"
#include "components/actorcomponent.h"
#include "components/transformcomponent.h"
#include <stack>
#include <cassert>

namespace killme
{
    Actor::Actor()
        : inLevel_()
        , name_()
        , componentMap_()
        , conceptComponents_()
        , rootTransform_()
        , enableTicking_(true)
        , tickProcess_()
    {
    }

    void Actor::setOwnerLevel(Level* inLevel, const std::string& name)
    {
        inLevel_ = inLevel;
        name_ = name;
    }

    Level& Actor::getOwnerLevel()
    {
        assert(inLevel_ && "No owner level.");
        return *inLevel_;
    }

    std::string Actor::getName() const
    {
        return name_;
    }

    void Actor::activate()
    {
        if (enableTicking_)
        {
            tickProcess_ = getOwnerLevel().registerTickingActor(*this);
        }

        for (const auto& c : conceptComponents_)
        {
            c->setOwnerActor(this);
            c->activate();
            componentMap_.emplace(c->getComponentType(), c);
        }

        if (!rootTransform_)
        {
            return;
        }

        std::stack<std::shared_ptr<TransformComponent>> stack;
        stack.emplace(rootTransform_);
        
        while (!stack.empty())
        {
            const auto top = stack.top();
            stack.pop();

            top->setOwnerActor(this);
            top->activate();
            componentMap_.emplace(top->getComponentType(), top);

            for (const auto& child : top->getChildren())
            {
                stack.emplace(std::static_pointer_cast<TransformComponent>(child));
            }
        }
    }

    void Actor::deactivate()
    {
        for (const auto& c : conceptComponents_)
        {
            c->deactivate();
            c->setOwnerActor(nullptr);
        }

        if (rootTransform_)
        {
            std::stack<std::shared_ptr<TransformComponent>> stack;
            stack.emplace(rootTransform_);

            while (!stack.empty())
            {
                const auto top = stack.top();
                stack.pop();

                top->deactivate();
                top->setOwnerActor(nullptr);

                for (const auto& child : top->getChildren())
                {
                    stack.emplace(std::static_pointer_cast<TransformComponent>(child));
                }
            }
        }

        rootTransform_.reset();
        conceptComponents_.clear();
        componentMap_.clear();
        tickProcess_.kill();
    }

    void Actor::kill()
    {
        if (inLevel_)
        {
            killActor(*inLevel_, name_);
        }
    }

    std::shared_ptr<TransformComponent> Actor::getRootTransform()
    {
        return rootTransform_;
    }

    void Actor::disableTicking()
    {
        enableTicking_ = false;
    }
}