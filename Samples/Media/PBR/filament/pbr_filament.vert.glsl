#version 330

// Apparently needed by layout(location = ) syntax
#extension GL_ARB_separate_shader_objects : enable

in vec4 position;

#if defined(HAS_ATTRIBUTE_TANGENTS)
in vec4 normal;
in vec4 tangent;
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
in vec4 colour;
#endif

#if defined(HAS_ATTRIBUTE_UV0)
in vec2 uv0;
#endif

#if defined(HAS_ATTRIBUTE_UV1)
in vec2 uv1;
#endif

out highp vec3 vertex_worldPosition;
#if defined(HAS_ATTRIBUTE_TANGENTS)
out mediump vec3 vertex_worldNormal;
#if defined(MATERIAL_HAS_ANISOTROPY) || defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
out mediump vec3 vertex_worldTangent;
out mediump vec3 vertex_worldBitangent;
#endif
#endif

#if defined(HAS_ATTRIBUTE_COLOR)
out mediump vec4 vertex_color;
#endif

#if defined(HAS_ATTRIBUTE_UV0) && !defined(HAS_ATTRIBUTE_UV1)
out highp vec2 vertex_uv01;
#elif defined(HAS_ATTRIBUTE_UV1)
out highp vec4 vertex_uv01;
#endif

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
out highp vec4 vertex_lightSpacePosition;
#endif

#include "pbr_filament_objectuniforms.glsl"
#include "pbr_filament_frameuniforms.glsl"

#include "common_getters.fs"
#include "common_math.fs"
#include "getters.vs"
#include "material_inputs.vs"

// This is mainly from main.vs

void main()
{
    // Initialize the inputs to sensible default values, see material_inputs.vs
    MaterialVertexInputs material;
    initMaterialVertex(material);

#if defined(HAS_ATTRIBUTE_TANGENTS)
    // If the material defines a value for the "normal" property, we need to output
    // the full orthonormal basis to apply normal mapping
    #if defined(MATERIAL_HAS_ANISOTROPY) || defined(MATERIAL_HAS_NORMAL) || defined(MATERIAL_HAS_CLEAR_COAT_NORMAL)
        // Extract the normal and tangent in world space from the input quaternion
        // We encode the orthonormal basis as a quaternion to save space in the attributes
        // toTangentFrame(mesh_tangents, material.worldNormal, vertex_worldTangent);
        // Ogre does not provide "orthonormal basis quaternion" but tangents/normals/binormals directly.
        material.worldNormal = normal.xyz;
        vertex_worldTangent = tangent.xyz;

        #if defined(HAS_SKINNING_OR_MORPHING)

            if (objectUniforms.morphingEnabled == 1) {
                vec3 normal0, normal1, normal2, normal3;
                toTangentFrame(mesh_custom4, normal0);
                toTangentFrame(mesh_custom5, normal1);
                toTangentFrame(mesh_custom6, normal2);
                toTangentFrame(mesh_custom7, normal3);
                material.worldNormal += objectUniforms.morphWeights.x * normal0;
                material.worldNormal += objectUniforms.morphWeights.y * normal1;
                material.worldNormal += objectUniforms.morphWeights.z * normal2;
                material.worldNormal += objectUniforms.morphWeights.w * normal3;
                material.worldNormal = normalize(material.worldNormal);
            }

            if (objectUniforms.skinningEnabled == 1) {
                skinNormal(material.worldNormal, mesh_bone_indices, mesh_bone_weights);
                skinNormal(vertex_worldTangent, mesh_bone_indices, mesh_bone_weights);
            }

        #endif

        // We don't need to normalize here, even if there's a scale in the matrix
        // because we ensure the worldFromModelNormalMatrix pre-scales the normal such that
        // all its components are < 1.0. This precents the bitangent to exceed the range of fp16
        // in the fragment shader, where we renormalize after interpolation
        // vertex_worldTangent = objectUniforms.worldFromModelNormalMatrix * vertex_worldTangent;
        // material.worldNormal = objectUniforms.worldFromModelNormalMatrix * material.worldNormal;
        mat3 worldFromModelNormalMatrix = mat3(objectUniforms.worldFromModelMatrix);
        worldFromModelNormalMatrix = mat3(
            normalize(worldFromModelNormalMatrix[0]),
            normalize(worldFromModelNormalMatrix[1]),
            normalize(worldFromModelNormalMatrix[2])
        );

        vertex_worldTangent = worldFromModelNormalMatrix * tangent.xyz;
        material.worldNormal = worldFromModelNormalMatrix * normal.xyz;

        vertex_worldTangent = normalize(vertex_worldTangent - dot(normalize(material.worldNormal), vertex_worldTangent) * normalize(material.worldNormal));

        // Reconstruct the bitangent from the normal and tangent. We don't bother with
        // normalization here since we'll do it after interpolation in the fragment stage
        // vertex_worldBitangent =
        //        cross(material.worldNormal, vertex_worldTangent) * sign(tangent.w);
        vertex_worldBitangent = cross(material.worldNormal, vertex_worldTangent) * sign(tangent.w);
    #else // MATERIAL_HAS_ANISOTROPY || MATERIAL_HAS_NORMAL
        // Without anisotropy or normal mapping we only need the normal vector
        // toTangentFrame(mesh_tangents, material.worldNormal);

        #if defined(HAS_SKINNING_OR_MORPHING)
            if (objectUniforms.skinningEnabled == 1) {
                skinNormal(material.worldNormal, mesh_bone_indices, mesh_bone_weights);
            }
        #endif

        // material.worldNormal = objectUniforms.worldFromModelNormalMatrix * material.worldNormal;

    #endif // MATERIAL_HAS_ANISOTROPY || MATERIAL_HAS_NORMAL
#endif // HAS_ATTRIBUTE_TANGENTS

    // Invoke user code
    // materialVertex(material);

    // Handle built-in interpolated attributes
#if defined(HAS_ATTRIBUTE_COLOR)
    vertex_color = material.color;
#endif
#if defined(HAS_ATTRIBUTE_UV0)
    vertex_uv01.xy = material.uv0;
#endif
#if defined(HAS_ATTRIBUTE_UV1)
    vertex_uv01.zw = material.uv1;
#endif

    // Handle user-defined interpolated attributes
#if defined(VARIABLE_CUSTOM0)
    VARIABLE_CUSTOM_AT0 = material.VARIABLE_CUSTOM0;
#endif
#if defined(VARIABLE_CUSTOM1)
    VARIABLE_CUSTOM_AT1 = material.VARIABLE_CUSTOM1;
#endif
#if defined(VARIABLE_CUSTOM2)
    VARIABLE_CUSTOM_AT2 = material.VARIABLE_CUSTOM2;
#endif
#if defined(VARIABLE_CUSTOM3)
    VARIABLE_CUSTOM_AT3 = material.VARIABLE_CUSTOM3;
#endif

    // The world position can be changed by the user in materialVertex()
    vertex_worldPosition = material.worldPosition.xyz;
#ifdef HAS_ATTRIBUTE_TANGENTS
    vertex_worldNormal = material.worldNormal;
#endif

#if defined(HAS_SHADOWING) && defined(HAS_DIRECTIONAL_LIGHTING)
    vertex_lightSpacePosition = getLightSpacePosition(vertex_worldPosition, vertex_worldNormal);
#endif

#if defined(VERTEX_DOMAIN_DEVICE)
    // The other vertex domains are handled in initMaterialVertex()->computeWorldPosition()
    gl_Position = getPosition();
#else
    gl_Position = getClipFromWorldMatrix() * getWorldPosition(material);
#endif

#ifdef MATERIAL_HAS_CLIP_SPACE_TRANSFORM
    gl_Position = getClipSpaceTransform(material) * gl_Position;
#endif

#if defined(TARGET_VULKAN_ENVIRONMENT)
    // In Vulkan, clip-space Z is [0,w] rather than [-w,+w] and Y is flipped.
    gl_Position.y = -gl_Position.y;
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
#endif
}
