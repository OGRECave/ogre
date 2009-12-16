@node Script Inheritence
@subsection Script Inheritence

When creating new script objects that are only slight variations of another object, it's good to avoid copying and pasting between scripts.  Script inheritence lets you do this; in this section we'll use material scripts as an example, but this applies to all scripts parsed with the script compilers in Ogre 1.6 onwards.@*@*

For example, to make a new material that is based on one previously defined, add a @emph{colon} @strong{:} after the new material name followed by the name of the material that is to be copied.@*@*

Format: material <NewUniqueChildName> : <ReferanceParentMaterial>@*@*

The only caveat is that a parent material must have been defined/parsed prior to the child material script being parsed.  The easiest way to achieve this is to either place parents at the beginning of the material script file, or to use the 'import' directive (@xref{Script Import Directive}). Note that inheritence is actually a copy - after scripts are loaded into Ogre, objects no longer maintain their copy inheritance structure.  If a parent material is modified through code at runtime, the changes have no effect on child materials that were copied from it in the script.@*@*

Material copying within the script alleviates some drudgery from copy/paste but having the ability to identify specific techniques, passes, and texture units to modify makes material copying easier.  Techniques, passes, texture units can be identified directly in the child material without having to layout previous techniques, passes, texture units by associating a name with them,  Techniques and passes can take a name and texture units can be numbered within the material script. You can also use variables, @xref{Script Variables}.@*@*

Names become very useful in materials that copy from other materials.  In order to override values they must be in the correct technique, pass, texture unit etc.  The script could be lain out using the sequence of techniques, passes, texture units in the child material but if only one parameter needs to change in say the 5th pass then the first four passes prior to the fifth would have to be placed in the script:@*@*

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

@subheading Identifying Texture Units to override values

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

@subheading Advanced Script Inheritence
Starting with Ogre 1.6, script objects can now inherit from each other more generally. The previous concept of inheritance, material copying, was restricted only to the top-level material objects. Now, any level of object can take advantage of inheritance (for instance, techniques, passes, and compositor targets).@*@*

@example
material Test
{
    technique
    {
        pass : ParentPass
        {
        }
    }
}
@end example

Notice that the pass inherits from ParentPass. This allows for the creation of more fine-grained inheritance hierarchies.@*@*

Along with the more generalized inheritance system comes an important new keyword: "abstract." This keyword is used at a top-level object declaration (not inside any other object) to denote that it is not something that the compiler should actually attempt to compile, but rather that it is only for the purpose of inheritance. For example, a material declared with the abstract keyword will never be turned into an actual usable material in the material framework. Objects which cannot be at a top-level in the document (like a pass) but that you would like to declare as such for inheriting purpose must be declared with the abstract keyword.@*@*

@example
abstract pass ParentPass
{
    diffuse 1 0 0 1
}
@end example

That declares the ParentPass object which was inherited from in the above example. Notice the abstract keyword which informs the compiler that it should not attempt to actually turn this object into any sort of Ogre resource. If it did attempt to do so, then it would obviously fail, since a pass all on its own like that is not valid.@*@*

The final matching option is based on wildcards. Using the '*' character, you can make a powerful matching scheme and override multiple objects at once, even if you don't know exact names or positions of those objects in the inherited object.@*@*

@example
abstract technique Overrider
{
   pass *color*
   {
      diffuse 0 0 0 0
   }
} 
@end example

This technique, when included in a material, will override all passes matching the wildcard "*color*" (color has to appear in the name somewhere) and turn their diffuse properties black. It does not matter their position or exact name in the inherited technique, this will match them. @*@*


@node Texture Aliases
@subsection Texture Aliases

Texture aliases are useful for when only the textures used in texture units need to be specified for a cloned material.  In the source material i.e. the original material to be cloned, each texture unit can be given a texture alias name.  The cloned material in the script can then specify what textures should be used for each texture alias. Note that texture aliases are a more specific version of @ref{Script Variables} which can be used to easily set other values.@*@*

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

      // spec map for shininess
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

      // spec map for shininess
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

@node Script Variables
@subsection Script Variables
A very powerful new feature in Ogre 1.6 is variables. Variables allow you to parameterize data in materials so that they can become more generalized. This enables greater reuse of scripts by targeting specific customization points. Using variables along with inheritance allows for huge amounts of overrides and easy object reuse.@*@*

@example
abstract pass ParentPass
{
   diffuse $diffuse_colour
}

material Test
{
   technique
   {
       pass : ParentPass
       {
           set $diffuse_colour "1 0 0 1"
       }
   }
}
@end example

The ParentPass object declares a variable called "diffuse_colour" which is then overridden in the Test material's pass. The "set" keyword is used to set the value of that variable. The variable assignment follows lexical scoping rules, which means that the value of "1 0 0 1" is only valid inside that pass definition. Variable assignment in outer scopes carry over into inner scopes.@*@*

@example
material Test
{
    set $diffuse_colour "1 0 0 1"
    technique
    {
        pass : ParentPass
        {
        }
    }
}
@end example

The $diffuse_colour assignment carries down through the technique and into the pass. @*@*



@node Script Import Directive
@subsection Script Import Directive

Imports are a feature introduced to remove ambiguity from script dependencies. When using scripts that inherit from each other but which are defined in separate files sometimes errors occur because the scripts are loaded in incorrect order. Using imports removes this issue. The script which is inheriting another can explicitly import its parent's definition which will ensure that no errors occur because the parent's definition was not found.@*@*

@example
import * from "parent.material"
material Child : Parent
{
}
@end example

The material "Parent" is defined in parent.material and the import ensures that those definitions are found properly. You can also import specific targets from within a file.

@example
import Parent from "parent.material"
@end example

If there were other definitions in the parent.material file, they would not be imported.@*@*

Note, however that importing does not actually cause objects in the imported script to be fully parsed & created, it just makes the definitions available for inheritence. This has a specific ramification for vertex / fragment program definitions, which must be loaded before any parameters can be specified. You should continue to put common program definitions in .program files to ensure they are fully parsed before being referenced in multiple .material files. The 'import' command just makes sure you can resolve dependencies between equivalent script definitions (e.g. material to material).@*@*

