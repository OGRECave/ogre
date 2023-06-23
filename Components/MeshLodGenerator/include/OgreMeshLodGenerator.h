/*
 * -----------------------------------------------------------------------------
 * This source file is part of OGRE
 * (Object-oriented Graphics Rendering Engine)
 * For the latest info, see http://www.ogre3d.org/
 *
 * Copyright (c) 2000-2014 Torus Knot Software Ltd
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * -----------------------------------------------------------------------------
 */

#ifndef __MeshLodGenerator_H_
#define __MeshLodGenerator_H_

#include "OgreLodPrerequisites.h"
#include "OgreLodConfig.h"
#include "OgreLodData.h"
#include "OgreLodInputProvider.h"
#include "OgreLodOutputProvider.h"
#include "OgreLodCollapseCost.h"
#include "OgreLodCollapser.h"
#include "OgreSharedPtr.h"
#include "OgreSingleton.h"
#include "OgreWorkQueue.h"

namespace Ogre
{

/** \addtogroup Optional
*  @{
*/
/** \defgroup MeshLodGenerator MeshLodGenerator
* Generate Low-poly models from High-poly models automatically
*  @{
*/

class _OgreLodExport MeshLodGenerator :
public Singleton<MeshLodGenerator>
{
public:

    static MeshLodGenerator* getSingletonPtr();
    static MeshLodGenerator& getSingleton();

    /**
     * @brief Generates the Lod levels for a mesh.
     */
    MeshLodGenerator();
    virtual ~MeshLodGenerator();

    /**
     * @brief Generates the Lod levels for a mesh.
     */
    virtual void generateLodLevels(LodConfig& lodConfig, LodCollapseCostPtr cost = LodCollapseCostPtr(), LodDataPtr data = LodDataPtr(), LodInputProviderPtr input = LodInputProviderPtr(), LodOutputProviderPtr output = LodOutputProviderPtr(), LodCollapserPtr collapser = LodCollapserPtr());

    /**
     * @brief Generates the Lod levels for a mesh without configuring it.
     *
     * @param mesh Generate the Lod for this mesh.
     */
    void generateAutoconfiguredLodLevels(MeshPtr& mesh);

    /**
     * @brief Fills Lod Config with a config, which works on any mesh.
     *
     * @param inMesh Optimize for this mesh.
     * @param outLodConfig Lod configuration storing the output.
     */
    void getAutoconfig(MeshPtr& inMesh, LodConfig& outLodConfig);

    void clearPendingLodRequests();

    static void _configureMeshLodUsage(const LodConfig& lodConfig);
    void _resolveComponents(LodConfig& lodConfig, LodCollapseCostPtr& cost, LodDataPtr& data, LodInputProviderPtr& input, LodOutputProviderPtr& output, LodCollapserPtr& collapser);

    /// If you only use manual Lod levels, then you don't need to build LodData mesh representation.
    /// This function will generate manual Lod levels without overhead, but every Lod level needs to be a manual Lod level.
    void _generateManualLodLevels(LodConfig& lodConfig);

    void setInjectorListener(LodWorkQueueInjectorListener* injectorListener) {mInjectorListener = injectorListener;}
    LodWorkQueueInjectorListener* getInjectorListener() {return mInjectorListener;}
    void removeInjectorListener() {mInjectorListener = 0;}
private:
    void _process(LodConfig& lodConfig, LodCollapseCost* cost, LodData* data, LodInputProvider* input, LodOutputProvider* output, LodCollapser* collapser);

    void computeLods(LodConfig& lodConfig, LodData* data, LodCollapseCost* cost, LodOutputProvider* output, LodCollapser* collapser);
    void calcLodVertexCount(const LodLevel& lodLevel, size_t uniqueVertexCount, size_t& outVertexCountLimit, Real& outCollapseCostLimit);

    OGRE_WQ_MUTEX(mQueueMutex);
    std::list<LodWorkQueueRequest*> mPendingLodRequests;

    LodWorkQueueInjectorListener* mInjectorListener;

    void addRequestToQueue(LodConfig& lodConfig, LodCollapseCostPtr& cost, LodDataPtr& data, LodInputProviderPtr& input, LodOutputProviderPtr& output, LodCollapserPtr& collapser);
    WorkQueue::Response* handleRequest(const WorkQueue::Request* req, const WorkQueue* srcQ);
    void handleResponse(const WorkQueue::Response* res, const WorkQueue* srcQ);
};
/** @} */
/** @} */
}
#endif
