#ifndef _KILLME_LEVELMANAGER_H_
#define _KILLME_LEVELMANAGER_H_

#include <memory>

namespace killme
{
    class LevelDesigner;
    class BrainHacker;

    class LevelManager
    {
    private:
        std::shared_ptr<LevelDesigner> designer_;

    public:
        /** Initializes */
        void startup();

        /** Finalizes */
        void shutdown();

        /** Loads new level */
        void load(const std::shared_ptr<LevelDesigner>& designer);
    };

    extern LevelManager levelManager;
}

#endif