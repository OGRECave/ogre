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
        [ "Materials", "_the-_core-_objects.html#Materials", null ],
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
          [ "Assembler Shaders", "_high-level-_programs.html#autotoc_md194", [
            [ "Specifying Named Constants", "_high-level-_programs.html#Specifying-Named-Constants-for-Assembler-Shaders", null ]
          ] ],
          [ "Multi-language Programs", "_high-level-_programs.html#multi-language-programs", null ],
          [ "Unified High-level Programs", "_high-level-_programs.html#Unified-High_002dlevel-Programs", null ],
          [ "Parameter specification", "_high-level-_programs.html#Program-Parameter-Specification", [
            [ "param_indexed", "_high-level-_programs.html#autotoc_md195", null ],
            [ "param_indexed_auto", "_high-level-_programs.html#autotoc_md196", null ],
            [ "param_named", "_high-level-_programs.html#autotoc_md197", null ],
            [ "param_named_auto", "_high-level-_programs.html#autotoc_md198", null ],
            [ "shared_params_ref", "_high-level-_programs.html#autotoc_md199", null ]
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
          [ "image_based_lighting", "rtss.html#autotoc_md247", null ],
          [ "gbuffer", "rtss.html#autotoc_md248", null ],
          [ "normal_map", "rtss.html#autotoc_md249", null ],
          [ "metal_roughness", "rtss.html#autotoc_md250", null ],
          [ "fog_stage", "rtss.html#autotoc_md251", null ],
          [ "light_count", "rtss.html#autotoc_md252", null ],
          [ "triplanarTexturing", "rtss.html#autotoc_md253", null ],
          [ "integrated_pssm4", "rtss.html#autotoc_md254", null ],
          [ "shadow_mapping", "rtss.html#shadow_mapping", null ],
          [ "hardware_skinning", "rtss.html#autotoc_md255", null ]
        ] ],
        [ "RTSS Texture Unit properties", "rtss.html#rtss_tu_props", [
          [ "normal_map", "rtss.html#normal_map", null ],
          [ "layered_blend", "rtss.html#autotoc_md256", null ],
          [ "source_modifier", "rtss.html#autotoc_md257", null ]
        ] ],
        [ "Setting properties programmatically", "rtss.html#RTSS-Props-API", null ],
        [ "System overview", "rtss.html#rtss_overview", [
          [ "Core features of the system", "rtss.html#core-feats", null ],
          [ "Controlling shader re-generation", "rtss.html#autotoc_md258", null ]
        ] ],
        [ "The RTSS in Depth", "rtss.html#rtss_indepth", [
          [ "Main components", "rtss.html#rtss__components", null ],
          [ "Initializing the system", "rtss.html#autotoc_md259", null ],
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
        [ "Built-in defines", "_cross-platform-_shaders.html#autotoc_md205", null ],
        [ "Cross-platform macros", "_cross-platform-_shaders.html#OgreUnifiedShader", null ],
        [ "Uber shader tips", "_cross-platform-_shaders.html#autotoc_md206", null ]
      ] ]
    ] ],
    [ "Tutorials", "tutorials.html", [
      [ "Guide to building OGRE", "building-ogre.html", [
        [ "What is CMake?", "building-ogre.html#autotoc_md260", null ],
        [ "Preparing the build environment", "building-ogre.html#autotoc_md261", null ],
        [ "Getting dependencies", "building-ogre.html#autotoc_md262", [
          [ "Linux", "building-ogre.html#autotoc_md263", null ],
          [ "Recommended dependencies", "building-ogre.html#autotoc_md264", null ],
          [ "Optional dependencies", "building-ogre.html#autotoc_md265", null ],
          [ "Deprecated dependencies", "building-ogre.html#autotoc_md266", null ]
        ] ],
        [ "Running CMake", "building-ogre.html#running-cmake", null ],
        [ "Building", "building-ogre.html#autotoc_md267", null ],
        [ "Installing", "building-ogre.html#installing-sdk", null ],
        [ "Installing and building via vcpkg", "building-ogre.html#autotoc_md268", null ],
        [ "Cross-Compiling", "building-ogre.html#autotoc_md269", [
          [ "Android", "building-ogre.html#autotoc_md270", null ],
          [ "WebAssembly / Emscripten", "building-ogre.html#autotoc_md271", null ],
          [ "iOS OS", "building-ogre.html#autotoc_md272", null ],
          [ "WinRT / UWP", "building-ogre.html#autotoc_md273", null ]
        ] ]
      ] ],
      [ "Setting up an OGRE project", "setup.html", [
        [ "CMake Configuration", "setup.html#cmake", null ],
        [ "Application skeleton", "setup.html#skeleton", null ],
        [ "Running your App", "setup.html#setupRunning", [
          [ "Configuration Files", "setup.html#autotoc_md308", [
            [ "plugins.cfg", "setup.html#autotoc_md309", null ],
            [ "resources.cfg", "setup.html#autotoc_md310", null ],
            [ "ogre.cfg", "setup.html#autotoc_md311", null ]
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
      [ "Working with NumPy", "working-with-numpy.html", [
        [ "Background Threads", "working-with-numpy.html#python-background-threads", null ]
      ] ],
      [ "Trays GUI System", "trays.html", [
        [ "Trays", "trays.html#trays-1", null ],
        [ "TrayManager", "trays.html#traymanager", [
          [ "The Cursor", "trays.html#autotoc_md316", null ],
          [ "The Backdrop", "trays.html#autotoc_md317", null ]
        ] ],
        [ "Widgets", "trays.html#widgets", [
          [ "Button", "trays.html#autotoc_md318", null ],
          [ "TextBox", "trays.html#autotoc_md319", null ],
          [ "SelectMenu", "trays.html#autotoc_md320", null ],
          [ "Label", "trays.html#autotoc_md321", null ],
          [ "Separator", "trays.html#autotoc_md322", null ],
          [ "Slider", "trays.html#autotoc_md323", null ],
          [ "ParamsPanel", "trays.html#autotoc_md324", null ],
          [ "CheckBox", "trays.html#autotoc_md325", null ],
          [ "DecorWidget", "trays.html#autotoc_md326", null ],
          [ "ProgressBar", "trays.html#autotoc_md327", null ],
          [ "The Null Tray", "trays.html#autotoc_md328", null ]
        ] ],
        [ "Special Widgets", "trays.html#special-widgets", [
          [ "Frame Stats", "trays.html#autotoc_md329", null ],
          [ "Logo", "trays.html#autotoc_md330", null ],
          [ "Loading Bar", "trays.html#autotoc_md331", null ],
          [ "Information Dialog", "trays.html#autotoc_md332", null ],
          [ "Question Dialog", "trays.html#autotoc_md333", null ]
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
          [ "Replacing a component", "meshlod-generator.html#autotoc_md302", null ]
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
        [ "Using Manual Object", "manual-mesh-creation.html#autotoc_md299", [
          [ "Example", "manual-mesh-creation.html#autotoc_md300", null ]
        ] ],
        [ "Using vertex and index buffers directly", "manual-mesh-creation.html#autotoc_md301", null ]
      ] ],
      [ "Static Geometry", "tut__static_geom.html", [
        [ "Blades of grass", "tut__static_geom.html#autotoc_md312", null ],
        [ "A field of grass", "tut__static_geom.html#autotoc_md313", null ],
        [ "Animating StaticGeometry", "tut__static_geom.html#autotoc_md314", null ],
        [ "Advanced Object Batching", "tut__static_geom.html#autotoc_md315", null ]
      ] ],
      [ "Using the Profiler", "profiler.html", [
        [ "Reading the Display", "profiler.html#profRead", null ],
        [ "Features", "profiler.html#profFeatures", [
          [ "Disabling the Profiler", "profiler.html#autotoc_md303", null ],
          [ "Disabling Individual Profiles", "profiler.html#autotoc_md304", null ],
          [ "Analyzing Application State", "profiler.html#autotoc_md305", null ],
          [ "Logging Results", "profiler.html#autotoc_md306", null ],
          [ "Changing the Frequency of Updating the Display", "profiler.html#autotoc_md307", null ]
        ] ],
        [ "Performance and Accuracy", "profiler.html#profAccuracy", null ],
        [ "Remotery Backend", "profiler.html#profRemotery", null ],
        [ "Release Version Considerations", "profiler.html#profRelmode", null ]
      ] ],
      [ "Reversed Depth", "reversed-depth.html", null ],
      [ "External Texture Sources", "_external-_texture-_sources.html", [
        [ "What Is An External Texture Source?", "_external-_texture-_sources.html#autotoc_md294", null ],
        [ "ExternalTextureSource Class", "_external-_texture-_sources.html#autotoc_md295", null ],
        [ "ExternalTextureSourceManager Class", "_external-_texture-_sources.html#autotoc_md296", null ],
        [ "Texture Source Material Script", "_external-_texture-_sources.html#autotoc_md297", null ],
        [ "Simplified Diagram of Process", "_external-_texture-_sources.html#autotoc_md298", null ]
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
          [ "Deferred Shading Advantages", "deferred.html#autotoc_md274", null ],
          [ "Deferred Shading Disadvantages", "deferred.html#autotoc_md275", null ]
        ] ],
        [ "Creating the G-Buffer", "deferred.html#creating", [
          [ "Deciding on the GBuffer format", "deferred.html#autotoc_md276", null ],
          [ "Preparing the objects for G-Buffer rendering", "deferred.html#autotoc_md277", [
            [ "Inspect the classic technique", "deferred.html#autotoc_md278", null ],
            [ "Generate the G-Buffer technique", "deferred.html#autotoc_md279", null ],
            [ "Add the G-Buffer technique to the original material", "deferred.html#autotoc_md280", null ],
            [ "Putting it all together", "deferred.html#autotoc_md281", null ]
          ] ],
          [ "Overriding the automatic process", "deferred.html#autotoc_md282", null ],
          [ "Seeing it in action", "deferred.html#autotoc_md283", null ]
        ] ],
        [ "Lighting the scene", "deferred.html#autotoc_md284", [
          [ "Rendering the light geometry", "deferred.html#lightgeom", [
            [ "Prepare ambient colour and rebuild original depth buffer", "deferred.html#autotoc_md285", null ],
            [ "Render the light geometries", "deferred.html#autotoc_md286", null ],
            [ "Rendering shadow casting lights", "deferred.html#autotoc_md287", null ],
            [ "Putting it all together", "deferred.html#autotoc_md288", null ],
            [ "Seeing it in action", "deferred.html#autotoc_md289", null ]
          ] ]
        ] ],
        [ "Post Processing", "deferred.html#post", [
          [ "Screen Space Ambient Occlusion", "deferred.html#autotoc_md290", null ]
        ] ],
        [ "Integration in real projects", "deferred.html#realprojects", [
          [ "Integration steps", "deferred.html#autotoc_md291", null ],
          [ "Adapting the framework", "deferred.html#autotoc_md292", null ],
          [ "Adding features to the framework", "deferred.html#autotoc_md293", null ]
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
"_particle-_scripts.html#autotoc_md233",
"class_ogre_1_1_animation.html#a991a5be3ad323e9690df2e065b6a1404",
"class_ogre_1_1_auto_param_data_source.html#a277b593c791b45b0521858d28374906c",
"class_ogre_1_1_billboard_chain.html#afe208f1c230dc69b02e118417892f122",
"class_ogre_1_1_bsp_ray_scene_query.html#a0a7e638608a44ba0b50998b84029d324",
"class_ogre_1_1_codec.html#ada1a5477ab6a563565f1b3599d7b806a",
"class_ogre_1_1_composition_pass.html#adb9a1389283be682baf5cb41e20cc3c7",
"class_ogre_1_1_config_file.html#ae9cea1fe4ad70279841635eb892ba514",
"class_ogre_1_1_d3_d9_render_system.html#ac150cea205fda65dacbccaee1d5d652fa25921c72928c345bce879cdeb59ea4de",
"class_ogre_1_1_deflate_stream.html#a5e29bee00d7357fd2597c20ab0d371f7",
"class_ogre_1_1_entity.html#a9a305624b894260c7cbeabd1eb5df612",
"class_ogre_1_1_frustum.html#a4c490f3613804ad0ff61a102f59e7e13",
"class_ogre_1_1_gpu_program_manager.html#ab03fd65fa6ec3cd96a5befd84b267c39",
"class_ogre_1_1_gpu_program_parameters.html#aea35597f193303287577318a36258f53",
"class_ogre_1_1_high_level_gpu_program.html#a97b590c014a190ee4bec05d88eb0c976",
"class_ogre_1_1_light.html#adae366516f078c422412f45778753516",
"class_ogre_1_1_manual_object.html#a523185f71dee3e6e8352462172dfe30e",
"class_ogre_1_1_math.html#a6beb574317f8085bab69587ef7dc72ed",
"class_ogre_1_1_mesh.html#ae57798459c55a564d81097c2af35863a",
"class_ogre_1_1_node.html#a0b69022b50c829ec21e9589bbc591597",
"class_ogre_1_1_octree_scene_manager.html#a019668f0437cee28be1c53ef1e0bdd3a",
"class_ogre_1_1_overlay_element.html#af15092ccb2db4413a5d5a74f02ca5dfd",
"class_ogre_1_1_p_c_z_scene_manager_factory.html#ac1149fd3168410fdedf4c09c1deb85d1",
"class_ogre_1_1_page_manager.html#a6bc619f289f2ed3491a822ee4abbbd3d",
"class_ogre_1_1_particle_emitter.html#aef756f0d236964369f790e8f4004993f",
"class_ogre_1_1_pass.html#a6e5c5f69be78ddd80cbb202f66649fef",
"class_ogre_1_1_plane_bounded_volume_list_scene_query.html#af82e119f79b5e4e91e599c27623977df",
"class_ogre_1_1_process_resource_name_script_compiler_event.html#afd04c7f48ef0823ba70812b83fde1acca79d98608fb92fdd1ff492f56d86b2ba1",
"class_ogre_1_1_quaternion.html#af3424f1e07092c0ffe293d18b277a04f",
"class_ogre_1_1_r_t_shader_1_1_parameter.html#a91fc3614f191bebf0461676a03750312a9b746851560ab52d22335755dd36a0ea",
"class_ogre_1_1_r_t_shader_1_1_uniform_parameter.html#add5b3d6cfb3ddb4ebf78f5656ce3b070",
"class_ogre_1_1_render_system.html#a5387ab6d7b2ae4813f562b758742cc92",
"class_ogre_1_1_render_window.html#a5fe9b7de87bc535e7278dac1e6bdc002",
"class_ogre_1_1_resource_manager.html#abd807a5a8f5754af45fc60db40b455d5",
"class_ogre_1_1_scene_manager.html#a0612e86af9d01cdbba02a32ade408892",
"class_ogre_1_1_scene_manager.html#ac6e42c947472bf09620ef974c3ac26a5",
"class_ogre_1_1_serializer.html#a6e038a802f8d1f293e73b4d98f6e297d",
"class_ogre_1_1_small_vector_impl.html#aeced538bf10b8a80e7a6ee17c63286a4",
"class_ogre_1_1_stream_serialiser.html#a0ffe90cc7b051f0f56faccc739988cea",
"class_ogre_1_1_technique.html#a0cc5cc046407bb94bc1bbddaea803bb9",
"class_ogre_1_1_terrain.html#ac78ab1806379267473d03a57dde9bbb8",
"class_ogre_1_1_terrain_material_generator.html#a7c66198986a8b44314be2aedcfbaf4fe",
"class_ogre_1_1_texture_manager.html#a8faa10c9150687d1bf229ccff16a8960",
"class_ogre_1_1_transform_key_frame.html#a46868a5a6248f24b96b2a5a2b54b9cc3",
"class_ogre_1_1_viewport.html#a4d2598f85e084bdd48948b78563a178d",
"class_ogre_1_1_vulkan_render_system.html#a662adfeeae0156349100e2041fb0e69f",
"class_ogre_bites_1_1_select_menu.html#a5828fc53ccc7c7343419ba23d12f7d1a",
"dir_67ea8a25251f25c005532ef7e5f68f3d.html",
"group___direct3_d11.html#ga6f6c9deb0948aee37017703804ddff8f",
"group___image.html#ga7d0e523490616c11c6b9b42e98099c63",
"group___materials.html#gga264ecab38a04d01e2c3158fa05ec1ee2a985f2ec53a477d7c19b41700589cb519",
"group___paging.html#ga6821abf728558c495596bd508cf20228",
"group___render_system.html#gga3d2965b7f378ebdcfe8a4a6cf74c3de7a699c269ed4c1a0b1c782d6d471f47c62",
"group___scene.html#ga4fdd447620e786031195fc75086a9043",
"group___script.html#ggafb79f4c215a4ac77110efe823e94b61ea4b69ca0461cfe83ade5749663560492d",
"group__deprecated.html#gga7383602bd480d43b80c626969b9af914a7eecd7057b3487616075032560582ec1",
"rtss.html#autotoc_md246",
"struct_ogre_1_1_linked_skeleton_animation_source.html#a02c73a85564ec0047f8de9b71e771331",
"struct_ogre_1_1_s_p_f_m_delete_t.html",
"struct_ogre_1_1_vector_base_3_013_00_01_real_01_4.html#a42a88148edf0e38db8a1fb4ab5d5b499",
"struct_ogre_bites_1_1_input_listener.html#a6b5bb758dff77d6de9c6fad00ddbe591"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';