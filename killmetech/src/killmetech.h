#ifndef _KILLME_KILLMETECH_H_
#define _KILLME_KILLMETECH_H_

#include "engine/actor.h"
#include "engine/debug.h"
#include "engine/runtime.h"
#include "engine/audiosystem.h"
#include "engine/graphicssystem.h"
#include "engine/resourcemanagesystem.h"
#include "engine/inputmanager.h"
#include "engine/keycode.h"
#include "engine/level.h"
#include "engine/eventutil.h"
#include "engine/components/actorcomponent.h"
#include "engine/components/transformcomponent.h"
#include "engine/components/audiocomponent.h"
#include "engine/components/listenercomponent.h"
#include "engine/components/cameracomponent.h"
#include "engine/components/lightcomponent.h"
#include "engine/components/meshcomponent.h"
#include "engine/components/rigidbodycomponent.h"

#include "events/event.h"
#include "events/eventdispatcher.h"

#include "processes/process.h"
#include "processes/processscheduler.h"

#include "physics/bulletsupport.h"
#include "physics/collisionshape.h"
#include "physics/physicsworld.h"
#include "physics/rigidbody.h"

#include "audio/audioclip.h"
#include "audio/audiomanager.h"
#include "audio/audioworld.h"
#include "audio/sourcevoice.h"
#include "audio/xaudiosupport.h"

#include "scene/camera.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/materialcreation.h"
#include "scene/effecttechnique.h"
#include "scene/effectpass.h"
#include "scene/mesh.h"
#include "scene/meshinstance.h"
#include "scene/scene.h"
#include "scene/debugdrawmanager.h"

#include "import/fbxmeshimporter.h"
#include "import/fbxsupport.h"

#include "renderer/commandlist.h"
#include "renderer/constantbuffer.h"
#include "renderer/d3dsupport.h"
#include "renderer/depthstencil.h"
#include "renderer/gpuresourceheap.h"
#include "renderer/image.h"
#include "renderer/inputlayout.h"
#include "renderer/pipelinestate.h"
#include "renderer/renderstate.h"
#include "renderer/rendersystem.h"
#include "renderer/rendertarget.h"
#include "renderer/rootsignature.h"
#include "renderer/shaders.h"
#include "renderer/texture.h"
#include "renderer/vertexdata.h"

#include "resources/resource.h"
#include "resources/resourcemanager.h"

#include "windows/console.h"
#include "windows/winsupport.h"

#include "core/exception.h"
#include "core/optional.h"
#include "core/platform.h"
#include "core/string.h"
#include "core/utility.h"
#include "core/variant.h"
#include "core/math/color.h"
#include "core/math/math.h"
#include "core/math/matrix44.h"
#include "core/math/quaternion.h"
#include "core/math/transform.h"
#include "core/math/vector3.h"

#ifndef KILLME_NO_INCLUDE_NAMESPACE
using namespace killme;
#endif

#endif