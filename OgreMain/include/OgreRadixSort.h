/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2006 Torus Knot Software Ltd
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
#ifndef __RadixSort_H__
#define __RadixSort_H__

#include "OgrePrerequisites.h"

namespace Ogre {

	/** Class for performing a radix sort (fast comparison-less sort based on 
		byte value) on various standard STL containers. 
	@remarks
		A radix sort is a very fast sort algorithm. It doesn't use comparisons
		and thus is able to break the theoretical minimum O(N*logN) complexity. 
		Radix sort is complexity O(k*N), where k is a constant. Note that radix
		sorting is not in-place, it requires additional storage, so it trades
		memory for speed. The overhead of copying means that it is only faster
		for fairly large datasets, so you are advised to only use it for collections
		of at least a few hundred items.
	@par
		This is a template class to allow it to deal with a variety of containers, 
		and a variety of value types to sort on. In addition to providing the
		container and value type on construction, you also need to supply a 
		functor object which will retrieve the value to compare on for each item
		in the list. For example, if you had an std::vector of by-value instances
		of an object of class 'Bibble', and you wanted to sort on 
		Bibble::getDoobrie(), you'd have to firstly create a functor
		like this:
	@code
		struct BibbleSortFunctor
		{
			float operator()(const Bibble& val) const
			{
				return val.getDoobrie();
			}
		}
	@endcode
		Then, you need to declare a RadixSort class which names the container type, 
		the value type in the container, and the type of the value you want to 
		sort by. You can then call the sort function. E.g.
	@code
		RadixSort<BibbleList, Bibble, float> radixSorter;
		BibbleSortFunctor functor;

		radixSorter.sort(myBibbleList, functor);
	@endcode
		You should try to reuse RadixSort instances, since repeated allocation of the 
		internal storage is then avoided.
	@note
		Radix sorting is often associated with just unsigned integer values. Our
		implementation can handle both unsigned and signed integers, as well as
		floats (which are often not supported by other radix sorters). doubles
		are not supported; you will need to implement your functor object to convert
		to float if you wish to use this sort routine.
	*/
	template <class TContainer, class TContainerValueType, typename TCompValueType>
	class RadixSort
	{
	public:
		typedef typename TContainer::iterator ContainerIter;
	protected:
		/// Alpha-pass counters of values (histogram)
		/// 4 of them so we can radix sort a maximum of a 32bit value
		int mCounters[4][256];
		/// Beta-pass offsets 
		int mOffsets[256];
		/// Sort area size
		int mSortSize;
		/// Number of passes for this type
		int mNumPasses;

		struct SortEntry
		{
			TCompValueType key;
			ContainerIter iter;
			SortEntry() {}
			SortEntry(TCompValueType k, ContainerIter it)
				: key(k), iter(it) {}

		};
		/// Temp sort storage
		std::vector<SortEntry> mSortArea1;
		std::vector<SortEntry> mSortArea2;
		std::vector<SortEntry>* mSrc;
		std::vector<SortEntry>* mDest;
		TContainer mTmpContainer; // initial copy


		void sortPass(int byteIndex)
		{
			// Calculate offsets
			// Basically this just leaves gaps for duplicate entries to fill
			mOffsets[0] = 0;
			for (int i = 1; i < 256; ++i)
			{
				mOffsets[i] = mOffsets[i-1] + mCounters[byteIndex][i-1];
			}

			// Sort pass
			for (int i = 0; i < mSortSize; ++i)
			{
				unsigned char byteVal = getByte(byteIndex, (*mSrc)[i].key);
				(*mDest)[mOffsets[byteVal]++] = (*mSrc)[i];
			}

		}
		template <typename T>
		void finalPass(int byteIndex, T val)
		{
			// default is to do normal pass
			sortPass(byteIndex);
		}
		
		// special case signed int
		void finalPass(int byteIndex, int val)
		{
			int numNeg = 0;
			// all negative values are in entries 128+ in most significant byte
			for (int i = 128; i < 256; ++i)
			{
				numNeg += mCounters[byteIndex][i];
			}
			// Calculate offsets - positive ones start at the number of negatives
			// do positive numbers
			mOffsets[0] = numNeg;
			for (int i = 1; i < 128; ++i)
			{
				mOffsets[i] = mOffsets[i-1] + mCounters[byteIndex][i-1];
			}
			// Do negative numbers (must start at zero)
			// No need to invert ordering, already correct (-1 is highest number)
			mOffsets[128] = 0;
			for (int i = 129; i < 256; ++i)
			{
				mOffsets[i] = mOffsets[i-1] + mCounters[byteIndex][i-1];
			}

			// Sort pass
			for (int i = 0; i < mSortSize; ++i)
			{
				unsigned char byteVal = getByte(byteIndex, (*mSrc)[i].key);
				(*mDest)[mOffsets[byteVal]++] = (*mSrc)[i];
			}
		}
		

		// special case float
		void finalPass(int byteIndex, float val)
		{
			// floats need to be special cased since negative numbers will come
			// after positives (high bit = sign) and will be in reverse order
			// (no ones-complement of the +ve value)
			int numNeg = 0;
			// all negative values are in entries 128+ in most significant byte
			for (int i = 128; i < 256; ++i)
			{
				numNeg += mCounters[byteIndex][i];
			}
			// Calculate offsets - positive ones start at the number of negatives
			// do positive numbers normally
			mOffsets[0] = numNeg;
			for (int i = 1; i < 128; ++i)
			{
				mOffsets[i] = mOffsets[i-1] + mCounters[byteIndex][i-1];
			}
			// Do negative numbers (must start at zero)
			// Also need to invert ordering
			// In order to preserve the stability of the sort (essential since
			// we rely on previous bytes already being sorted) we have to count
			// backwards in our offsets from 
			mOffsets[255] = mCounters[byteIndex][255];
			for (int i = 254; i > 127; --i)
			{
				mOffsets[i] = mOffsets[i+1] + mCounters[byteIndex][i];
			}

			// Sort pass
			for (int i = 0; i < mSortSize; ++i)
			{
				unsigned char byteVal = getByte(byteIndex, (*mSrc)[i].key);
				if (byteVal > 127)
				{
					// -ve; pre-decrement since offsets set to count
					(*mDest)[--mOffsets[byteVal]] = (*mSrc)[i];
				}
				else
				{
					// +ve
					(*mDest)[mOffsets[byteVal]++] = (*mSrc)[i];
				}
			}
		}

		inline unsigned char getByte(int byteIndex, TCompValueType val)
		{
#if OGRE_ENDIAN == OGRE_ENDIAN_LITTLE
			return ((unsigned char*)(&val))[byteIndex];
#else
			return ((unsigned char*)(&val))[mNumPasses - byteIndex - 1];
#endif
		}

	public:

		RadixSort() {}
		~RadixSort() {}

		/** Main sort function
		@param container A container of the type you declared when declaring
		@param func A functor which returns the value for comparison when given
			a container value
		*/
		template <class TFunction>
		void sort(TContainer& container, TFunction func)
		{
			if (container.empty())
				return;

			// Set up the sort areas
			mSortSize = static_cast<int>(container.size());
			mSortArea1.resize(container.size());
			mSortArea2.resize(container.size());

			// Copy data now (we need constant iterators for sorting)
			mTmpContainer = container;

			mNumPasses = sizeof(TCompValueType);

			// Counter pass
			// Initialise the counts
			int p;
			for (p = 0; p < mNumPasses; ++p)
				memset(mCounters[p], 0, sizeof(int) * 256);

			// Perform alpha pass to count
			ContainerIter i = mTmpContainer.begin();
			TCompValueType prevValue = func.operator()(*i); 
			bool needsSorting = false;
			for (int u = 0; i != mTmpContainer.end(); ++i, ++u)
			{
				// get sort value
				TCompValueType val = func.operator()(*i);
				// cheap check to see if needs sorting (temporal coherence)
				if (!needsSorting && val < prevValue)
					needsSorting = true;

				// Create a sort entry
				mSortArea1[u].key = val;
				mSortArea1[u].iter = i;

				// increase counters
				for (p = 0; p < mNumPasses; ++p)
				{
					unsigned char byteVal = getByte(p, val);
					mCounters[p][byteVal]++;
				}

				prevValue = val;

			}

			// early exit if already sorted
			if (!needsSorting)
				return;


			// Sort passes
			mSrc = &mSortArea1;
			mDest = &mSortArea2;

			for (p = 0; p < mNumPasses - 1; ++p)
			{
				sortPass(p);
				// flip src/dst
				std::vector<SortEntry>* tmp = mSrc;
				mSrc = mDest;
				mDest = tmp;
			}
			// Final pass may differ, make polymorphic
			finalPass(p, prevValue);

			// Copy everything back
			int c = 0;
			for (i = container.begin(); 
				i != container.end(); ++i, ++c)
			{
				*i = *((*mDest)[c].iter);
			}
		}

	};


}
#endif

