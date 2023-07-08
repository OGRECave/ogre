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
            [ "shared_params_ref", "_high-level-_programs.html#autotoc_md198", null ],
            [ "use_linear_colours", "_high-level-_programs.html#autotoc_md199", null ]
          ] ],
          [ "Declaring Shared Parameters", "_high-level-_programs.html#Declaring-Shared-Parameters", [
            [ "Hardware Support", "_high-level-_programs.html#autotoc_md200", null ]
          ] ],
          [ "Shadows and Vertex Programs", "_high-level-_programs.html#Shadows-and-Vertex-Programs", null ],
          [ "Instancing in Vertex Programs", "_high-level-_programs.html#Instancing-in-Vertex-Programs", null ],
          [ "Skeletal Animation in Vertex Programs", "_high-level-_programs.html#Skeletal-Animation-in-Vertex-Programs", null ],
          [ "Morph Animation in Vertex Programs", "_high-level-_programs.html#Morph-Animation-in-Vertex-Programs", null ],
          [ "Pose Animation in Vertex Programs", "_high-level-_programs.html#Pose-Animation-in-Vertex-Programs", null ],
          [ "Vertex Texture Fetch", "_high-level-_programs.html#Vertex-Texture-Fetch", [
            [ "Declaring the use of vertex texture fetching", "_high-level-_programs.html#autotoc_md201", null ],
            [ "DirectX9 binding limitations", "_high-level-_programs.html#autotoc_md202", null ],
            [ "Texture format limitations", "_high-level-_programs.html#autotoc_md203", null ],
            [ "Hardware limitations", "_high-level-_programs.html#autotoc_md204", null ]
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
            [ "quota", "_particle-_scripts.html#autotoc_md207", null ],
            [ "material", "_particle-_scripts.html#autotoc_md208", null ],
            [ "particle_width", "_particle-_scripts.html#autotoc_md209", null ],
            [ "particle_height", "_particle-_scripts.html#autotoc_md210", null ],
            [ "cull_each", "_particle-_scripts.html#autotoc_md211", null ],
            [ "renderer", "_particle-_scripts.html#autotoc_md212", null ],
            [ "sorted", "_particle-_scripts.html#autotoc_md213", null ],
            [ "local_space", "_particle-_scripts.html#autotoc_md214", null ],
            [ "iteration_interval", "_particle-_scripts.html#autotoc_md215", null ],
            [ "nonvisible_update_timeout", "_particle-_scripts.html#autotoc_md216", null ]
          ] ],
          [ "Billboard Renderer Attributes", "_particle-_scripts.html#Billboard-Renderer-Attributes", [
            [ "billboard_type", "_particle-_scripts.html#autotoc_md217", null ],
            [ "billboard_origin", "_particle-_scripts.html#autotoc_md218", null ],
            [ "billboard_rotation_type", "_particle-_scripts.html#autotoc_md219", null ],
            [ "common_direction", "_particle-_scripts.html#autotoc_md220", null ],
            [ "common_up_vector", "_particle-_scripts.html#autotoc_md221", null ],
            [ "point_rendering", "_particle-_scripts.html#autotoc_md222", null ],
            [ "accurate_facing", "_particle-_scripts.html#autotoc_md223", null ],
            [ "texture_sheet_size", "_particle-_scripts.html#autotoc_md224", null ]
          ] ],
          [ "Particle Emitters", "_particle-_scripts.html#Particle-Emitters", [
            [ "Emitting Emitters", "_particle-_scripts.html#Emitting-Emitters", null ],
            [ "Common Emitter Attributes", "_particle-_scripts.html#autotoc_md225", null ],
            [ "angle", "_particle-_scripts.html#autotoc_md226", null ],
            [ "colour", "_particle-_scripts.html#autotoc_md227", null ],
            [ "colour_range_start & colour_range_end", "_particle-_scripts.html#autotoc_md228", null ],
            [ "direction", "_particle-_scripts.html#autotoc_md229", null ],
            [ "direction_position_reference", "_particle-_scripts.html#autotoc_md230", null ],
            [ "emission_rate", "_particle-_scripts.html#autotoc_md231", null ],
            [ "position", "_particle-_scripts.html#autotoc_md232", null ],
            [ "velocity", "_particle-_scripts.html#autotoc_md233", null ],
            [ "velocity_min & velocity_max", "_particle-_scripts.html#autotoc_md234", null ],
            [ "time_to_live", "_particle-_scripts.html#autotoc_md235", null ],
            [ "time_to_live_min & time_to_live_max", "_particle-_scripts.html#autotoc_md236", null ],
            [ "duration", "_particle-_scripts.html#autotoc_md237", null ],
            [ "duration_min & duration_max", "_particle-_scripts.html#autotoc_md238", null ],
            [ "repeat_delay", "_particle-_scripts.html#autotoc_md239", null ],
            [ "repeat_delay_min & repeat_delay_max", "_particle-_scripts.html#autotoc_md240", null ]
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
          [ "transform_stage", "rtss.html#autotoc_md245", null ],
          [ "lighting_stage", "rtss.html#autotoc_md246", null ],
          [ "texturing_stage", "rtss.html#autotoc_md247", null ],
          [ "image_based_lighting", "rtss.html#autotoc_md248", null ],
          [ "gbuffer", "rtss.html#autotoc_md249", null ],
          [ "normal_map", "rtss.html#autotoc_md250", null ],
          [ "metal_roughness", "rtss.html#autotoc_md251", null ],
          [ "fog_stage", "rtss.html#autotoc_md252", null ],
          [ "light_count", "rtss.html#autotoc_md253", null ],
          [ "triplanarTexturing", "rtss.html#autotoc_md254", null ],
          [ "integrated_pssm4", "rtss.html#autotoc_md255", null ],
          [ "shadow_mapping", "rtss.html#shadow_mapping", null ],
          [ "hardware_skinning", "rtss.html#autotoc_md256", null ]
        ] ],
        [ "RTSS Texture Unit properties", "rtss.html#rtss_tu_props", [
          [ "normal_map", "rtss.html#normal_map", null ],
          [ "layered_blend", "rtss.html#autotoc_md257", null ],
          [ "source_modifier", "rtss.html#autotoc_md258", null ]
        ] ],
        [ "Setting properties programmatically", "rtss.html#RTSS-Props-API", null ],
        [ "System overview", "rtss.html#rtss_overview", [
          [ "Core features of the system", "rtss.html#core-feats", null ],
          [ "Controlling shader re-generation", "rtss.html#autotoc_md259", null ]
        ] ],
        [ "The RTSS in Depth", "rtss.html#rtss_indepth", [
          [ "Main components", "rtss.html#rtss__components", null ],
          [ "Initializing the system", "rtss.html#autotoc_md260", null ],
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
          [ "writeData and readData", "_hardware-_buffers.html#autotoc_md241", null ],
          [ "Locking buffers", "_hardware-_buffers.html#Locking-buffers", [
            [ "Lock parameters", "_hardware-_buffers.html#autotoc_md242", null ]
          ] ]
        ] ],
        [ "Practical Buffer Tips", "_hardware-_buffers.html#Practical-Buffer-Tips", [
          [ "Vulkan specific notes", "_hardware-_buffers.html#autotoc_md243", null ]
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
          [ "Creating an Index Buffer", "_hardware-_buffers.html#autotoc_md244", null ],
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
        [ "Explicit Instance Manager", "_instancing.html#Instance-Manager", null ],
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
        [ "Built-in defines", "_cross-platform-_shaders.html#autotoc_md205", null ],
        [ "Cross-platform macros", "_cross-platform-_shaders.html#OgreUnifiedShader", null ],
        [ "Uber shader tips", "_cross-platform-_shaders.html#autotoc_md206", null ]
      ] ]
    ] ],
    [ "Tutorials", "tutorials.html", [
      [ "Guide to building OGRE", "building-ogre.html", [
        [ "What is CMake?", "building-ogre.html#autotoc_md261", null ],
        [ "Preparing the build environment", "building-ogre.html#autotoc_md262", null ],
        [ "Getting dependencies", "building-ogre.html#autotoc_md263", [
          [ "Linux", "building-ogre.html#autotoc_md264", null ],
          [ "Recommended dependencies", "building-ogre.html#autotoc_md265", null ],
          [ "Optional dependencies", "building-ogre.html#autotoc_md266", null ],
          [ "Deprecated dependencies", "building-ogre.html#autotoc_md267", null ]
        ] ],
        [ "Running CMake", "building-ogre.html#running-cmake", null ],
        [ "Building", "building-ogre.html#autotoc_md268", null ],
        [ "Installing", "building-ogre.html#installing-sdk", null ],
        [ "Installing and building via vcpkg", "building-ogre.html#autotoc_md269", null ],
        [ "Cross-Compiling", "building-ogre.html#autotoc_md270", [
          [ "Android", "building-ogre.html#autotoc_md271", null ],
          [ "WebAssembly / Emscripten", "building-ogre.html#autotoc_md272", null ],
          [ "iOS OS", "building-ogre.html#autotoc_md273", null ],
          [ "WinRT / UWP", "building-ogre.html#autotoc_md274", null ]
        ] ]
      ] ],
      [ "Setting up an OGRE project", "setup.html", [
        [ "CMake Configuration", "setup.html#cmake", null ],
        [ "Application skeleton", "setup.html#skeleton", null ],
        [ "Running your App", "setup.html#setupRunning", [
          [ "Configuration Files", "setup.html#autotoc_md309", [
            [ "plugins.cfg", "setup.html#autotoc_md310", null ],
            [ "resources.cfg", "setup.html#autotoc_md311", null ],
            [ "ogre.cfg", "setup.html#autotoc_md312", null ]
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
          [ "The Cursor", "trays.html#autotoc_md317", null ],
          [ "The Backdrop", "trays.html#autotoc_md318", null ]
        ] ],
        [ "Widgets", "trays.html#widgets", [
          [ "Button", "trays.html#autotoc_md319", null ],
          [ "TextBox", "trays.html#autotoc_md320", null ],
          [ "SelectMenu", "trays.html#autotoc_md321", null ],
          [ "Label", "trays.html#autotoc_md322", null ],
          [ "Separator", "trays.html#autotoc_md323", null ],
          [ "Slider", "trays.html#autotoc_md324", null ],
          [ "ParamsPanel", "trays.html#autotoc_md325", null ],
          [ "CheckBox", "trays.html#autotoc_md326", null ],
          [ "DecorWidget", "trays.html#autotoc_md327", null ],
          [ "ProgressBar", "trays.html#autotoc_md328", null ],
          [ "The Null Tray", "trays.html#autotoc_md329", null ]
        ] ],
        [ "Special Widgets", "trays.html#special-widgets", [
          [ "Frame Stats", "trays.html#autotoc_md330", null ],
          [ "Logo", "trays.html#autotoc_md331", null ],
          [ "Loading Bar", "trays.html#autotoc_md332", null ],
          [ "Information Dialog", "trays.html#autotoc_md333", null ],
          [ "Question Dialog", "trays.html#autotoc_md334", null ]
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
          [ "Replacing a component", "meshlod-generator.html#autotoc_md303", null ]
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
        [ "Using Manual Object", "manual-mesh-creation.html#autotoc_md300", [
          [ "Example", "manual-mesh-creation.html#autotoc_md301", null ]
        ] ],
        [ "Using vertex and index buffers directly", "manual-mesh-creation.html#autotoc_md302", null ]
      ] ],
      [ "Static Geometry", "tut__static_geom.html", [
        [ "Blades of grass", "tut__static_geom.html#autotoc_md313", null ],
        [ "A field of grass", "tut__static_geom.html#autotoc_md314", null ],
        [ "Animating StaticGeometry", "tut__static_geom.html#autotoc_md315", null ],
        [ "Advanced Object Batching", "tut__static_geom.html#autotoc_md316", null ]
      ] ],
      [ "Using the Profiler", "profiler.html", [
        [ "Reading the Display", "profiler.html#profRead", null ],
        [ "Features", "profiler.html#profFeatures", [
          [ "Disabling the Profiler", "profiler.html#autotoc_md304", null ],
          [ "Disabling Individual Profiles", "profiler.html#autotoc_md305", null ],
          [ "Analyzing Application State", "profiler.html#autotoc_md306", null ],
          [ "Logging Results", "profiler.html#autotoc_md307", null ],
          [ "Changing the Frequency of Updating the Display", "profiler.html#autotoc_md308", null ]
        ] ],
        [ "Performance and Accuracy", "profiler.html#profAccuracy", null ],
        [ "Remotery Backend", "profiler.html#profRemotery", null ],
        [ "Release Version Considerations", "profiler.html#profRelmode", null ]
      ] ],
      [ "Reversed Depth", "reversed-depth.html", null ],
      [ "External Texture Sources", "_external-_texture-_sources.html", [
        [ "What Is An External Texture Source?", "_external-_texture-_sources.html#autotoc_md295", null ],
        [ "ExternalTextureSource Class", "_external-_texture-_sources.html#autotoc_md296", null ],
        [ "ExternalTextureSourceManager Class", "_external-_texture-_sources.html#autotoc_md297", null ],
        [ "Texture Source Material Script", "_external-_texture-_sources.html#autotoc_md298", null ],
        [ "Simplified Diagram of Process", "_external-_texture-_sources.html#autotoc_md299", null ]
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
          [ "Deferred Shading Advantages", "deferred.html#autotoc_md275", null ],
          [ "Deferred Shading Disadvantages", "deferred.html#autotoc_md276", null ]
        ] ],
        [ "Creating the G-Buffer", "deferred.html#creating", [
          [ "Deciding on the GBuffer format", "deferred.html#autotoc_md277", null ],
          [ "Preparing the objects for G-Buffer rendering", "deferred.html#autotoc_md278", [
            [ "Inspect the classic technique", "deferred.html#autotoc_md279", null ],
            [ "Generate the G-Buffer technique", "deferred.html#autotoc_md280", null ],
            [ "Add the G-Buffer technique to the original material", "deferred.html#autotoc_md281", null ],
            [ "Putting it all together", "deferred.html#autotoc_md282", null ]
          ] ],
          [ "Overriding the automatic process", "deferred.html#autotoc_md283", null ],
          [ "Seeing it in action", "deferred.html#autotoc_md284", null ]
        ] ],
        [ "Lighting the scene", "deferred.html#autotoc_md285", [
          [ "Rendering the light geometry", "deferred.html#lightgeom", [
            [ "Prepare ambient colour and rebuild original depth buffer", "deferred.html#autotoc_md286", null ],
            [ "Render the light geometries", "deferred.html#autotoc_md287", null ],
            [ "Rendering shadow casting lights", "deferred.html#autotoc_md288", null ],
            [ "Putting it all together", "deferred.html#autotoc_md289", null ],
            [ "Seeing it in action", "deferred.html#autotoc_md290", null ]
          ] ]
        ] ],
        [ "Post Processing", "deferred.html#post", [
          [ "Screen Space Ambient Occlusion", "deferred.html#autotoc_md291", null ]
        ] ],
        [ "Integration in real projects", "deferred.html#realprojects", [
          [ "Integration steps", "deferred.html#autotoc_md292", null ],
          [ "Adapting the framework", "deferred.html#autotoc_md293", null ],
          [ "Adding features to the framework", "deferred.html#autotoc_md294", null ]
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
"_numpy_8py.html",
"_ogre_page_8h.html",
"_ogre_quake3_types_8h.html#ab13a5a0938c6d5f3a005ef3fdf461781",
"_particle-_scripts.html#autotoc_md232",
"class_ogre_1_1_animation.html#a8af56d49ff39965b5544f4714f60921e",
"class_ogre_1_1_auto_param_data_source.html#a1b250c601f5c295a56cb9a3851b9f131",
"class_ogre_1_1_billboard_chain.html#ac08f3a22eb7f8526824158877e41bff5",
"class_ogre_1_1_bsp_node.html#aa10efd90b923c891eba69bad3b06d450",
"class_ogre_1_1_cg_program.html#a65a86c6f911a8ac4b0c6d7366fb44f26",
"class_ogre_1_1_composition_pass.html#a6e14a28e603b902ee0794f78912f329f",
"class_ogre_1_1_compositor_manager.html#a774da533de0e9dc7263551a37136a2c6",
"class_ogre_1_1_d3_d9_render_system.html#a62a49b9f66b36b9be215f88e190366a2",
"class_ogre_1_1_default_zone.html#ac4b3f64f42180cb1cb3aeb01be947bcb",
"class_ogre_1_1_entity.html#a39f729c2f6f30d659a101d5c967f9e9b",
"class_ogre_1_1_frame_time_controller_value.html#ab4c6b8b58ce0325c0a54e749ba3b48e1",
"class_ogre_1_1_gpu_program.html#af420c447afb13728b4ddfcb8d8d7d812",
"class_ogre_1_1_gpu_program_parameters.html#a73446046fdeb72e5ffc137b4b7407415",
"class_ogre_1_1_hardware_pixel_buffer.html",
"class_ogre_1_1_light.html#a63ea738c19ed36e7c0a160585095e9ef",
"class_ogre_1_1_log_listener.html#af554fab1d935fecbc80531e06cea5c1a",
"class_ogre_1_1_material_serializer_1_1_listener.html#a12692ae5f00a3012d83342ab7539dadd",
"class_ogre_1_1_mesh.html#a87ffd99faf08ec80bd6932ac77381f3f",
"class_ogre_1_1_movable_object_factory.html#aae05ac12370740edd291355fa2865b9b",
"class_ogre_1_1_octree_node.html",
"class_ogre_1_1_overlay_element.html#a712333ce516ee70e1ad7e72fd4bf0336",
"class_ogre_1_1_p_c_z_scene_manager.html#a7858690b21eaad51929e3c02086a17ca",
"class_ogre_1_1_page_content_collection_factory.html#a28360612aa20d5ca8ad8545c96dbfca3",
"class_ogre_1_1_particle_emitter.html#a6607b5a35ab594e8e5e01df766cd5462",
"class_ogre_1_1_pass.html#a48a2a5f63407356c4e080cbea73b651b",
"class_ogre_1_1_plane.html#a1530cc182a2a783e072eabe44f536149",
"class_ogre_1_1_pose.html",
"class_ogre_1_1_quaternion.html#a58a45c5dd46ceee8dbf81dfced570dbe",
"class_ogre_1_1_r_t_shader_1_1_parameter.html#a4fa6286f097d19e92a8ed91645fbfbe7a4d6be8e5c18ecc447fcb9bdf50e0af08",
"class_ogre_1_1_r_t_shader_1_1_sub_render_state_factory.html#a49db795c9a9223f0fc68b8de768ba8ec",
"class_ogre_1_1_render_queue_listener.html#a0a6a53d54e3215fc2bf2c877fedd47d6",
"class_ogre_1_1_render_target.html#adebc1792e75341f37a44c14294b84441",
"class_ogre_1_1_resource_group_manager.html#ae9b9256ef21eefb36c466ecf7b92111a",
"class_ogre_1_1_s_t_b_i_image_codec.html#a6d61006387ccb5c517aec9c397444ac0",
"class_ogre_1_1_scene_manager.html#a9c28c0635389f53023a58304597fb2b0",
"class_ogre_1_1_script_compiler_manager.html#a5dc660484560a9f099c8bd80f947e504",
"class_ogre_1_1_small_vector_3_01_t_00_010_01_4.html#ace0be9a57bd0aff5c735415161a70414",
"class_ogre_1_1_static_geometry_1_1_material_bucket.html#a59961706795c7ad082210c65748d0c83",
"class_ogre_1_1_sub_mesh.html#aad428fa6595c7fb01ef7543e5d8d6454",
"class_ogre_1_1_terrain.html#a86553dfa0e4c39018d8ee23f96fe588d",
"class_ogre_1_1_terrain_group.html#afce782965e3d453f9ceb97ae95780e6f",
"class_ogre_1_1_texture.html#a7eded50cabb3e9dbeeee531c168016da",
"class_ogre_1_1_timer.html#a81a04caabee9ee5462b68e41d12117e6",
"class_ogre_1_1_vertex_element.html#a0c6ec880969e888a995e5e7bcae73e6f",
"class_ogre_1_1_volume_1_1_octree_node.html#ad0e8bfe8ac3e332a10a94de61f84e584",
"class_ogre_bites_1_1_decor_widget.html#aca1aaf1510557e6c9443f58457964cda",
"class_ogre_bites_1_1_window_event_utilities.html#a9cea96078c3b412c9bcacca1b1dc3aac",
"group___animation.html#ga8c89213416d55c0252c9b6b86783efa0",
"group___general.html#gga4b8cfe4e77d7d21264a6497c06ed924aa631b8a9e92ad1186744a5fa9521b08dc",
"group___input.html#gga07f9567987aa7747ca26ca5a1d3f69a5acd17613d5b4d5b41c8daa8813afeb91a",
"group___overlays.html#ga923def772455064a18eaa248c04627a0",
"group___r_t_shader.html#gga7d20b2397c3eab2b52ec405863c6f274afdb7e1c00705255e5a10459e723e9acd",
"group___resources.html#ggaf140ec886884a864abc74a7556f8bf67a2b9d0399c32f2098957ebc089c337e55",
"group___script.html#ggafb79f4c215a4ac77110efe823e94b61ea073c3169e6dc41e37ea85fa2b01ca02f",
"group___script.html#ggafb79f4c215a4ac77110efe823e94b61eae245c8bfb168d549db8333f8b29144fa",
"namespacemembers_eval_v.html",
"struct_ogre_1_1_gpu_constant_definition.html#a173d7d09ffd3050979b542019e1c67be",
"struct_ogre_1_1_quake3_shader_1_1_pass.html#a0c4523d558327ebc0648d5ab2f3c0ae1",
"struct_ogre_1_1_terrain_group_1_1_terrain_slot.html#aa0cb112bc24303af1881f7849b7c0585",
"struct_ogre_1_1is_pod_like_3_01long_01_4.html",
"tut__lights_cameras_shadows.html#CreatingaLight"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';