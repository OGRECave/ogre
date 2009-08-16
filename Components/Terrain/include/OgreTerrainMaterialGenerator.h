/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2009 Torus Knot Software Ltd
Also see acknowledgements in Readme.html

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the Free Software
Foundation; either version 2 of the License, or (at your option) any later
version.

This program is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License along with
this program; if not, write to the Free Software Foundation, Inc., 59 Temple
Place - Suite 330, Boston, MA 02111-1307, USA, or go to
http://www.gnu.org/copyleft/lesser.txt.

You may alternatively use this source under the terms of a specific version of
the OGRE Unrestricted License provided you have obtained such a license from
Torus Knot Software Ltd.
-----------------------------------------------------------------------------
*/

#ifndef __Ogre_TerrainMaterialGenerator_H__
#define __Ogre_TerrainMaterialGenerator_H__

#include "OgreTerrainPrerequisites.h"
#include "OgrePixelFormat.h"
#include "OgreMaterial.h"

namespace Ogre
{
	class Terrain;

	/** \addtogroup Optional Components
	*  @{
	*/
	/** \addtogroup Terrain
	*  Some details on the terrain component
	*  @{
	*/


	/** Enumeration of types of data that can be read from textures that are
	specific to a given layer. Notice that global texture information 
	such as shadows and terrain normals are not represented
	here because they are not a per-layer attribute, and blending
	is stored in packed texture structures which are stored separately.
	*/
	enum TerrainLayerSamplerSemantic
	{
		/// Albedo colour (diffuse reflectance colour)
		TLSS_ALBEDO = 0,
		/// Tangent-space normal information from a detail texture
		TLSS_NORMAL = 1,
		/// Height information for the detail texture
		TLSS_HEIGHT = 2,
		/// Specular reflectance
		TLSS_SPECULAR = 3
	};

	/** Information about one element of a sampler / texture within a layer. 
	*/
	struct _OgreTerrainExport TerrainLayerSamplerElement
	{
		/// The source sampler index of this element relative to LayerDeclaration's list
		uint8 source;
		/// The semantic this element represents
		TerrainLayerSamplerSemantic semantic;
		/// The colour element at which this element starts
		uint8 elementStart;
		/// The number of colour elements this semantic uses (usually standard per semantic)
		uint8 elementCount;

		bool operator==(const TerrainLayerSamplerElement& e) const
		{
			return source == e.source &&
				semantic == e.semantic &&
				elementStart == e.elementStart &&
				elementCount == e.elementCount;
		}

		TerrainLayerSamplerElement() {}
		TerrainLayerSamplerElement(uint8 src, TerrainLayerSamplerSemantic sem,
			uint8 elemStart, uint8 elemCount)
			: source(src), semantic(sem), elementStart(elemStart), elementCount(elemCount)
		{
		}
	};
	typedef vector<TerrainLayerSamplerElement>::type TerrainLayerSamplerElementList;

	/** Description of a sampler that will be used with each layer. 
	*/
	struct _OgreTerrainExport TerrainLayerSampler
	{
		/// A descriptive name that is merely used to assist in recognition
		String alias;
		/// The format required of this texture
		PixelFormat format;

		bool operator==(const TerrainLayerSampler& s) const
		{
			return alias == s.alias && format == s.format;
		}

		TerrainLayerSampler() {}

		TerrainLayerSampler(const String& aliasName, PixelFormat fmt)
			: alias(aliasName), format(fmt)
		{
		}
	};
	typedef vector<TerrainLayerSampler>::type TerrainLayerSamplerList;

	/** The definition of the information each layer will contain in this terrain.
	All layers must contain the same structure of information, although the
	input textures can be different per layer instance. 
	*/
	struct _OgreTerrainExport TerrainLayerDeclaration
	{
		TerrainLayerSamplerList samplers;
		TerrainLayerSamplerElementList elements;

		bool operator==(const TerrainLayerDeclaration& dcl) const
		{
			return samplers == dcl.samplers && elements == dcl.elements;
		}
	};

	/** Class that provides functionality to generate materials for use with a terrain.
	@remarks
		Terrains are composed of one or more layers of texture information, and
		require that a material is generated to render them. There are various approaches
		to rendering the terrain, which may vary due to:
		<ul><li>Hardware support (static)</li>
		<li>Texture instances assigned to a particular terrain (dynamic in an editor)</li>
		<li>User selection (e.g. changing to a cheaper option in order to increase performance, 
		or in order to test how the material might look on other hardware (dynamic)</li>
		</ul>
		Subclasses of this class are responsible for responding to these factors and
		to generate a terrain material. 
		@par
		In order to cope with both hardware support and user selection, the generator
		must expose a number of named 'profiles'. These profiles should function on
		a known range of hardware, and be graded by quality. At runtime, the user 
		should be able to select the profile they wish to use (provided hardware
		support is available). 
	*/
	class _OgreTerrainExport TerrainMaterialGenerator : public TerrainAlloc
	{
	public:
		/** Inner class which should also be subclassed to provide profile-specific 
			material generation.
		*/
		class _OgreTerrainExport Profile : public TerrainAlloc
		{
		protected:
			TerrainMaterialGenerator* mParent;
			String mName;
			String mDesc;
		public:
			Profile(TerrainMaterialGenerator* parent, const String& name, const String& desc)
				: mParent(parent), mName(name), mDesc(desc) {}
			Profile(const Profile& prof) 
				: mParent(prof.mParent), mName(prof.mName), mDesc(prof.mDesc) {}
			virtual ~Profile() {}
			/// Get the generator which owns this profile
			TerrainMaterialGenerator* getParent() const { return mParent; }
			/// Get the name of this profile
			const String& getName() const { return mName; }
			/// Get the description of this profile
			const String& getDescription() const { return mDesc; }
			
			/// Generate / resuse a material for the terrain
			virtual MaterialPtr generate(const Terrain* terrain) = 0;
			/// Get the number of layers supported
			virtual uint8 getMaxLayers(const Terrain* terrain) const = 0;
			/// Update the composite map for a terrain
			virtual void updateCompositeMap(const Terrain* terrain, const Rect& rect) = 0;

			/// Update params for a terrain
			virtual void updateParams(const MaterialPtr& mat, const Terrain* terrain) = 0;

			/// Request the options needed from the terrain
			virtual void requestOptions(Terrain* terrain) = 0;

		};

		TerrainMaterialGenerator() 
			: mActiveProfile(0), mChangeCounter(0), mDebugLevel(0) {}
		virtual ~TerrainMaterialGenerator()
		{
			for (ProfileList::iterator i = mProfiles.begin(); i != mProfiles.end(); ++i)
				OGRE_DELETE *i;
		}

		/// List of profiles - NB should be ordered in descending complexity
		typedef vector<Profile*>::type ProfileList;
	
		/** Get the list of profiles that this generator supports.
		*/
		virtual const ProfileList& getProfiles() const { return mProfiles; }

		/** Set the active profile by name. */
		virtual void setActiveProfile(const String& name)
		{
			if (!mActiveProfile || mActiveProfile->getName() != name)
			{
				for (ProfileList::iterator i = mProfiles.begin(); i != mProfiles.end(); ++i)
				{
					if ((*i)->getName() == name)
					{
						setActiveProfile(*i);
						break;
					}
				}
			}

		}

		/** Set the active Profile. */
		virtual void setActiveProfile(Profile* p)
		{
			if (mActiveProfile != p)
			{
				mActiveProfile = p;
				_markChanged();
			}
		}
		/// Get the active profile
		Profile* getActiveProfile() const 
		{ 
			// default if not chosen yet
			if (!mActiveProfile && !mProfiles.empty())
				mActiveProfile = mProfiles[0];

			return mActiveProfile; 
		}

		/// Internal method - indicates that a change has been made that would require material regeneration
		void _markChanged() { ++mChangeCounter; }

		/** Returns the number of times the generator has undergone a change which 
			would require materials to be regenerated.
		*/
		unsigned long long int getChangeCount() const { return mChangeCounter; }

		/** Get the layer declaration that this material generator operates with.
		*/
		virtual const TerrainLayerDeclaration& getLayerDeclaration() const { return mLayerDecl; }
		/** Whether this generator can generate a material for a given declaration. 
			By default this only returns true if the declaration is equal to the 
			standard one returned from getLayerDeclaration, but if a subclass wants
			to be flexible to generate materials for other declarations too, it 
			can specify here. 
		*/
		virtual bool canGenerateUsingDeclaration(const TerrainLayerDeclaration& decl)
		{
			return decl == mLayerDecl;
		}

		/** Triggers the generator to request the options that it needs.
		*/
		virtual void requestOptions(Terrain* terrain)
		{
			Profile* p = getActiveProfile();
			if (p)
				p->requestOptions(terrain);

		}
		/** Generate a material for the given terrain using the active profile.
		*/
		virtual MaterialPtr generate(const Terrain* terrain)
		{
			Profile* p = getActiveProfile();
			if (!p)
				return MaterialPtr();
			else
				return p->generate(terrain);
		}
		/** Get the maximum number of layers supported with the given terrain. 
		@note When you change the options on the terrain, this value can change. 
		*/
		virtual uint8 getMaxLayers(const Terrain* terrain) const
		{
			Profile* p = getActiveProfile();
			if (p)
				return p->getMaxLayers(terrain);
			else
				return 0;
		}

		/** Update the composite map for a terrain.
		The composite map for a terrain must match what the terrain should look like
		at distance. This method will only be called in the render thread so the
		generator is free to render into a texture to support this, so long as 
		the results are blitted into the Terrain's own composite map afterwards.
		*/
		virtual void updateCompositeMap(const Terrain* terrain, const Rect& rect)
		{
			Profile* p = getActiveProfile();
			if (!p)
				return;
			else
				p->updateCompositeMap(terrain, rect);
		}


		/** Update parameters for the given terrain using the active profile.
		*/
		virtual void updateParams(const MaterialPtr& mat, const Terrain* terrain)
		{
			Profile* p = getActiveProfile();
			if (p)
				p->updateParams(mat, terrain);
		}

		/** Set the debug level of the material. 
		@remarks
			Sets the level of debug display for this material.
			What this debug level means is entirely depdendent on the generator, 
			the only constant is that 0 means 'no debug' and non-zero means 
			'some level of debugging', with any graduations in non-zero values
			being generator-specific.
		*/
		virtual void setDebugLevel(unsigned int dbg)
		{
			if (mDebugLevel != dbg)
			{
				mDebugLevel = dbg;
				_markChanged();
			}
		}
		/// Get the debug level of the material. 
		virtual unsigned int getDebugLevel() const { return mDebugLevel; }
	protected:

		ProfileList mProfiles;
		mutable Profile* mActiveProfile;
		unsigned long long int mChangeCounter;
		TerrainLayerDeclaration mLayerDecl;
		unsigned int mDebugLevel;



	};

	typedef SharedPtr<TerrainMaterialGenerator> TerrainMaterialGeneratorPtr;

	/** @} */
	/** @} */

}
#endif

