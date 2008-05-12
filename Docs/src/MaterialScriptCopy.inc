@node Copying Materials
@subsection Copying Materials

When creating new materials scripts that are only slight variations of another material script, the script writer resorts to copy and pasting materials within scripts.  For simple materials, the copy/paste method is acceptable.  Complex materials that use multiple passes along with vertex and pixel shaders become very tedious to edit when many variations are needed.  To reduce the tediousness of making new materials that are just slight variations of previously defined materials, new materials can copy from previously defined materials and modify specific techniques, passes, texture units or add new ones.@*@*

To make a new material that is based on one previously defined, add a @emph{colon} @strong{:} after the new material name followed by the name of the material that is to be copied.@*@*

Format: material <NewUniqueChildName> : <ReferanceParentMaterial>@*@*

The only caveat is that a parent material must have been defined/parsed prior to the child material script being parsed.  The easiest way to achieve this is to place parents at the begining of the material script file.  After material scripts are loaded into Ogre, materials no longer maintain their copy inheritance structure.  If a parent material is modified through code at runtime, the changes have no effect on child materials that were copied from it in the script.@*@*

Material copying within the script alleviates some drudgery from copy/paste but having the ability to identify specific techniques, passes, and texture units to modify makes material copying easier.  Techniques, passes, texture units can be identified directly in the child material without having to layout previous techniques, passes, texture units by associating a name with them,  Techniques and passes can take a name and texture units can be numbered within the material script.@*@*

@strong{NOTE:  Using names is optional so older scripts will still work just fine and you can inherit from them as well.}@*@*

Names become very usefull in materials that copy from other materials.  In order to overide values they must be in the correct technique, pass, texture unit etc.  The script could be layed out using the sequence of techniques, passes, texture units in the child material but if only one paramter needs to change in say the 5th pass then the first four passes prior to the fith would have to be placed in the script:@*@*

Here is an example:
@example
material test2 : test1
{
  technique
  {
    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
    }

    pass
    {
	  ambient 0.5 0.7 0.3 1.0
    }
  }
}
@end example

This method is tedious for materials that only have slight variations to their parent.
An easier way is to name the pass directly without listing the previous passes:@*

@example
material test2 : test1
{
  technique 0
  {
    pass 4
    {
	  ambient 0.5 0.7 0.3 1.0
    }
  }
}
@end example

The parent pass name must be known and the pass must be in the correct technique in order for this to work correctly.  Specifying the technique name and the pass name is the best method.  If the parent technique/pass are not named then use their index values for their name as done in the example.@*@*

@subheading Adding new Techniques, Passes, to copied materials:

If a new technique or pass needs to be added to a copied material then use a unique name for the technique or pass that does not exist in the parent material.  Using an index for the name that is one greater than the last index in the parent will do the same thing.  The new technique/pass will be added to the end of the techniques/passes copied from the parent material.@*@*

Note:  if passes or techniques aren't given a name, they will take on a default name based on their index.  For example the first pass has index 0 so its name will be 0.

@subheading Dealing with Program References and Shader Parameters:

For a child material that only needs to change some shader parameters within a pass, do the following:@*@*

@example
material furtest2 : furtest1
{
  technique ati8500
  {
    pass fur3
    {
      fragment_program_ref
      {
        param_named furlength float 0.5
      }
    }
  }
}
@end example

There is no requirement to give the name of the fragment program reference since the parent material has already done this and the same applies to a vertex program reference, but dropping the program reference name only applies if one is defined in the pass copied from the parent material.  If the parent didn't have a program reference then an exception is generated since the material parser has no clue as to what the script intended.@*@*

Program references can be added to copied passes that had no program references.  The whole program reference will have to be defined with in the pass.@*@*

If a different program ref is to be used in the copied pass then just put in the name of the different vertex/fragment program reference.  This is usefull when you want to inherit from a parent material but want to use a different shader program for one of the passes.@*@*

@subheading Identifying Texture Units to overide values

A specific texture unit state (TUS) can be given a unique name within a pass of a material so that it can be identified later in cloned materials that need to override specified texture unit states in the pass without declaring previous texture units.  Using a unique name for a Texture unit in a pass of a cloned material adds a new texture unit at the end of the texture unit list for the pass.@*@*

@example
material BumpMap2 : BumpMap1
{
  technique ati8500
  {
    pass 0
    {
      texture_unit NormalMap
      {
        texture BumpyMetalNM.png
      }
    }
  }
}
@end example

@subheading Texture Aliases

Texture aliases are usefull for when only the textures used in texture units need to be specified for a cloned material.  In the source material ie the original material to be cloned, each texture unit can be given a texture alias name.  The cloned material in the script can then specify what textures should be used for each texture alias.@*@*

Using texture aliases within texture units:@*
Format:@*
texture_alias <name>@*@*

Default: <name> will default to texture_unit <name> if set
@example
texture_unit DiffuseTex
{
  texture diffuse.jpg
}
@end example
texture_alias defaults to DiffuseTex.@*

Example: The base material to be cloned:@*
@example
material TSNormalSpecMapping
{ 
  technique GLSL
  { 
    pass 
    { 
      ambient 0.1 0.1 0.1
      diffuse 0.7 0.7 0.7
      specular 0.7 0.7 0.7 128
		
      vertex_program_ref GLSLDemo/OffsetMappingVS
      {
        param_named_auto lightPosition light_position_object_space 0
        param_named_auto eyePosition camera_position_object_space
        param_named textureScale float 1.0
      }

      fragment_program_ref GLSLDemo/TSNormalSpecMappingFS 
      { 
        param_named normalMap int 0
        param_named diffuseMap int 1
        param_named fxMap int 2
      } 

      // Normal map
      texture_unit NormalMap
      {
        texture defaultNM.png
        tex_coord_set 0
        filtering trilinear
      }

      // Base diffuse texture map
      texture_unit DiffuseMap
      {
        texture defaultDiff.png
        filtering trilinear
        tex_coord_set 1
      }

      // spec map for shinnines
      texture_unit SpecMap
      {
        texture defaultSpec.png
        filtering trilinear
        tex_coord_set 2
      }

    } 

  } 

  technique HLSL_DX9
  { 
    pass 
    { 
			
      vertex_program_ref FxMap_HLSL_VS
      {
        param_named_auto worldViewProj_matrix worldviewproj_matrix 
        param_named_auto lightPosition light_position_object_space 0
        param_named_auto eyePosition camera_position_object_space
      }

      fragment_program_ref FxMap_HLSL_PS 
      { 
        param_named ambientColor float4 0.2 0.2 0.2 0.2
      } 

      // Normal map
      texture_unit 
      {
        texture_alias NormalMap
        texture defaultNM.png
        tex_coord_set 0
        filtering trilinear
      }

      // Base diffuse texture map
      texture_unit 
      {
        texture_alias DiffuseMap
        texture defaultDiff.png
        filtering trilinear
        tex_coord_set 1
      }

      // spec map for shinnines
      texture_unit
      {
        texture_alias SpecMap
        texture defaultSpec.png
        filtering trilinear
        tex_coord_set 2
      }

    } 

  } 
}
@end example

Note that the GLSL and HLSL techniques use the same textures.  For each texture usage type a texture alias is given that describes what the texture is used for.  So the first texture unit in the GLSL technique has the same alias as the TUS in the HLSL technique since its the same texture used. Same goes for the second and third texture units.@*
For demonstration purposes, the GLSL technique makes use of texture_unit naming and therefore the texture_alias name does not have to be set since it defaults to the texture unit name.  So why not use the default all the time since its less typing?  For most situations you can.  Its when you clone a material that and then want to change the alias that you must use the texture_alias command in the script.  You cannot change the name of a texture_unit in a cloned material so texture_alias provides a facility to assign an alias name.@*@*

Now we want to clone the material but only want to change the textures used.  We could copy and paste the whole material but if we decide to change the base material later then we also have to update the copied material in the script.  With set_texture_alias, copying a material is very easy now.  set_texture_alias is specified at the top of the material definition.  All techniques using the specified texture alias will be effected by set_texture_alias.@*@*

Format:@*
set_texture_alias <alias name> <texture name>@*
@example
material fxTest : TSNormalSpecMapping
{
  set_texture_alias NormalMap fxTestNMap.png
  set_texture_alias DiffuseMap fxTestDiff.png
  set_texture_alias SpecMap fxTestMap.png
}
@end example

The textures in both techniques in the child material will automatically get replaced with the new ones we want to use.@*@*

  The same process can be done in code as long you set up the texture alias names so then there is no need to traverse technique/pass/TUS to change a texture.  You just call myMaterialPtr->applyTextureAliases(myAliasTextureNameList) which will update all textures in all texture units that match the alias names in the map container reference you passed as a parameter.@*@*

 You don't have to supply all the textures in the copied material.@*
@example
material fxTest2 : fxTest
{
  set_texture_alias DiffuseMap fxTest2Diff.png
  set_texture_alias SpecMap fxTest2Map.png
}
@end example

Material fxTest2 only changes the diffuse and spec maps of material fxTest and uses the same normal map.@*@*

Another example:
@example
material fxTest3 : TSNormalSpecMapping
{
  set_texture_alias DiffuseMap fxTest2Diff.png
}
@end example

fxTest3 will end up with the default textures for the normal map and spec map setup in TSNormalSpecMapping material but will have a different diffuse map.  So your base material can define the default textures to use and then the child materials can override specific textures.@*@*
