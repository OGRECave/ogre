/* Copyright 2026 Google LLC
Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at
      http://www.apache.org/licenses/LICENSE-2.0
Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
*/

/*
 * Fuzzer for Ogre's script compiler pipeline.
 *
 * Feeds the input to ScriptCompilerManager as a generic ".os" script. Ogre
 * does not partition its script grammar by file extension - any top-level
 * keyword (material, particle_system, compositor, vertex_program, ...) is
 * accepted from any script - so a single target reaches every translator:
 *
 *   - material / technique / pass / texture_unit / sampler
 *   - particle_system / emitter / affector
 *   - compositor / technique / target / target_output
 *   - vertex_program / fragment_program / shared_params
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <memory>
#include <string>

#include "OgreTextureManager.h"
#include "OgreCompositorManager.h"
#include "OgreDataStream.h"
#include "OgreException.h"
#include "OgreLogManager.h"
#include "OgreMaterialManager.h"
#include "OgreParticleSystemManager.h"
#include "OgreResourceGroupManager.h"
#include "OgreRoot.h"
#include "OgreScriptCompiler.h"

static const char *const FUZZ_GROUP = "FuzzGroup";
static const size_t FUZZ_MAX_INPUT = 64 * 1024;

/* Swallow diagnostics, and keep `import` off the filesystem: returning a
 * non-null (empty) list short-circuits ScriptCompiler::loadImportPath(). */
class FuzzScriptListener : public Ogre::ScriptCompilerListener
{
public:
    void handleError(Ogre::ScriptCompiler*, Ogre::uint32, const Ogre::String&, int, const Ogre::String&) override {}

    Ogre::ConcreteNodeListPtr importFile(Ogre::ScriptCompiler*, const Ogre::String&) override
    {
        return std::make_shared<Ogre::ConcreteNodeList>();
    }
};

static bool g_initialized = false;

static void global_init() {
  if (g_initialized)
    return;

  auto *logMgr = new Ogre::LogManager();
  logMgr->createLog("fuzz.log", true, false);
  logMgr->setMinLogLevel(Ogre::LML_CRITICAL);

  new Ogre::Root("", "", "");

  /* Root::initialise() would do this, but that needs a RenderSystem. */
  Ogre::MaterialManager::getSingleton().initialise();
  new Ogre::DefaultTextureManager();

  static FuzzScriptListener listener;
  Ogre::ScriptCompilerManager::getSingleton().setListener(&listener);

  Ogre::ResourceGroupManager::getSingleton().createResourceGroup(FUZZ_GROUP, false);
  g_initialized = true;
}

/* Keep iterations independent, and keep RSS flat. */
static void purge_state() {
  auto &rgm = Ogre::ResourceGroupManager::getSingleton();
  rgm.destroyResourceGroup(FUZZ_GROUP);
  rgm.createResourceGroup(FUZZ_GROUP, false);

  /* Particle system templates are not Resources - they live in a private map
   * in ParticleSystemManager and would grow without bound otherwise. */
  Ogre::ParticleSystemManager::getSingleton().removeAllTemplates();
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
  if (size == 0 || size > FUZZ_MAX_INPUT)
    return 0;

  global_init();

  Ogre::DataStreamPtr stream(new Ogre::MemoryDataStream("fuzz.os", const_cast<uint8_t*>(data), size, false, true));

  try
  {
    Ogre::ScriptCompilerManager::getSingleton().parseScript(stream, FUZZ_GROUP);
  }
  catch (std::exception&)
  {
  }

  purge_state();
  return 0;
}