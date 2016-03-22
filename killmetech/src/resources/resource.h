#ifndef _KILLME_RESOURCE_H_
#define _KILLME_RESOURCE_H_

#include "resourcemanager.h"
#include <memory>
#include <string>
#include <functional>
#include <cassert>

namespace killme
{
    /** For resource manegement */
    class IsResource
    {
    public:
        virtual ~IsResource() = default;

    protected:
        IsResource() = default;
    };

    /** Resource accessor */
    /// NOTE: Not support const resources
    template <class T>
    class Resource
    {
    private:
        struct Holder
        {
            virtual ~Holder() = default;
            virtual std::unique_ptr<Holder> copy() const = 0;
            virtual std::shared_ptr<T> access() const = 0;
            virtual void load() const = 0;
            virtual void unload() const = 0;
        };

        // For media resources
        struct MediaHolder : Holder
        {
            std::weak_ptr<ResourceStore> store_;
            std::string path_;
            mutable std::weak_ptr<T> resource_;

            MediaHolder(const std::weak_ptr<ResourceStore>& store, const std::string& path)
                : store_(store)
                , path_(path)
                , resource_()
            {
            }

            std::unique_ptr<Holder> copy() const
            {
                return std::make_unique<MediaHolder>(store_, path_);
            }

            std::shared_ptr<T> access() const
            {
                if (resource_.expired())
                {
                    if (const auto r = store_.lock()->getLoadedResource(path_))
                    {
                        resource_ = std::dynamic_pointer_cast<T>(r);
                        assert(resource_.lock() && "Mismatch resource type.");
                    }
                    else
                    {
                        load();
                    }
                }
                return resource_.lock();
            }

            void load() const
            {
                if (const auto s = store_.lock())
                {
                    resource_ = std::dynamic_pointer_cast<T>(s->load(path_));
                    assert(resource_.lock() && "Mismatch resource type.");
                }
            }

            void unload() const
            {
                if (const auto s = store_.lock())
                {
                    s->unload(path_);
                }
            }
        };

        // For application resources
        struct AppHolder : Holder
        {
            using Loader = std::function<std::shared_ptr<T>()>;

            mutable Loader loader_;
            mutable std::shared_ptr<T> resource_;

            AppHolder(Loader loader, const std::shared_ptr<T>& resource)
                : loader_(loader)
                , resource_(resource)
            {
            }

            std::unique_ptr<Holder> copy() const
            {
                return std::make_unique<AppHolder>(loader_, resource_);
            }

            std::shared_ptr<T> access() const
            {
                if (!resource_)
                {
                    load();
                }
                return resource_;
            }

            void load() const
            {
                resource_ = loader_();
            }

            void unload() const
            {
                resource_.reset();
            }
        };

        std::unique_ptr<Holder> holder_;

    public:
        /** Construct */
        Resource() = default;

        /** Construct as the media resource */
        Resource(ResourceManager& mng, const std::string& path)
            : holder_(std::make_unique<MediaHolder>(mng.getStore(), path))
        {
        }

        /** Construct as the application resource */
        explicit Resource(typename AppHolder::Loader loader)
            : holder_(std::make_unique<AppHolder>(loader, loader()))
        {
        }

        /** Copy constructor */
        Resource(const Resource& lhs)
            : holder_()
        {
            *this = lhs;
        }

        /** Move constructor */
        Resource(Resource&& rhs) = default;

        /** Copy assignment operator */
        Resource& operator=(const Resource& lhs)
        {
            holder_.reset();
            if (lhs.holder_)
            {
                holder_ = lhs.holder_->copy();
            }
            return *this;
        }

        /** Move assignment operator */
        Resource& operator=(Resource&& rhs) = default;

        /** Accesse resource. If resource is not loaded, load resource immediately. */
        std::shared_ptr<T> access() const
        {
            return holder_->access();
        }

        /** Load resource */
        std::shared_ptr<T> load() const
        {
            holder_->load();
            return holder_->access();
        }

        /** Unload resource */
        void unload() const
        {
            return holder_->unload();
        }

        /** Return whether bound a resource or not */
        bool bound() const
        {
            return !!holder_;
        }
    };
}

#endif
