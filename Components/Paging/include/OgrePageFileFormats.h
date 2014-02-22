/*
-----------------------------------------------------------------------------
This source file is part of OGRE
(Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __Ogre_PageFileFormats_H__
#define __Ogre_PageFileFormats_H__

#include "OgrePagingPrerequisites.h"

/** \addtogroup Optional Components
*  @{
*/
/** \addtogroup Paging
*  Some details on paging component
*/
/*@{*/


/** @file
    The paging file format is a composite one - a single file / stream can contain
    data which is not necessarily all read by a single class. Instead, data
    chunks can be read by different classes, allowing the format to be extended to
    different types smoothly.
    @par
    Paging world files have by default an extension ".world", but that is not
    a requirement. Internally, the only thing that matters is the data chunks
    and their identifiers, which are 4-character codes embedded in a uint32 as
    calculated by StreamSerialiser::makeIdentifier. All data will be read and 
    written using DataStream and the StreamSerialiser class. 
    @par
    Data types are expressed at the lowest level exposed by the StreamSerialiser class, 
    which is used to read / write this file. 
    @par
    <b>Chunk Definitions</b>
    @par
    <b>PagedWorld (Identifier 'PWLD')</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>World Name</td>
        <td>char*</td>
        <td>The name of the world - should be unique</td>
    </tr>
    <tr>
        <td>PagedWorldSection List</td>
        <td>Chunk List</td>
        <td>A variable-length list of nested PagedWorldSection chunks</td>
    </tr>
    </table>

    @par
    <b>PagedWorldSection (Identifier 'PWSC')</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>World Section Name</td>
        <td>char*</td>
        <td>The name of the world section - should be unique within world</td>
    </tr>
    <tr>
        <td>Bounding box</td>
        <td>AABB</td>
        <td>AABB of this world section in world space</td>
    </tr>
    <tr>
        <td>PageStrategy name</td>
        <td>char*</td>
        <td>The name of the PageStrategy class this world section uses to manage pages</td>
    </tr>
    <tr>
        <td>Page Strategy Data</td>
        <td>Nested Chunk</td>
        <td>PageStrategy specific data for this world section</td>
    </tr>
    </table>

    @par
    <b>PagedStrategyData (Identifier defined by subclass)</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>PageStrategy defined</td>
        <td>???</td>
        <td>This chunk will contain data as defined by the specific PageStrategyData used by the
            parent PagedWorldSection</td>
    </tr>
    </table>
    @par
    <b>Page (Identifier 'PAGE')</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>Page ID</td>
        <td>uint32</td>
        <td>The identifier of the page</td>
    </tr>
    <tr>
        <td>PageContentCollection list</td>
        <td>Nested chunk list</td>
        <td>1-n nested chunks of type PageContentCollection</td>
    </tr>
    </table>
    @par
    <b>PageContentCollection (Identifier 'PGCC')</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>Factory name</td>
        <td>char*</td>
        <td>The name of the content collection factory</td>
    </tr>
    <tr>
        <td>PageContentCollectionData chunk</td>
        <td>Nested chunk</td>
        <td>Data required to initialise & configure the content collection</td>
    </tr>
    </table>
    @par
    <b>PageContentCollectionData (Identifier defined by subclass)</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>PageContentCollection subclass defined</td>
        <td>???</td>
        <td>This chunk will contain data as defined by the specific PageContentCollection subclass</td>
    </tr>
    </table>
    @par
    <b>PageContent (Identifier 'PGCN')</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>Factory name</td>
        <td>char*</td>
        <td>The name of the content factory</td>
    </tr>
    <tr>
        <td>PageContentData chunk</td>
        <td>Nested chunk</td>
        <td>Data required to initialise & configure the content</td>
    </tr>
    </table>
    @par
    <b>PageContentData (Identifier defined by subclass)</b>\n
    [Version 1]
    <table>
    <tr>
        <td><b>Name</b></td>
        <td><b>Type</b></td>
        <td><b>Description</b></td>
    </tr>
    <tr>
        <td>PageContent subclass defined</td>
        <td>???</td>
        <td>This chunk will contain data as defined by the specific PageContent subclass</td>
    </tr>
    </table>
*/

/*@}*/
/*@}*/


#endif 