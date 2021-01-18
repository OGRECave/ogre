
# Licence Information


<p>OGRE is released under an <a href="http://www.opensource.org/">open source</a> license, specifically the MIT License.

<p>Under  the <a href="../LICENSE">MIT License</a> you may use Ogre for any purpose you wish, without warranty, and modify it if you require, subject to one condition:</p>
<ol>
    <li>"The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software."</li>
</ol>

In practice this means that whenever you distribute your application, whether as binary or as source code, you must include somewhere in your distribution the text <a href="../LICENSE">in this file</a>. This might be in the printed documentation, as a file on delivered media, or even on the credits / acknowledgements of the runtime application itself; any of those would satisfy the requirement.

The [OGRE Logo](ogre-logo.png) is licenced [CC-BY](https://creativecommons.org/licenses/by/2.0/) [Pierre Fontaine](http://codrer.com/).

# Licenses for 3rd-party libraries used in OGRE

Several 3rd-party libraries are used in OGRE for various purposes, and the licensing details of each are given here.

## Core Dependencies

These are the libraries which the core of OGRE depends on, and are
therefore prerequisites for any use of OGRE. None of these libraries
have been modified from their original distributions, and they are all
licensed under liberal terms. You should read the detail of the
licenses when distributing an application since most require you to
include the license in your documentation. All the license texts are
included in the Docs/licenses folder.

### Zlib

<table style="text-align: left; width: 100%;" border="0" cellpadding="0" cellspacing="0">
<tbody>

<tr>
<td style="width: 20%;">Original Authors:</td>
<td>Jean-loup Gailly and Mark Adler</td>
</tr>

<tr>
<td>Website:</td>
<td>http://www.zlib.net</td>
</tr>

<tr>
<td>Licensed Under:</td>
<td><a href="licenses/zlib.txt">Zlib License</a></td>
</tr>

</tbody>
</table>

### Zziplib

<table style="text-align: left; width: 100%;" border="0" cellpadding="0" cellspacing="0">
<tbody>

<tr>
<td style="width: 20%;">Original Author:</td>
<td>Guido Draheim</td>
</tr>

<tr>
<td>Website:</td>
<td>http://zziplib.sourceforge.net</td>
</tr>

<tr>
<td>Licensed Under:</td>
<td><a href="licenses/mpl.txt">Mozilla Public License 1.1</a></td>
</tr>

</tbody>
</table>

## Optional Dependencies
These dependencies are only needed if you use the plugins they relate to, or you enable them in the source build.

### SDL2
<table style="text-align: left; width: 100%" border="0" cellpadding="0" cellspacing="0">
<tbody>
<tr>
<td style="width: 20%;">
Original Authors:</td>
<td>
Many</td>
</tr>
<tr>
<td>
Website:</td>
<td>
https://www.libsdl.org/</td>
</tr>
<tr>
<td>
Licensed Under:</td>
<td>
<a href="licenses/zlib.txt">Zlib License</a></td>
</tr>
<tr>
<td>
Needed By:</td>
<td>
Bites Component</td>
</tr>
</tbody>
</table>

### NVidia Cg

<table style="text-align: left; width: 100%;" border="0" cellpadding="0" cellspacing="0">
<tbody>

<tr>
<td style="width: 20%;">Original Authors:</td>
<td>NVidia</td>
</tr>

<tr>
<td>Website:</td>
<td>http://developer.nvidia.com</td>
</tr>

<tr>
<td>Licensed Under:</td>
<td>Custom free license (binary only)</td>
</tr>

<tr>
<td>Needed By:</td>
<td>Plugin_CgProgramManager</td>
</tr>

</tbody>
</table>

### STB Image

<table style="text-align: left; width: 100%;" border="0" cellpadding="0" cellspacing="0">
<tbody>

<tr>
    <td style="width: 20%;">Original Authors:</td>
    <td>Sean Barrett<br />
    </td>
</tr>

<tr>
    <td>Website:</td>
    <td>https://github.com/nothings/stb<br />
    </td>
</tr>

<tr>
    <td>Licensed Under:</td>
    <td>public domain</td>
</tr>

<tr>
<td>Needed By:</td>
<td>Codec_STBI</td>
</tr>

</tbody>
</table>

### OpenEXR

<table style="text-align: left; width: 100%;" border="0" cellpadding="0" cellspacing="0">
<tbody>

<tr>
<td style="width: 20%;">Original Authors:</td>
<td>Industrial Light and Magic</td>
</tr>

<tr>
<td>Website:</td>
<td>http://www.openexr.com</td>
</tr>

<tr>
<td>Licensed Under:</td>
<td><a href="http://www.xfree86.org/3.3.6/COPYRIGHT2.html#5">Modified BSD License</a></td>
</tr>

<tr>
<td>Needed By:</td>
<td>Codec_EXR</td>
</tr>

</tbody>
</table>

### pugixml

<table>
<tbody>
<tr>
<td style="width: 20%;">
Original Authors:</td>
<td>
Arseny Kapoulkine</td>
</tr>
<tr>
<td>
Website:</td>
<td>
 https://pugixml.org/</td>
</tr>
<tr>
<td>
Licensed Under:</td>
<td>
<a href="licenses/mit.txt">MIT License</a></td>
</tr>

<tr>
<td>Needed By:</td>
<td>OgreXMLConverter, Bites Component</td>
</tr>

</tbody>
</table>

## Source code used in-place in OGRE

<ul>
<li>
    <p style="margin-bottom: 0cm;">Many of the maths/spatial routines
        were adapted from work by Geometric Tools, LLC, Copyright (c) 1998-2010 and licensed under the Boost software license.</p>

</li>

</ul>

## Art credits for demos

<ul>
    <li>
        <p style="margin-bottom: 0cm;">Matt Anderson at
            <a href="http://www.The3dStudio.com/">www.The3dStudio.com</a> who
            kindly gave permission for the use of some textures. These textures are used with permission of
            <a href="http://www.the3dstudio.com/">www.The3dStudio.com</a> and may
            not be re-distributed, sold, or given away except in the form of
            rendered images, animations, or real time 3D applications when credit
            is given to <a href="http://www.the3dstudio.com/">www.The3dStudio.com</a>.</p>
    </li>
    <li>
        <p style="margin-bottom: 0cm;">Jonathan Clark at
            <a href="http://www.jonathanclark.com/">http://www.jonathanclark.com</a>
            for some Golgotha textures thet were released to the public domain.</p>
    </li>
    <li>
        <p style="margin-bottom: 0cm;">The 'Raptor Assault Gunboat' mesh
            &amp; texture are &copy; 2002 by Adrian 'cearny' Cearnau.</p>
    </li>
    <li>
        <p style="margin-bottom: 0cm;">The robot andn ninja mesh and
            animation are by <a href="http://www.psionic3d.co.uk/">Psionic</a>,
            kindly made available from the <a href="http://www.insanesoftware.de/">CharacterFX</a>
            site</p>
    </li>
    <li>
        <p style="margin-bottom: 0cm;">The 'Razor 2' mesh is by Dennis
            Verbeek</p>
    </li>
    <li>
        <p style="margin-bottom: 0cm;">Skyboxes in cubemapJS.zip are
            &copy;<a href="http://www.schlorb.com/">Johannes Schlorb</a>, used
            with permission.
        </p>
    </li>
    <li>
        <p>Grass texture is courtesy of Mathias 'freezer' Walc</p>
    </li>
    <li>
        <p>The 'Cuckoo' TrueType font is provided by <a href="http://www.grsites.com">http://www.grsites.com</a>.</p>
    </li>
    <li>SoftImage|XSI sample media files (facial.mesh, jaiqua.mesh and associated files) are provided courtesy of Avid Technology, Inc.<br />
        &copy; 2004 Avid Technology, Inc. All rights
        reserved. Avid is either a registered trademark or trademark of Avid
        Technology, Inc. in the United States and/or other countries.
    </li>
    <li>
        <p>The Sibenik Cathedral model by Marko Dabrovic - <a href="marko@3lhd.com">marko@3lhd.com</a>.</p>
    </li>
    <li>
        <p>Battle Damaged Sci-fi Helmet - PBR by theblueturtle_, published under a Creative Commons Attribution-NonCommercial license</p>
    </li>
    <li>
        <p>Smoke15Frames.png by Beast, published under CC-0 https://opengameart.org/content/smoke-aura</p>
    </li>
</ul>
