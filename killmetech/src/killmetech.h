#ifndef _KILLME_KILLMETECH_H_
#define _KILLME_KILLMETECH_H_

#include "engine/actor.h"
#include "engine/actordesigner.h"
#include "engine/leveldesigner.h"
#include "engine/world.h"
#include "engine/runtime.h"
#include "engine/processes.h"
#include "engine/audio.h"
#include "engine/physics.h"
#include "engine/graphics.h"
#include "engine/events.h"
#include "engine/inputs.h"
#include "engine/resources.h"
#include "engine/components/actorcomponent.h"
#include "engine/components/transformcomponent.h"
#include "engine/components/audioemittercomponent.h"
#include "engine/components/audiolistenercomponent.h"
#include "engine/components/cameracomponent.h"
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

#include "audio/audio3d.h"
#include "audio/audioclip.h"
#include "audio/audioengine.h"
#include "audio/sourcevoice.h"
#include "audio/xaudiosupport.h"

#include "hid/inputstatus.h"
#include "hid/keycode.h"

#include "scene/camera.h"
#include "scene/material.h"
#include "scene/mesh.h"
#include "scene/meshentity.h"
#include "scene/renderqueue.h"
#include "scene/scene.h"
#include "scene/sceneentity.h"
#include "scene/scenenode.h"
#include "scene/debugdrawmanager.h"

#include "import/fbxmeshimporter.h"
#include "import/fbxsupport.h"

#include "renderer/commandlist.h"
#include "renderer/constantbuffer.h"
#include "renderer/d3dsupport.h"
#include "renderer/depthstencil.h"
#include "renderer/gpuresourceheap.h"
#include "renderer/inputlayout.h"
#include "renderer/pipelinestate.h"
#include "renderer/pixelshader.h"
#include "renderer/renderstate.h"
#include "renderer/rendersystem.h"
#include "renderer/rendertarget.h"
#include "renderer/rootsignature.h"
#include "renderer/shader.h"
#include "renderer/vertexdata.h"
#include "renderer/vertexshader.h"

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

#ifdef KILLME_USING_NAMESPACE
using namespace killme;
#endif

#endif