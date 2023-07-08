/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "OGRE", "index.html", [
    [ "API Reference Start Page", "index.html", "index" ],
    [ "Manual", "manual.html", [
      [ "Introduction", "_introduction.html", [
        [ "Object Orientation - more than just a buzzword", "_introduction.html#autotoc_md25", null ],
        [ "Multi-everything", "_introduction.html#autotoc_md26", null ]
      ] ],
      [ "The Core Objects", "_the-_core-_objects.html", [
        [ "Overview from 10,000 feet", "_the-_core-_objects.html#autotoc_md27", null ],
        [ "The Root object", "_the-_core-_objects.html#The-Root-Object", null ],
        [ "The RenderSystem object", "_the-_core-_objects.html#The-RenderSystem-object", null ],
        [ "The SceneManager object", "_the-_core-_objects.html#The-SceneManager-object", null ],
        [ "The ResourceGroupManager Object", "_the-_core-_objects.html#The-ResourceGroupManager-Object", null ],
        [ "The Mesh Object", "_the-_core-_objects.html#The-Mesh-Object", null ],
        [ "Entities", "_the-_core-_objects.html#Entities", null ],
        [ "Materials", "_the-_core-_objects.html#Materials", [
          [ "Managing", "_the-_core-_objects.html#Materials-Manage", null ],
          [ "Structure", "_the-_core-_objects.html#Materials-Structure", null ]
        ] ],
        [ "Overlays", "_the-_core-_objects.html#Overlays", [
          [ "How to Enable Overlays", "_the-_core-_objects.html#autotoc_md28", null ],
          [ "Creating 2D Elements", "_the-_core-_objects.html#autotoc_md29", null ],
          [ "Adding 2D Elements to the Overlay", "_the-_core-_objects.html#autotoc_md30", null ],
          [ "A word about 2D coordinates", "_the-_core-_objects.html#autotoc_md31", null ],
          [ "GUI systems", "_the-_core-_objects.html#autotoc_md33", null ]
        ] ]
      ] ],
      [ "Resource Management", "_resource-_management.html", [
        [ "Resource Life-cycle", "_resource-_management.html#Resource-Life-cycle", null ],
        [ "Locations", "_resource-_management.html#Resource-Location", null ],
        [ "Groups", "_resource-_management.html#Resource-Groups", [
          [ "Resource-Declaration", "_resource-_management.html#Resource-Declaration", null ]
        ] ]
      ] ],
      [ "Scripts", "_scripts.html", [
        [ "Loading scripts", "_scripts.html#autotoc_md63", null ],
        [ "Format", "_scripts.html#Format", [
          [ "Script Inheritance", "_scripts.html#Script-Inheritance", [
            [ "Advanced Script Inheritance", "_scripts.html#Advanced-Script-Inheritance", null ]
          ] ],
          [ "Script Variables", "_scripts.html#Script-Variables", null ],
          [ "Script Import Directive", "_scripts.html#Script-Import-Directive", null ]
        ] ],
        [ "Custom Translators", "_scripts.html#custom-translators", null ],
        [ "Material Scripts", "_material-_scripts.html", [
          [ "Material", "_material-_scripts.html#Material", [
            [ "lod_strategy", "_material-_scripts.html#autotoc_md110", null ],
            [ "lod_values", "_material-_scripts.html#autotoc_md111", null ],
            [ "lod_distances", "_material-_scripts.html#autotoc_md112", null ],
            [ "receive_shadows", "_material-_scripts.html#autotoc_md113", null ],
            [ "transparency_casts_shadows", "_material-_scripts.html#autotoc_md114", null ],
            [ "set_texture_alias", "_material-_scripts.html#autotoc_md115", null ]
          ] ],
          [ "Techniques", "_material-_scripts.html#Techniques", [
            [ "scheme", "_material-_scripts.html#autotoc_md116", null ],
            [ "lod_index", "_material-_scripts.html#autotoc_md117", null ],
            [ "shadow_caster_material", "_material-_scripts.html#autotoc_md118", null ],
            [ "shadow_receiver_material", "_material-_scripts.html#autotoc_md119", null ],
            [ "gpu_vendor_rule and gpu_device_rule", "_material-_scripts.html#autotoc_md120", null ]
          ] ],
          [ "Passes", "_material-_scripts.html#Passes", null ],
          [ "Attribute Descriptions", "_material-_scripts.html#autotoc_md121", [
            [ "ambient", "_material-_scripts.html#autotoc_md122", null ],
            [ "diffuse", "_material-_scripts.html#autotoc_md123", null ],
            [ "specular", "_material-_scripts.html#autotoc_md124", null ],
            [ "emissive", "_material-_scripts.html#autotoc_md125", null ],
            [ "scene_blend", "_material-_scripts.html#autotoc_md126", null ],
            [ "separate_scene_blend", "_material-_scripts.html#autotoc_md127", null ],
            [ "scene_blend_op", "_material-_scripts.html#autotoc_md128", null ],
            [ "separate_scene_blend_op", "_material-_scripts.html#autotoc_md129", null ],
            [ "depth_check", "_material-_scripts.html#autotoc_md130", null ],
            [ "depth_write", "_material-_scripts.html#autotoc_md131", null ],
            [ "depth_func", "_material-_scripts.html#autotoc_md132", null ],
            [ "depth_bias", "_material-_scripts.html#autotoc_md133", null ],
            [ "iteration_depth_bias", "_material-_scripts.html#autotoc_md134", null ],
            [ "alpha_rejection", "_material-_scripts.html#autotoc_md135", null ],
            [ "alpha_to_coverage", "_material-_scripts.html#autotoc_md136", null ],
            [ "light_scissor", "_material-_scripts.html#autotoc_md137", null ],
            [ "light_clip_planes", "_material-_scripts.html#autotoc_md138", null ],
            [ "illumination_stage", "_material-_scripts.html#illumination_005fstage", null ],
            [ "transparent_sorting", "_material-_scripts.html#autotoc_md139", null ],
            [ "cull_hardware", "_material-_scripts.html#autotoc_md140", null ],
            [ "cull_software", "_material-_scripts.html#autotoc_md141", null ],
            [ "lighting", "_material-_scripts.html#autotoc_md142", null ],
            [ "shading", "_material-_scripts.html#autotoc_md143", null ],
            [ "polygon_mode", "_material-_scripts.html#autotoc_md144", null ],
            [ "polygon_mode_overrideable", "_material-_scripts.html#autotoc_md145", null ],
            [ "fog_override", "_material-_scripts.html#autotoc_md146", null ],
            [ "colour_write", "_material-_scripts.html#autotoc_md147", null ],
            [ "start_light", "_material-_scripts.html#autotoc_md148", null ],
            [ "max_lights", "_material-_scripts.html#autotoc_md149", null ],
            [ "iteration", "_material-_scripts.html#iteration", null ],
            [ "point_size", "_material-_scripts.html#autotoc_md150", null ],
            [ "point_sprites", "_material-_scripts.html#autotoc_md151", null ],
            [ "point_size_attenuation", "_material-_scripts.html#autotoc_md152", null ],
            [ "point_size_min", "_material-_scripts.html#autotoc_md153", null ],
            [ "point_size_max", "_material-_scripts.html#autotoc_md154", null ],
            [ "line_width", "_material-_scripts.html#autotoc_md155", null ]
          ] ],
          [ "Texture Units", "_material-_scripts.html#Texture-Units", [
            [ "Available Texture Unit Attributes", "_material-_scripts.html#autotoc_md156", null ],
            [ "Attribute Descriptions", "_material-_scripts.html#autotoc_md157", null ],
            [ "texture_alias", "_material-_scripts.html#autotoc_md158", null ],
            [ "texture", "_material-_scripts.html#autotoc_md159", null ],
            [ "anim_texture", "_material-_scripts.html#autotoc_md160", null ],
            [ "cubic_texture", "_material-_scripts.html#autotoc_md161", null ],
            [ "content_type", "_material-_scripts.html#autotoc_md162", null ],
            [ "tex_coord_set", "_material-_scripts.html#autotoc_md163", null ],
            [ "colour_op", "_material-_scripts.html#autotoc_md164", null ],
            [ "colour_op_ex", "_material-_scripts.html#autotoc_md165", null ],
            [ "colour_op_multipass_fallback", "_material-_scripts.html#autotoc_md166", null ],
            [ "alpha_op_ex", "_material-_scripts.html#autotoc_md167", null ],
            [ "env_map", "_material-_scripts.html#autotoc_md168", null ],
            [ "scroll", "_material-_scripts.html#autotoc_md169", null ],
            [ "scroll_anim", "_material-_scripts.html#autotoc_md170", null ],
            [ "rotate", "_material-_scripts.html#autotoc_md171", null ],
            [ "rotate_anim", "_material-_scripts.html#autotoc_md172", null ],
            [ "scale", "_material-_scripts.html#autotoc_md173", null ],
            [ "wave_xform", "_material-_scripts.html#autotoc_md174", null ],
            [ "transform", "_material-_scripts.html#autotoc_md175", null ],
            [ "sampler_ref", "_material-_scripts.html#autotoc_md176", null ],
            [ "unordered_access_mip", "_material-_scripts.html#autotoc_md177", null ]
          ] ],
          [ "Samplers", "_material-_scripts.html#Samplers", [
            [ "Available parameters", "_material-_scripts.html#autotoc_md178", null ],
            [ "tex_address_mode", "_material-_scripts.html#autotoc_md179", null ],
            [ "tex_border_colour", "_material-_scripts.html#autotoc_md180", null ],
            [ "filtering", "_material-_scripts.html#autotoc_md181", [
              [ "Simple Format", "_material-_scripts.html#autotoc_md182", null ],
              [ "Complex Format", "_material-_scripts.html#autotoc_md183", null ]
            ] ],
            [ "max_anisotropy", "_material-_scripts.html#autotoc_md184", null ],
            [ "mipmap_bias", "_material-_scripts.html#autotoc_md185", null ],
            [ "compare_test", "_material-_scripts.html#autotoc_md186", null ],
            [ "comp_func", "_material-_scripts.html#autotoc_md187", null ]
          ] ],
          [ "Using GPU Programs in a Pass", "_material-_scripts.html#Using-Vertex_002fGeometry_002fFragment-Programs-in-a-Pass", null ],
          [ "Adding new Techniques, Passes, to copied materials", "_material-_scripts.html#Adding-new-Techniques_002c-Passes_002c-to-copied-materials_003a", null ],
          [ "Identifying Texture Units to override values", "_material-_scripts.html#Identifying-Texture-Units-to-override-values", null ]
        ] ],
        [ "GPU Program Scripts", "_high-level-_programs.html", [
          [ "Default Program Parameters", "_high-level-_programs.html#Default-Program-Parameters", null ],
          [ "High Level Programs", "_high-level-_programs.html#autotoc_md188", [
            [ "Preprocessor definitions", "_high-level-_programs.html#Preprocessor-definitions", null ],
            [ "Entry Point", "_high-level-_programs.html#autotoc_md189", null ]
          ] ],
          [ "GLSL programs", "_high-level-_programs.html#GLSL", [
            [ "Binding vertex attributes", "_high-level-_programs.html#Binding-vertex-attributes", null ],
            [ "Buffers in Mesh Shaders", "_high-level-_programs.html#GLSL-Mesh-Shaders", null ],
            [ "Binding Texture Samplers", "_high-level-_programs.html#GLSL-Texture-Samplers", null ],
            [ "Matrix parameters", "_high-level-_programs.html#Matrix-parameters", null ],
            [ "Uniform Buffers", "_high-level-_programs.html#Uniform-Buffers", null ],
            [ "Transform Feedback Varyings", "_high-level-_programs.html#Transform-Feedback-Varyings", null ],
            [ "Compatibility profile GLSL features", "_high-level-_programs.html#Legacy-GLSL-features", [
              [ "OpenGL state", "_high-level-_programs.html#autotoc_md190", null ],
              [ "Built-in attributes", "_high-level-_programs.html#autotoc_md191", null ],
              [ "Geometry shader in/ out", "_high-level-_programs.html#autotoc_md192", null ],
              [ "Multi module shaders", "_high-level-_programs.html#autotoc_md193", null ]
            ] ]
          ] ],
          [ "Cg programs", "_high-level-_programs.html#Cg", null ],
          [ "DirectX HLSL", "_high-level-_programs.html#HLSL", null ],
          [ "Assembler Shaders", "_high-level-_programs.html#Assembler-Shaders", [
            [ "Specifying Named Constants", "_high-level-_programs.html#Specifying-Named-Constants-for-Assembler-Shaders", null ]
          ] ],
          [ "Multi-language Programs", "_high-level-_programs.html#multi-language-programs", null ],
          [ "Unified High-level Programs", "_high-level-_programs.html#Unified-High_002dlevel-Programs", null ],
          [ "Parameter specification", "_high-level-_programs.html#Program-Parameter-Specification", [
            [ "param_indexed", "_high-level-_programs.html#autotoc_md194", null ],
            [ "param_indexed_auto", "_high-level-_programs.html#autotoc_md195", null ],
            [ "param_named", "_high-level-_programs.html#autotoc_md196", null ],
            [ "param_named_auto", "_high-level-_programs.html#autotoc_md197", null ],
            [ "shared_params_ref", "_high-level-_programs.html#autotoc_md198", null ]
          ] ],
          [ "Declaring Shared Parameters", "_high-level-_programs.html#Declaring-Shared-Parameters", [
            [ "Hardware Support", "_high-level-_programs.html#autotoc_md199", null ]
          ] ],
          [ "Shadows and Vertex Programs", "_high-level-_programs.html#Shadows-and-Vertex-Programs", null ],
          [ "Instancing in Vertex Programs", "_high-level-_programs.html#Instancing-in-Vertex-Programs", null ],
          [ "Skeletal Animation in Vertex Programs", "_high-level-_programs.html#Skeletal-Animation-in-Vertex-Programs", null ],
          [ "Morph Animation in Vertex Programs", "_high-level-_programs.html#Morph-Animation-in-Vertex-Programs", null ],
          [ "Pose Animation in Vertex Programs", "_high-level-_programs.html#Pose-Animation-in-Vertex-Programs", null ],
          [ "Vertex Texture Fetch", "_high-level-_programs.html#Vertex-Texture-Fetch", [
            [ "Declaring the use of vertex texture fetching", "_high-level-_programs.html#autotoc_md200", null ],
            [ "DirectX9 binding limitations", "_high-level-_programs.html#autotoc_md201", null ],
            [ "Texture format limitations", "_high-level-_programs.html#autotoc_md202", null ],
            [ "Hardware limitations", "_high-level-_programs.html#autotoc_md203", null ]
          ] ],
          [ "Programmatic creation", "_high-level-_programs.html#GpuProgram-API", null ]
        ] ],
        [ "Compositor Scripts", "_compositor-_scripts.html", [
          [ "Compositor Fundamentals", "_compositor-_scripts.html#Compositor-Fundamentals", null ],
          [ "Techniques", "_compositor-_scripts.html#Compositor-Techniques", [
            [ "texture", "_compositor-_scripts.html#compositor-texture", null ],
            [ "texture_ref", "_compositor-_scripts.html#autotoc_md64", null ],
            [ "scheme", "_compositor-_scripts.html#autotoc_md65", null ],
            [ "compositor_logic", "_compositor-_scripts.html#autotoc_md66", null ]
          ] ],
          [ "Target Sections", "_compositor-_scripts.html#Compositor-Target-Passes", [
            [ "input", "_compositor-_scripts.html#autotoc_md67", null ],
            [ "only_initial", "_compositor-_scripts.html#autotoc_md68", null ],
            [ "visibility_mask", "_compositor-_scripts.html#autotoc_md69", null ],
            [ "lod_bias", "_compositor-_scripts.html#autotoc_md70", null ],
            [ "shadows", "_compositor-_scripts.html#autotoc_md71", null ],
            [ "material_scheme", "_compositor-_scripts.html#autotoc_md72", null ]
          ] ],
          [ "Passes", "_compositor-_scripts.html#Compositor-Passes", [
            [ "render_quad & compute", "_compositor-_scripts.html#autotoc_md73", [
              [ "material", "_compositor-_scripts.html#autotoc_md74", null ],
              [ "input", "_compositor-_scripts.html#autotoc_md75", null ],
              [ "identifier", "_compositor-_scripts.html#autotoc_md76", null ],
              [ "quad_normals", "_compositor-_scripts.html#autotoc_md77", null ],
              [ "thread_groups", "_compositor-_scripts.html#autotoc_md78", null ]
            ] ],
            [ "render_scene", "_compositor-_scripts.html#autotoc_md79", [
              [ "first_render_queue", "_compositor-_scripts.html#autotoc_md80", null ],
              [ "last_render_queue", "_compositor-_scripts.html#autotoc_md81", null ],
              [ "material_scheme", "_compositor-_scripts.html#autotoc_md82", null ],
              [ "camera", "_compositor-_scripts.html#autotoc_md83", null ]
            ] ],
            [ "clear", "_compositor-_scripts.html#Clear-Section", [
              [ "buffers", "_compositor-_scripts.html#autotoc_md84", null ],
              [ "colour_value", "_compositor-_scripts.html#autotoc_md85", null ],
              [ "depth_value", "_compositor-_scripts.html#autotoc_md86", null ],
              [ "stencil_value", "_compositor-_scripts.html#autotoc_md87", null ]
            ] ],
            [ "stencil", "_compositor-_scripts.html#Stencil-Section", [
              [ "check", "_compositor-_scripts.html#autotoc_md88", null ],
              [ "comp_func", "_compositor-_scripts.html#autotoc_md89", null ],
              [ "ref_value", "_compositor-_scripts.html#autotoc_md90", null ],
              [ "mask", "_compositor-_scripts.html#autotoc_md91", null ],
              [ "fail_op", "_compositor-_scripts.html#autotoc_md92", null ],
              [ "depth_fail_op", "_compositor-_scripts.html#autotoc_md93", null ],
              [ "pass_op", "_compositor-_scripts.html#autotoc_md94", null ],
              [ "two_sided", "_compositor-_scripts.html#autotoc_md95", null ]
            ] ]
          ] ],
          [ "Applying a Compositor", "_compositor-_scripts.html#Applying-a-Compositor", null ],
          [ "Programmatic creation", "_compositor-_scripts.html#Compositor-API", null ]
        ] ],
        [ "Particle Scripts", "_particle-_scripts.html", [
          [ "Particle System Attributes", "_particle-_scripts.html#Particle-System-Attributes", [
            [ "quota", "_particle-_scripts.html#autotoc_md206", null ],
            [ "material", "_particle-_scripts.html#autotoc_md207", null ],
            [ "particle_width", "_particle-_scripts.html#autotoc_md208", null ],
            [ "particle_height", "_particle-_scripts.html#autotoc_md209", null ],
            [ "cull_each", "_particle-_scripts.html#autotoc_md210", null ],
            [ "renderer", "_particle-_scripts.html#autotoc_md211", null ],
            [ "sorted", "_particle-_scripts.html#autotoc_md212", null ],
            [ "local_space", "_particle-_scripts.html#autotoc_md213", null ],
            [ "iteration_interval", "_particle-_scripts.html#autotoc_md214", null ],
            [ "nonvisible_update_timeout", "_particle-_scripts.html#autotoc_md215", null ]
          ] ],
          [ "Billboard Renderer Attributes", "_particle-_scripts.html#Billboard-Renderer-Attributes", [
            [ "billboard_type", "_particle-_scripts.html#autotoc_md216", null ],
            [ "billboard_origin", "_particle-_scripts.html#autotoc_md217", null ],
            [ "billboard_rotation_type", "_particle-_scripts.html#autotoc_md218", null ],
            [ "common_direction", "_particle-_scripts.html#autotoc_md219", null ],
            [ "common_up_vector", "_particle-_scripts.html#autotoc_md220", null ],
            [ "point_rendering", "_particle-_scripts.html#autotoc_md221", null ],
            [ "accurate_facing", "_particle-_scripts.html#autotoc_md222", null ],
            [ "texture_sheet_size", "_particle-_scripts.html#autotoc_md223", null ]
          ] ],
          [ "Particle Emitters", "_particle-_scripts.html#Particle-Emitters", [
            [ "Emitting Emitters", "_particle-_scripts.html#Emitting-Emitters", null ],
            [ "Common Emitter Attributes", "_particle-_scripts.html#autotoc_md224", null ],
            [ "angle", "_particle-_scripts.html#autotoc_md225", null ],
            [ "colour", "_particle-_scripts.html#autotoc_md226", null ],
            [ "colour_range_start & colour_range_end", "_particle-_scripts.html#autotoc_md227", null ],
            [ "direction", "_particle-_scripts.html#autotoc_md228", null ],
            [ "direction_position_reference", "_particle-_scripts.html#autotoc_md229", null ],
            [ "emission_rate", "_particle-_scripts.html#autotoc_md230", null ],
            [ "position", "_particle-_scripts.html#autotoc_md231", null ],
            [ "velocity", "_particle-_scripts.html#autotoc_md232", null ],
            [ "velocity_min & velocity_max", "_particle-_scripts.html#autotoc_md233", null ],
            [ "time_to_live", "_particle-_scripts.html#autotoc_md234", null ],
            [ "time_to_live_min & time_to_live_max", "_particle-_scripts.html#autotoc_md235", null ],
            [ "duration", "_particle-_scripts.html#autotoc_md236", null ],
            [ "duration_min & duration_max", "_particle-_scripts.html#autotoc_md237", null ],
            [ "repeat_delay", "_particle-_scripts.html#autotoc_md238", null ],
            [ "repeat_delay_min & repeat_delay_max", "_particle-_scripts.html#autotoc_md239", null ]
          ] ],
          [ "Standard Particle Emitters", "_particle-_scripts.html#Standard-Particle-Emitters", [
            [ "Point Emitter", "_particle-_scripts.html#Point-Emitter", null ],
            [ "Box Emitter", "_particle-_scripts.html#Box-Emitter", null ],
            [ "Cylinder Emitter", "_particle-_scripts.html#Cylinder-Emitter", null ],
            [ "Ellipsoid Emitter", "_particle-_scripts.html#Ellipsoid-Emitter", null ],
            [ "Hollow Ellipsoid Emitter", "_particle-_scripts.html#Hollow-Ellipsoid-Emitter", null ],
            [ "Ring Emitter", "_particle-_scripts.html#Ring-Emitter", null ]
          ] ],
          [ "Particle Affectors", "_particle-_scripts.html#Particle-Affectors", null ],
          [ "Standard Particle Affectors", "_particle-_scripts.html#Standard-Particle-Affectors", [
            [ "Linear Force Affector", "_particle-_scripts.html#Linear-Force-Affector", null ],
            [ "ColourFader Affector", "_particle-_scripts.html#ColourFader-Affector", null ],
            [ "ColourFader2 Affector", "_particle-_scripts.html#ColourFader2-Affector", null ],
            [ "Scaler Affector", "_particle-_scripts.html#Scaler-Affector", null ],
            [ "Rotator Affector", "_particle-_scripts.html#Rotator-Affector", null ],
            [ "ColourInterpolator Affector", "_particle-_scripts.html#ColourInterpolator-Affector", null ],
            [ "ColourImage Affector", "_particle-_scripts.html#ColourImage-Affector", null ],
            [ "DeflectorPlane Affector", "_particle-_scripts.html#DeflectorPlane-Affector", null ],
            [ "DirectionRandomiser Affector", "_particle-_scripts.html#DirectionRandomiser-Affector", null ],
            [ "TextureAnimator Affector", "_particle-_scripts.html#TextureAnimator-Affector", null ]
          ] ]
        ] ],
        [ "Overlay Scripts", "_overlay-_scripts.html", [
          [ "Adding elements to the overlay", "_overlay-_scripts.html#Adding-elements-to-the-overlay", [
            [ "’overlay_element’ blocks", "_overlay-_scripts.html#autotoc_md96", null ]
          ] ],
          [ "Templates", "_overlay-_scripts.html#Templates", [
            [ "OverlayElement Attributes", "_overlay-_scripts.html#autotoc_md97", null ],
            [ "metrics_mode", "_overlay-_scripts.html#autotoc_md98", null ],
            [ "horz_align", "_overlay-_scripts.html#autotoc_md99", null ],
            [ "vert_align", "_overlay-_scripts.html#autotoc_md100", null ],
            [ "left", "_overlay-_scripts.html#autotoc_md101", null ],
            [ "top", "_overlay-_scripts.html#autotoc_md102", null ],
            [ "width", "_overlay-_scripts.html#autotoc_md103", null ],
            [ "height", "_overlay-_scripts.html#autotoc_md104", null ],
            [ "material", "_overlay-_scripts.html#autotoc_md105", null ],
            [ "caption", "_overlay-_scripts.html#autotoc_md106", null ],
            [ "rotation", "_overlay-_scripts.html#autotoc_md107", null ]
          ] ],
          [ "Standard OverlayElements", "_overlay-_scripts.html#Standard-OverlayElements", [
            [ "Panel (container)", "_overlay-_scripts.html#Panel", null ],
            [ "BorderPanel (container)", "_overlay-_scripts.html#BorderPanel", null ],
            [ "TextArea (element)", "_overlay-_scripts.html#TextArea", null ]
          ] ]
        ] ],
        [ "Font Definition Scripts", "_font-_definition-_scripts.html", [
          [ "Using an existing font texture", "_font-_definition-_scripts.html#autotoc_md108", null ],
          [ "Generating a font texture", "_font-_definition-_scripts.html#autotoc_md109", null ]
        ] ]
      ] ],
      [ "Runtime Shader Generation", "rtss.html", [
        [ "RTSS Pass properties", "rtss.html#rtss_custom_mat", [
          [ "transform_stage", "rtss.html#autotoc_md244", null ],
          [ "lighting_stage", "rtss.html#autotoc_md245", null ],
          [ "image_based_lighting", "rtss.html#autotoc_md246", null ],
          [ "gbuffer", "rtss.html#autotoc_md247", null ],
          [ "normal_map", "rtss.html#autotoc_md248", null ],
          [ "metal_roughness", "rtss.html#autotoc_md249", null ],
          [ "fog_stage", "rtss.html#autotoc_md250", null ],
          [ "light_count", "rtss.html#autotoc_md251", null ],
          [ "triplanarTexturing", "rtss.html#autotoc_md252", null ],
          [ "integrated_pssm4", "rtss.html#autotoc_md253", null ],
          [ "shadow_mapping", "rtss.html#shadow_mapping", null ],
          [ "hardware_skinning", "rtss.html#autotoc_md254", null ]
        ] ],
        [ "RTSS Texture Unit properties", "rtss.html#rtss_tu_props", [
          [ "normal_map", "rtss.html#normal_map", null ],
          [ "layered_blend", "rtss.html#autotoc_md255", null ],
          [ "source_modifier", "rtss.html#autotoc_md256", null ]
        ] ],
        [ "Setting properties programmatically", "rtss.html#RTSS-Props-API", null ],
        [ "System overview", "rtss.html#rtss_overview", [
          [ "Core features of the system", "rtss.html#core-feats", null ],
          [ "Controlling shader re-generation", "rtss.html#autotoc_md257", null ]
        ] ],
        [ "The RTSS in Depth", "rtss.html#rtss_indepth", [
          [ "Main components", "rtss.html#rtss__components", null ],
          [ "Initializing the system", "rtss.html#autotoc_md258", null ],
          [ "Customizing the default RenderState", "rtss.html#rtss_custom_api", null ],
          [ "Creating the shader based technique", "rtss.html#rtssTech", null ],
          [ "Shader generation at runtime", "rtss.html#rtssGenerate", null ],
          [ "Creating custom shader extensions", "rtss.html#creating-extensions", null ],
          [ "Tips for debugging shaders", "rtss.html#debugging", null ]
        ] ]
      ] ],
      [ "Mesh Tools", "_mesh-_tools.html", [
        [ "XMLConverter", "_mesh-_tools.html#XMLConverter", null ],
        [ "MeshUpgrader", "_mesh-_tools.html#MeshUpgrader", null ],
        [ "AssimpConverter", "_mesh-_tools.html#AssimpConverter", null ],
        [ "Exporters", "_mesh-_tools.html#Exporters", [
          [ "Empty material names", "_mesh-_tools.html#empty-material-names", null ],
          [ "Skeletal animation", "_mesh-_tools.html#autotoc_md34", null ]
        ] ]
      ] ],
      [ "Hardware Buffers", "_hardware-_buffers.html", [
        [ "The Hardware Buffer Manager", "_hardware-_buffers.html#The-Hardware-Buffer-Manager", null ],
        [ "Buffer Usage", "_hardware-_buffers.html#Buffer-Usage", null ],
        [ "Shadow Buffers", "_hardware-_buffers.html#Shadow-Buffers", null ],
        [ "Data Transfer", "_hardware-_buffers.html#Data-Transfer", [
          [ "writeData and readData", "_hardware-_buffers.html#autotoc_md240", null ],
          [ "Locking buffers", "_hardware-_buffers.html#Locking-buffers", [
            [ "Lock parameters", "_hardware-_buffers.html#autotoc_md241", null ]
          ] ]
        ] ],
        [ "Practical Buffer Tips", "_hardware-_buffers.html#Practical-Buffer-Tips", [
          [ "Vulkan specific notes", "_hardware-_buffers.html#autotoc_md242", null ]
        ] ],
        [ "Hardware Vertex Buffers", "_hardware-_buffers.html#Hardware-Vertex-Buffers", [
          [ "The VertexData class", "_hardware-_buffers.html#The-VertexData-class", null ],
          [ "Vertex Declarations", "_hardware-_buffers.html#Vertex-Declarations", null ],
          [ "Vertex Buffer Bindings", "_hardware-_buffers.html#Vertex-Buffer-Bindings", null ],
          [ "Creating the Vertex Buffer", "_hardware-_buffers.html#Creating-the-Vertex-Buffer", null ],
          [ "Binding the Vertex Buffer", "_hardware-_buffers.html#Binding-the-Vertex-Buffer", null ],
          [ "Updating Vertex Buffers", "_hardware-_buffers.html#Updating-Vertex-Buffers", null ]
        ] ],
        [ "Hardware Index Buffers", "_hardware-_buffers.html#Hardware-Index-Buffers", [
          [ "The IndexData class", "_hardware-_buffers.html#The-IndexData-class", null ],
          [ "Creating an Index Buffer", "_hardware-_buffers.html#autotoc_md243", null ],
          [ "Updating Index Buffers", "_hardware-_buffers.html#Updating-Index-Buffers", null ]
        ] ],
        [ "Hardware Pixel Buffers", "_hardware-_buffers.html#Hardware-Pixel-Buffers", [
          [ "Pixel boxes", "_hardware-_buffers.html#Pixel-boxes", null ],
          [ "Updating Pixel Buffers", "_hardware-_buffers.html#Updating-Pixel-Buffers", [
            [ "Blit from memory", "_hardware-_buffers.html#blitFromMemory", null ],
            [ "Direct memory locking", "_hardware-_buffers.html#Direct-memory-locking", null ]
          ] ]
        ] ],
        [ "Textures", "_hardware-_buffers.html#Textures", [
          [ "Creating a texture", "_hardware-_buffers.html#Creating-a-texture", null ],
          [ "Getting a PixelBuffer", "_hardware-_buffers.html#Getting-a-PixelBuffer", null ],
          [ "Cube map textures", "_hardware-_buffers.html#Cube-map-textures", null ],
          [ "Pixel Formats", "_hardware-_buffers.html#Pixel-Formats", [
            [ "Colour channels", "_hardware-_buffers.html#Colour-channels", null ]
          ] ]
        ] ]
      ] ],
      [ "Shadows", "_shadows.html", [
        [ "Enabling shadows", "_shadows.html#Enabling-shadows", null ],
        [ "Opting out of shadows", "_shadows.html#Opting-out-of-shadows", null ],
        [ "Stencil Shadows", "_shadows.html#Stencil-Shadows", [
          [ "CPU Overhead", "_shadows.html#autotoc_md35", null ],
          [ "Extrusion distance", "_shadows.html#autotoc_md36", null ],
          [ "Camera far plane positioning", "_shadows.html#autotoc_md37", null ],
          [ "Mesh edge lists", "_shadows.html#autotoc_md38", null ],
          [ "The Silhouette Edge", "_shadows.html#autotoc_md39", null ],
          [ "Be realistic", "_shadows.html#autotoc_md40", null ],
          [ "Stencil optimisations performed by Ogre", "_shadows.html#autotoc_md41", null ]
        ] ],
        [ "Texture-based Shadows", "_shadows.html#Texture_002dbased-Shadows", [
          [ "Directional Lights", "_shadows.html#autotoc_md42", null ],
          [ "Spotlights", "_shadows.html#autotoc_md43", null ],
          [ "Point Lights", "_shadows.html#autotoc_md44", null ],
          [ "Shadow Casters and Shadow Receivers", "_shadows.html#autotoc_md45", null ]
        ] ],
        [ "Configuring Texture Shadows", "_shadows.html#Configuring-Texture-Shadows", [
          [ "Maximum number of shadow textures", "_shadows.html#autotoc_md46", null ],
          [ "Shadow texture size", "_shadows.html#autotoc_md47", null ],
          [ "Shadow far distance", "_shadows.html#autotoc_md48", null ],
          [ "Shadow texture offset (Directional Lights)", "_shadows.html#autotoc_md49", null ],
          [ "Shadow fade settings", "_shadows.html#autotoc_md50", null ]
        ] ],
        [ "Texture shadows and vertex / fragment programs", "_shadows.html#texture_shadows_and_shaders", [
          [ "Custom shadow camera setups", "_shadows.html#autotoc_md51", null ],
          [ "Shadow texture Depth Buffer sharing", "_shadows.html#autotoc_md52", null ],
          [ "Integrated Texture Shadows", "_shadows.html#Integrated-Texture-Shadows", null ]
        ] ],
        [ "Modulative Shadows", "_shadows.html#Modulative-Shadows", [
          [ "Shadow Colour", "_shadows.html#autotoc_md53", null ]
        ] ],
        [ "Additive Light Masking", "_shadows.html#Additive-Light-Masking", [
          [ "Manually Categorising Illumination Passes", "_shadows.html#Manually-Categorising-Illumination-Passes", null ],
          [ "Pass Classification and Vertex Programs", "_shadows.html#Pass-Classification-and-Vertex-Programs", null ],
          [ "Static Lighting", "_shadows.html#Static-Lighting", null ]
        ] ]
      ] ],
      [ "Animation", "_animation.html", [
        [ "Animation State", "_animation.html#Animation-State", null ],
        [ "Skeletal Animation", "_animation.html#Skeletal-Animation", null ],
        [ "SceneNode Animation", "_animation.html#SceneNode-Animation", null ],
        [ "Vertex Animation", "_animation.html#Vertex-Animation", [
          [ "Why two subtypes?", "_animation.html#autotoc_md54", null ],
          [ "Subtype applies per track", "_animation.html#autotoc_md55", null ],
          [ "Vertex buffer arrangements", "_animation.html#autotoc_md56", null ],
          [ "Morph Animation", "_animation.html#Morph-Animation", null ],
          [ "Pose Animation", "_animation.html#Pose-Animation", null ],
          [ "Combining Skeletal and Vertex Animation", "_animation.html#Combining-Skeletal-and-Vertex-Animation", [
            [ "Combined Hardware Skinning", "_animation.html#Combined-Hardware-Skinning", null ],
            [ "Submesh Splits", "_animation.html#Submesh-Splits", null ]
          ] ]
        ] ],
        [ "Numeric Value Animation", "_animation.html#Numeric-Value-Animation", [
          [ "AnimableObject", "_animation.html#autotoc_md57", null ],
          [ "AnimableValue", "_animation.html#autotoc_md58", null ]
        ] ]
      ] ],
      [ "Instancing", "_instancing.html", [
        [ "Static Geometry", "_instancing.html#Static-Geometry", null ],
        [ "Instance Manager", "_instancing.html#Instance-Manager", null ],
        [ "Static Geometry vs Instancing", "_instancing.html#Static-Geometry-vs-Instancing", null ],
        [ "Instancing User-Guide", "_what_is_instancing.html", [
          [ "Instances per batch", "_what_is_instancing.html#InstancesPerBatch", null ],
          [ "Techniques", "_what_is_instancing.html#InstancingTechniques", [
            [ "ShaderBased", "_what_is_instancing.html#InstancingTechniquesShaderBased", null ],
            [ "VTF (Software)", "_what_is_instancing.html#InstancingTechniquesVTFSoftware", null ],
            [ "HW VTF", "_what_is_instancing.html#InstancingTechniquesHWVTF", [
              [ "HW VTF LUT", "_what_is_instancing.html#InstancingTechniquesHW", null ]
            ] ],
            [ "HW Basic", "_what_is_instancing.html#InstancingTechniquesHWBasic", null ]
          ] ],
          [ "Custom parameters", "_what_is_instancing.html#InstancingCustomParameters", null ],
          [ "Supporting multiple submeshes", "_what_is_instancing.html#InstancingMultipleSubmeshes", null ],
          [ "Defragmenting batches", "_what_is_instancing.html#InstancingDefragmentingBatches", [
            [ "What is batch fragmentation?", "_what_is_instancing.html#InstancingDefragmentingBatchesIntro", null ],
            [ "Prevention: Avoiding fragmentation", "_what_is_instancing.html#InstancingDefragmentingBatchesPrevention", null ],
            [ "Cure: Defragmenting on the fly", "_what_is_instancing.html#InstancingDefragmentingBatchesOnTheFly", null ]
          ] ],
          [ "Troubleshooting", "_what_is_instancing.html#InstancingTroubleshooting", null ]
        ] ]
      ] ],
      [ "Cross-platform Shaders", "_cross-platform-_shaders.html", [
        [ "Built-in defines", "_cross-platform-_shaders.html#autotoc_md204", null ],
        [ "Cross-platform macros", "_cross-platform-_shaders.html#OgreUnifiedShader", null ],
        [ "Uber shader tips", "_cross-platform-_shaders.html#autotoc_md205", null ]
      ] ]
    ] ],
    [ "Tutorials", "tutorials.html", [
      [ "Guide to building OGRE", "building-ogre.html", [
        [ "What is CMake?", "building-ogre.html#autotoc_md259", null ],
        [ "Preparing the build environment", "building-ogre.html#autotoc_md260", null ],
        [ "Getting dependencies", "building-ogre.html#autotoc_md261", [
          [ "Linux", "building-ogre.html#autotoc_md262", null ],
          [ "Recommended dependencies", "building-ogre.html#autotoc_md263", null ],
          [ "Optional dependencies", "building-ogre.html#autotoc_md264", null ],
          [ "Deprecated dependencies", "building-ogre.html#autotoc_md265", null ]
        ] ],
        [ "Running CMake", "building-ogre.html#running-cmake", null ],
        [ "Building", "building-ogre.html#autotoc_md266", null ],
        [ "Installing", "building-ogre.html#installing-sdk", null ],
        [ "Installing and building via vcpkg", "building-ogre.html#autotoc_md267", null ],
        [ "Cross-Compiling", "building-ogre.html#autotoc_md268", [
          [ "Android", "building-ogre.html#autotoc_md269", null ],
          [ "WebAssembly / Emscripten", "building-ogre.html#autotoc_md270", null ],
          [ "iOS OS", "building-ogre.html#autotoc_md271", null ],
          [ "WinRT / UWP", "building-ogre.html#autotoc_md272", null ]
        ] ]
      ] ],
      [ "Setting up an OGRE project", "setup.html", [
        [ "CMake Configuration", "setup.html#cmake", null ],
        [ "Application skeleton", "setup.html#skeleton", null ],
        [ "Running your App", "setup.html#setupRunning", [
          [ "Configuration Files", "setup.html#autotoc_md307", [
            [ "plugins.cfg", "setup.html#autotoc_md308", null ],
            [ "resources.cfg", "setup.html#autotoc_md309", null ],
            [ "ogre.cfg", "setup.html#autotoc_md310", null ]
          ] ]
        ] ]
      ] ],
      [ "Your First Scene", "tut__first_scene.html", [
        [ "How Ogre Works", "tut__first_scene.html#howogreworks", [
          [ "SceneManager", "tut__first_scene.html#scenemanager", null ],
          [ "SceneNode", "tut__first_scene.html#SceneNode", null ],
          [ "Entity", "tut__first_scene.html#Entity", null ]
        ] ],
        [ "Setting Up the Scene", "tut__first_scene.html#SettingUptheScene", null ],
        [ "Coordinates Systems", "tut__first_scene.html#CoordinatesSystems", null ],
        [ "Adding Another Entity", "tut__first_scene.html#AddingAnotherEntity", null ],
        [ "More About Entities", "tut__first_scene.html#MoreAboutEntities", null ],
        [ "More About SceneNodes", "tut__first_scene.html#MoreAboutSceneNodes", null ],
        [ "Changing An Entity's Scale", "tut__first_scene.html#ChangingAnEntitysScale", null ],
        [ "Rotating An Entity", "tut__first_scene.html#RotatingAnEntity", null ],
        [ "Plugins", "tut__first_scene.html#TheOgreEnvironment", [
          [ "Testing vs Release", "tut__first_scene.html#autotoc_md15", null ]
        ] ],
        [ "Conclusion", "tut__first_scene.html#Conclusion1", null ]
      ] ],
      [ "Lights, Cameras, and Shadows", "tut__lights_cameras_shadows.html", [
        [ "The Ogre Camera Class", "tut__lights_cameras_shadows.html#bt2TheOgreCameraClass", null ],
        [ "Creating a Camera", "tut__lights_cameras_shadows.html#bt2CreatingaCamera", null ],
        [ "Viewports", "tut__lights_cameras_shadows.html#bt2Viewports", [
          [ "Creating a Viewport", "tut__lights_cameras_shadows.html#bt2CreatingaViewport", null ]
        ] ],
        [ "Building the Scene", "tut__lights_cameras_shadows.html#bt2BuildingtheScene", null ],
        [ "Using Shadows in Ogre", "tut__lights_cameras_shadows.html#bt2UsingShadowsinOgre", null ],
        [ "Lights", "tut__lights_cameras_shadows.html#bt2Lights", [
          [ "Creating a Light", "tut__lights_cameras_shadows.html#CreatingaLight", null ],
          [ "Creating More Lights", "tut__lights_cameras_shadows.html#CreatingMoreLights", null ]
        ] ],
        [ "Shadow Types", "tut__lights_cameras_shadows.html#ShadowTypes", null ],
        [ "Conclusion", "tut__lights_cameras_shadows.html#Conclusion2", null ]
      ] ],
      [ "Terrain, Sky and Fog", "tut__terrain_sky_fog.html", [
        [ "An Introduction to Terrain", "tut__terrain_sky_fog.html#tut_terrain", [
          [ "Setting Up the Camera", "tut__terrain_sky_fog.html#autotoc_md16", null ],
          [ "Setting Up a Light", "tut__terrain_sky_fog.html#autotoc_md17", null ],
          [ "Loading overview", "tut__terrain_sky_fog.html#bt3Overview", null ],
          [ "Appearance", "tut__terrain_sky_fog.html#bt3Appearance", null ],
          [ "Merging textures", "tut__terrain_sky_fog.html#bt3MergingTextures", null ],
          [ "Defining a terrain chunk", "tut__terrain_sky_fog.html#bt3TerrainChunk", null ],
          [ "Loading a heightmap", "tut__terrain_sky_fog.html#bt3Heightmap", null ],
          [ "Height based blending", "tut__terrain_sky_fog.html#bt3Blendmap", null ],
          [ "Storing changes", "tut__terrain_sky_fog.html#bt3StoringChanges", null ],
          [ "Loading Label", "tut__terrain_sky_fog.html#bt3LoadingLabel", null ]
        ] ],
        [ "Simulating a sky", "tut__terrain_sky_fog.html#bt3sky", [
          [ "SkyBoxes", "tut__terrain_sky_fog.html#autotoc_md18", null ],
          [ "SkyDomes", "tut__terrain_sky_fog.html#autotoc_md19", null ],
          [ "SkyPlanes", "tut__terrain_sky_fog.html#autotoc_md20", null ]
        ] ],
        [ "Fog", "tut__terrain_sky_fog.html#tut_fog", [
          [ "Adding Fog to Our Scene", "tut__terrain_sky_fog.html#autotoc_md21", null ]
        ] ],
        [ "Conclusion", "tut__terrain_sky_fog.html#conclusion3", null ]
      ] ],
      [ "Working with Python", "working-with-numpy.html", [
        [ "HighPy: A High-Level API", "working-with-numpy.html#python-highpy", null ],
        [ "NumPy Interoperability", "working-with-numpy.html#python-numpy", null ],
        [ "Running Background Threads", "working-with-numpy.html#python-background-threads", null ]
      ] ],
      [ "Trays GUI System", "trays.html", [
        [ "Trays", "trays.html#trays-1", null ],
        [ "TrayManager", "trays.html#traymanager", [
          [ "The Cursor", "trays.html#autotoc_md315", null ],
          [ "The Backdrop", "trays.html#autotoc_md316", null ]
        ] ],
        [ "Widgets", "trays.html#widgets", [
          [ "Button", "trays.html#autotoc_md317", null ],
          [ "TextBox", "trays.html#autotoc_md318", null ],
          [ "SelectMenu", "trays.html#autotoc_md319", null ],
          [ "Label", "trays.html#autotoc_md320", null ],
          [ "Separator", "trays.html#autotoc_md321", null ],
          [ "Slider", "trays.html#autotoc_md322", null ],
          [ "ParamsPanel", "trays.html#autotoc_md323", null ],
          [ "CheckBox", "trays.html#autotoc_md324", null ],
          [ "DecorWidget", "trays.html#autotoc_md325", null ],
          [ "ProgressBar", "trays.html#autotoc_md326", null ],
          [ "The Null Tray", "trays.html#autotoc_md327", null ]
        ] ],
        [ "Special Widgets", "trays.html#special-widgets", [
          [ "Frame Stats", "trays.html#autotoc_md328", null ],
          [ "Logo", "trays.html#autotoc_md329", null ],
          [ "Loading Bar", "trays.html#autotoc_md330", null ],
          [ "Information Dialog", "trays.html#autotoc_md331", null ],
          [ "Question Dialog", "trays.html#autotoc_md332", null ]
        ] ],
        [ "TrayListener", "trays.html#tray-listener", null ],
        [ "Things to Try", "trays.html#things-to-try", null ]
      ] ],
      [ "Volume Component", "volume.html", [
        [ "How to use it", "volume.html#howto", null ],
        [ "Manual creation of a CSG-Tree", "volume.html#creation", null ],
        [ "Getting the triangles of the chunks", "volume.html#triangles", null ],
        [ "Intersecting a ray with a volume", "volume.html#intersecting", null ],
        [ "Editing a Volume made from a GridSource", "volume.html#editing", null ]
      ] ],
      [ "Automatic Mesh LOD Generator", "meshlod-generator.html", [
        [ "How to use it for non-programmers", "meshlod-generator.html#meshlod-nonprog", null ],
        [ "How to use it for programmers", "meshlod-generator.html#meshlod-prog", null ],
        [ "Extending MeshLodGenerator", "meshlod-generator.html#meshlod-extend", [
          [ "Replacing a component", "meshlod-generator.html#autotoc_md301", null ]
        ] ]
      ] ],
      [ "DotScene Overview", "dotscene_overview.html", [
        [ "What is DotScene?", "dotscene_overview.html#autotoc_md9", null ],
        [ "User Data", "dotscene_overview.html#autotoc_md10", null ],
        [ "How to use DotScene", "dotscene_overview.html#autotoc_md11", null ],
        [ "Instancing", "dotscene_overview.html#autotoc_md12", [
          [ "Static Geometry", "dotscene_overview.html#autotoc_md13", null ],
          [ "Instance Manager", "dotscene_overview.html#autotoc_md14", null ]
        ] ]
      ] ],
      [ "Manual mesh creation", "manual-mesh-creation.html", [
        [ "Using Manual Object", "manual-mesh-creation.html#autotoc_md298", [
          [ "Example", "manual-mesh-creation.html#autotoc_md299", null ]
        ] ],
        [ "Using vertex and index buffers directly", "manual-mesh-creation.html#autotoc_md300", null ]
      ] ],
      [ "Static Geometry", "tut__static_geom.html", [
        [ "Blades of grass", "tut__static_geom.html#autotoc_md311", null ],
        [ "A field of grass", "tut__static_geom.html#autotoc_md312", null ],
        [ "Animating StaticGeometry", "tut__static_geom.html#autotoc_md313", null ],
        [ "Advanced Object Batching", "tut__static_geom.html#autotoc_md314", null ]
      ] ],
      [ "Using the Profiler", "profiler.html", [
        [ "Reading the Display", "profiler.html#profRead", null ],
        [ "Features", "profiler.html#profFeatures", [
          [ "Disabling the Profiler", "profiler.html#autotoc_md302", null ],
          [ "Disabling Individual Profiles", "profiler.html#autotoc_md303", null ],
          [ "Analyzing Application State", "profiler.html#autotoc_md304", null ],
          [ "Logging Results", "profiler.html#autotoc_md305", null ],
          [ "Changing the Frequency of Updating the Display", "profiler.html#autotoc_md306", null ]
        ] ],
        [ "Performance and Accuracy", "profiler.html#profAccuracy", null ],
        [ "Remotery Backend", "profiler.html#profRemotery", null ],
        [ "Release Version Considerations", "profiler.html#profRelmode", null ]
      ] ],
      [ "Reversed Depth", "reversed-depth.html", null ],
      [ "External Texture Sources", "_external-_texture-_sources.html", [
        [ "What Is An External Texture Source?", "_external-_texture-_sources.html#autotoc_md293", null ],
        [ "ExternalTextureSource Class", "_external-_texture-_sources.html#autotoc_md294", null ],
        [ "ExternalTextureSourceManager Class", "_external-_texture-_sources.html#autotoc_md295", null ],
        [ "Texture Source Material Script", "_external-_texture-_sources.html#autotoc_md296", null ],
        [ "Simplified Diagram of Process", "_external-_texture-_sources.html#autotoc_md297", null ]
      ] ],
      [ "Background Resource Loading", "background-resource.html", null ],
      [ "Shadow Mapping in Ogre", "_shadow_mapping_ogre.html", [
        [ "Introduction to the Shadow Mapping Algorithm", "_shadow_mapping_ogre.html#ShadowMappingIntro", [
          [ "Formalism", "_shadow_mapping_ogre.html#sm_formalism", null ],
          [ "Depth Biasing", "_shadow_mapping_ogre.html#DepthBias", null ],
          [ "Percentage Closest Filtering", "_shadow_mapping_ogre.html#sm_pcm", null ]
        ] ],
        [ "Variants", "_shadow_mapping_ogre.html#sm_variants", [
          [ "Storing Additional Info", "_shadow_mapping_ogre.html#sm_additional_info", null ],
          [ "Breaking up Shadow Frusta", "_shadow_mapping_ogre.html#sm_breaking_frusta", null ],
          [ "Playing with Projection Matrices", "_shadow_mapping_ogre.html#sect_planeopt", null ]
        ] ],
        [ "Theory and Analysis", "_shadow_mapping_ogre.html#sm_theory", [
          [ "(Non) Optimality of Logarithmic Shadow Maps", "_shadow_mapping_ogre.html#sm_nonopt", null ],
          [ "Sampling Aliasing versus Depth Precision Aliasing", "_shadow_mapping_ogre.html#sm_aliasing", null ],
          [ "Projective versus Perspective Aliasing", "_shadow_mapping_ogre.html#sm_proj_aliasing", null ]
        ] ],
        [ "Implementation", "_shadow_mapping_ogre.html#Implementation", [
          [ "Caster", "_shadow_mapping_ogre.html#autotoc_md59", null ],
          [ "Receiver", "_shadow_mapping_ogre.html#autotoc_md60", null ],
          [ "Debugging Shadows", "_shadow_mapping_ogre.html#autotoc_md61", null ],
          [ "Improving Shadow Quality", "_shadow_mapping_ogre.html#autotoc_md62", null ]
        ] ]
      ] ],
      [ "Deferred Shading", "deferred.html", [
        [ "What is Deferred Shading?", "deferred.html#what", [
          [ "Deferred Shading Advantages", "deferred.html#autotoc_md273", null ],
          [ "Deferred Shading Disadvantages", "deferred.html#autotoc_md274", null ]
        ] ],
        [ "Creating the G-Buffer", "deferred.html#creating", [
          [ "Deciding on the GBuffer format", "deferred.html#autotoc_md275", null ],
          [ "Preparing the objects for G-Buffer rendering", "deferred.html#autotoc_md276", [
            [ "Inspect the classic technique", "deferred.html#autotoc_md277", null ],
            [ "Generate the G-Buffer technique", "deferred.html#autotoc_md278", null ],
            [ "Add the G-Buffer technique to the original material", "deferred.html#autotoc_md279", null ],
            [ "Putting it all together", "deferred.html#autotoc_md280", null ]
          ] ],
          [ "Overriding the automatic process", "deferred.html#autotoc_md281", null ],
          [ "Seeing it in action", "deferred.html#autotoc_md282", null ]
        ] ],
        [ "Lighting the scene", "deferred.html#autotoc_md283", [
          [ "Rendering the light geometry", "deferred.html#lightgeom", [
            [ "Prepare ambient colour and rebuild original depth buffer", "deferred.html#autotoc_md284", null ],
            [ "Render the light geometries", "deferred.html#autotoc_md285", null ],
            [ "Rendering shadow casting lights", "deferred.html#autotoc_md286", null ],
            [ "Putting it all together", "deferred.html#autotoc_md287", null ],
            [ "Seeing it in action", "deferred.html#autotoc_md288", null ]
          ] ]
        ] ],
        [ "Post Processing", "deferred.html#post", [
          [ "Screen Space Ambient Occlusion", "deferred.html#autotoc_md289", null ]
        ] ],
        [ "Integration in real projects", "deferred.html#realprojects", [
          [ "Integration steps", "deferred.html#autotoc_md290", null ],
          [ "Adapting the framework", "deferred.html#autotoc_md291", null ],
          [ "Adding features to the framework", "deferred.html#autotoc_md292", null ]
        ] ],
        [ "Summary", "deferred.html#summary", [
          [ "Further reading", "deferred.html#further", null ]
        ] ]
      ] ],
      [ "Using the PCZ Scene Manager", "pczscenemanager.html", [
        [ "LOADING & INITIALIZATION", "pczscenemanager.html#autotoc_md2", null ],
        [ "CREATING ZONES", "pczscenemanager.html#autotoc_md3", null ],
        [ "CREATING PORTALS", "pczscenemanager.html#autotoc_md4", null ],
        [ "ANTIPORTALS", "pczscenemanager.html#autotoc_md5", null ],
        [ "CREATING OBJECTS/ENTITIES", "pczscenemanager.html#autotoc_md6", null ],
        [ "SCENE QUERIES", "pczscenemanager.html#autotoc_md7", null ],
        [ "KNOWN BUGS", "pczscenemanager.html#autotoc_md8", null ]
      ] ]
    ] ],
    [ "Deprecated List", "deprecated.html", null ],
    [ "Bibliography", "citelist.html", null ],
    [ "Topics", "topics.html", "topics" ],
    [ "Namespaces", "namespaces.html", [
      [ "Namespace List", "namespaces.html", "namespaces_dup" ],
      [ "Namespace Members", "namespacemembers.html", [
        [ "All", "namespacemembers.html", "namespacemembers_dup" ],
        [ "Functions", "namespacemembers_func.html", null ],
        [ "Variables", "namespacemembers_vars.html", null ],
        [ "Typedefs", "namespacemembers_type.html", "namespacemembers_type" ],
        [ "Enumerations", "namespacemembers_enum.html", null ],
        [ "Enumerator", "namespacemembers_eval.html", "namespacemembers_eval" ]
      ] ]
    ] ],
    [ "Classes", "annotated.html", [
      [ "Class List", "annotated.html", "annotated_dup" ],
      [ "Class Index", "classes.html", null ],
      [ "Class Hierarchy", "hierarchy.html", "hierarchy" ],
      [ "Class Members", "functions.html", [
        [ "All", "functions.html", "functions_dup" ],
        [ "Functions", "functions_func.html", "functions_func" ],
        [ "Variables", "functions_vars.html", "functions_vars" ],
        [ "Typedefs", "functions_type.html", "functions_type" ],
        [ "Enumerations", "functions_enum.html", null ],
        [ "Enumerator", "functions_eval.html", "functions_eval" ],
        [ "Related Symbols", "functions_rela.html", null ]
      ] ]
    ] ],
    [ "Files", "files.html", [
      [ "File List", "files.html", "files_dup" ],
      [ "File Members", "globals.html", [
        [ "All", "globals.html", null ],
        [ "Typedefs", "globals_type.html", null ],
        [ "Enumerations", "globals_enum.html", null ],
        [ "Enumerator", "globals_eval.html", null ],
        [ "Macros", "globals_defs.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"_animation.html",
"_ogre_8h.html",
"_ogre_page_connection_8h.html",
"_ogre_quake3_types_8h.html#ab5dbbb0f6a60af03e34dc0267ca03d7a",
"_particle-_scripts.html#autotoc_md232",
"class_ogre_1_1_animation.html#a8b6ec2b9f63b32a7ef5ee3016d3c6cfb",
"class_ogre_1_1_auto_param_data_source.html#a20da1d44783a1f3454bf34671aed328e",
"class_ogre_1_1_billboard_chain.html#ae2873540c27dddd3062af468d6fa8d83",
"class_ogre_1_1_bsp_node.html#ac5702538be4c791af37cd73d2aa353a5",
"class_ogre_1_1_cg_program_1_1_cmd_args.html#acdf515557498e491df587a0191b1532c",
"class_ogre_1_1_composition_pass.html#aa20bff864069127a799a3593df0146aa",
"class_ogre_1_1_compositor_manager.html#ae9b8ca4c2a51c0720bb31393d20fe7c9",
"class_ogre_1_1_d3_d9_render_system.html#a869123f35563093de0f4fe68321b9b4b",
"class_ogre_1_1_default_zone_factory.html",
"class_ogre_1_1_entity.html#a5224496a80fa5c04e8a623794b397186",
"class_ogre_1_1_free_image_codec.html#ae74575c79ccc668087d8d9a4f837db87",
"class_ogre_1_1_gpu_program_manager.html#a1be2dd3263dfadb44abce22bf46b9dd2",
"class_ogre_1_1_gpu_program_parameters.html#a8d413edcc3750d74a698b21feae29465",
"class_ogre_1_1_hardware_pixel_buffer.html#ac22574a29056d258f3d4e5accb020da5",
"class_ogre_1_1_light.html#a8ef572fd5d20b366328cbdb0eb71b4a0",
"class_ogre_1_1_log_manager.html#ab62cc2995ec5cad0c75f7665ebf8cfb2",
"class_ogre_1_1_math.html#a157a69d3d7d991975dd3b000842156b2",
"class_ogre_1_1_mesh.html#a9f74a43715bc0916f106365965f394ed",
"class_ogre_1_1_movable_plane.html#ae6b5c820290f166708abfe707db288fe",
"class_ogre_1_1_octree_node.html#afd689904e1736d7aadccca240e0b2480",
"class_ogre_1_1_overlay_element.html#a98050c3c2f7e3bf91769e9f98c203e64",
"class_ogre_1_1_p_c_z_scene_manager.html#aa194a8264693295fcbcdd14b9e8d325c",
"class_ogre_1_1_page_manager.html#a020c01d7404878f466fa62f9bd37e1ef",
"class_ogre_1_1_particle_emitter.html#a826773806a1fc8b1e6731421dd513209",
"class_ogre_1_1_pass.html#a5485f8b014bfa01d765a442999ad9ed3",
"class_ogre_1_1_plane.html#a7f593c4a54f2d424a8f213421895715e",
"class_ogre_1_1_pose.html#a8bec32386677c48fd9a8d1add845313d",
"class_ogre_1_1_quaternion.html#a98993e6f63209a39149d132623bbf41b",
"class_ogre_1_1_r_t_shader_1_1_parameter.html#a4fa6286f097d19e92a8ed91645fbfbe7a8feeee8fd4b72d723f412ec8d7bc4f00",
"class_ogre_1_1_r_t_shader_1_1_target_render_state.html#ad89fc9a3c7ec0c73a5d7805013547c24",
"class_ogre_1_1_render_system.html#a0f882060a14f71ce307629d198a34595",
"class_ogre_1_1_render_target_listener.html#ae6e3dc0a3a760bf152513fafbf51f9b3",
"class_ogre_1_1_resource_manager.html#a2514952116c93b5b5eba051492d2caa9",
"class_ogre_1_1_sampler.html#a8486e32aaa07278882e46877b3fc3d17",
"class_ogre_1_1_scene_manager.html#aa613e5ffb08b95a90644a61ac10c9b5c",
"class_ogre_1_1_script_translator.html#a21b5a1fd7c18f7fa01e39df491088e03",
"class_ogre_1_1_small_vector_impl.html#a40bfd177bea1ec263d342c9973d0721d",
"class_ogre_1_1_static_geometry_1_1_optimised_sub_mesh_geometry.html#ad74c3d8bc57c3902ad3f1be62fc552fd",
"class_ogre_1_1_sub_mesh.html#af9bf141e65f964f131229501804940e5",
"class_ogre_1_1_terrain.html#a9cc3d8901c73b6d2b6e1613a22636801",
"class_ogre_1_1_terrain_layer_blend_map.html#aa1746e4fe6ba9de17b626c1d44f56879",
"class_ogre_1_1_texture.html#ae32d1cfbbb44d87d698aae547ecda35f",
"class_ogre_1_1_tiny_render_system.html#a2bc08704f38ec1392b0c6ad8378cf20f",
"class_ogre_1_1_vertex_element.html#af6a1db40e39a289be0533339b694d2fe",
"class_ogre_1_1_volume_1_1_simplex_noise.html",
"class_ogre_bites_1_1_input_listener_chain.html#ab69dcabeca45b0bb55af54baeb6cd2dd",
"deferred.html#autotoc_md284",
"group___animation.html#gga5b37bdf2f67384dbd81643164545f7bdaedacebfaa34fe1ed9b95efb6210e13e1",
"group___general.html#gga5d04131dcb51ec3cf219465196b6de85a8d3eb8e812449857231d35205dce797a",
"group___input.html#ggafba156da7d846753caa2c0be29f6da0da0eb561d30eb04f7403af3fd1204870b0",
"group___overlays.html#gga7d22ba70debe31488cdd62c5dc307ce9ac5c0824a886f1c3946159c1303397c68",
"group___render_system.html#ga7af5c611687492846891914dcac303d2",
"group___rs_image_codec.html",
"group___script.html#ggafb79f4c215a4ac77110efe823e94b61ea240973daf73c9191b1e285f504f98083",
"group___script.html#ggafb79f4c215a4ac77110efe823e94b61eafec4425353b7cafd7fda2a2bdb959211",
"namespacemembers_type_e.html",
"struct_ogre_1_1_gpu_logical_buffer_struct.html#adab78cf8e26f03488bda5609bed785b7",
"struct_ogre_1_1_r_t_shader_1_1_at.html#a6de1fe7642fed92774305600b0fe5093",
"struct_ogre_1_1_terrain_lod_manager_1_1_lod_info.html#a3d3eaf36cee2c6b6cb64bad691cd98f6",
"struct_ogre_1_1is_pod_like_3_01unsigned_01long_01_4.html",
"tut__terrain_sky_fog.html#tut_fog"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';