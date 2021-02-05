
/*================================================================
    * Copyright: 2020 John Jackson 
    * File: gs_graphics_impl.h
    All Rights Reserved
=================================================================*/

#ifndef __GS_GRAPHICS_IMPL_H__
#define __GS_GRAPHICS_IMPL_H__

#ifdef GS_GRAPHICS_IMPL_OPENGL
    #define GS_GRAPHICS_IMPL_DEFAULT
#endif

#ifdef GS_GRAPHICS_IMPL_DEFAULT

// Default stuff here, if any...

/* Graphics Info Object Query */
gs_graphics_info_t* gs_graphics_info()
{
    return &gs_engine_subsystem(graphics)->info;
}

#endif

#ifdef GS_GRAPHICS_IMPL_OPENGL

typedef enum gsgl_uniform_type
{
    GSGL_UNIFORMTYPE_FLOAT,
    GSGL_UNIFORMTYPE_INT,
    GSGL_UNIFORMTYPE_VEC2,
    GSGL_UNIFORMTYPE_VEC3,
    GSGL_UNIFORMTYPE_VEC4,
    GSGL_UNIFORMTYPE_MAT4,
    GSGL_UNIFORMTYPE_SAMPLER2D,
    GSGL_UNIFORMTYPE_UNIFORM_BLOCK
} gsgl_uniform_type;

/* Uniform (stores samplers as well as primitive uniforms) */
typedef struct gsgl_uniform_t {
    const char* name;
    gsgl_uniform_type type;
    uint32_t location;
    size_t size;                    // Total data size of uniform
} gsgl_uniform_t;

/* Pipeline */
typedef struct gsgl_pipeline_t {
    gs_graphics_blend_state_desc_t blend;
    gs_graphics_depth_state_desc_t depth;
    gs_graphics_raster_state_desc_t raster;
    gs_graphics_stencil_state_desc_t stencil;
    gs_graphics_compute_state_desc_t compute;
    gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout;
} gsgl_pipeline_t;

/* Render Pass */
typedef struct gsgl_render_pass_t {
    gs_handle(gs_graphics_buffer_t) fbo;                        
    gs_dyn_array(gs_handle(gs_graphics_texture_t)) color;
    gs_handle(gs_graphics_texture_t) depth; 
    gs_handle(gs_graphics_texture_t) stencil;
} gsgl_render_pass_t;

/* Shader */
typedef uint32_t gsgl_shader_t;

/* Gfx Buffer */
typedef uint32_t gsgl_buffer_t;

/* Texture */
typedef uint32_t gsgl_texture_t;

typedef struct gsgl_vertex_buffer_decl_t {
    gsgl_buffer_t vbo;
    gs_graphics_vertex_data_type data_type;
    size_t offset;
} gsgl_vertex_buffer_decl_t;

/* Cached data between draws */
typedef struct gsgl_data_cache_t
{
    gsgl_buffer_t vao;
    gsgl_buffer_t ibo;
    size_t ibo_elem_sz;
    gs_dyn_array(gsgl_vertex_buffer_decl_t) vdecls;
    gs_handle(gs_graphics_pipeline_t) pipeline;
} gsgl_data_cache_t;

/* Internal Opengl Data */
typedef struct gsgl_data_t
{
    gs_slot_array(gsgl_shader_t)        shaders;
    gs_slot_array(gsgl_texture_t)       textures;
    gs_slot_array(gsgl_buffer_t)        vertex_buffers;
    gs_slot_array(gsgl_buffer_t)        index_buffers;
    gs_slot_array(gsgl_buffer_t)        frame_buffers;
    gs_slot_array(gsgl_uniform_t)       uniforms;
    gs_slot_array(gsgl_pipeline_t)      pipelines;
    gs_slot_array(gsgl_render_pass_t)   render_passes;

    // Cached data between draw calls (to minimize state changes)
    gsgl_data_cache_t cache;

} gsgl_data_t;

// Do I want to add a deferred buffer update here?... Or add that elsewhere?
// Means I'd have to create a VM for the immediate mode...

/* Internal OGL Command Buffer Op Code */
typedef enum gs_opengl_op_code_type
{
    GS_OPENGL_OP_BEGIN_RENDER_PASS = 0x00,
    GS_OPENGL_OP_END_RENDER_PASS,
    GS_OPENGL_OP_SET_VIEWPORT,
    GS_OPENGL_OP_SET_VIEW_SCISSOR,
    GS_OPENGL_OP_REQUEST_BUFFER_UPDATE,
    GS_OPENGL_OP_BIND_PIPELINE,
    GS_OPENGL_OP_BIND_BINDINGS,
    GS_OPENGL_OP_DISPATCH_COMPUTE,
    GS_OPENGL_OP_DRAW,
} gs_opengl_op_code_type;

void gsgl_reset_data_cache(gsgl_data_cache_t* cache)
{
    cache->ibo = 0;
    cache->ibo_elem_sz = 0;
    cache->pipeline = gs_handle_invalid(gs_graphics_pipeline_t);
    gs_dyn_array_clear(cache->vdecls);
}

void gsgl_pipeline_state()
{
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindImageTexture(0, 0, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_BLEND);
}

/* GS/OGL Utilities */
int32_t gsgl_buffer_usage_to_gl_enum(gs_graphics_buffer_usage_type type)
{
    switch (type)
    {
        case GS_GRAPHICS_BUFFER_USAGE_STATIC:   return GL_STATIC_DRAW;
        case GS_GRAPHICS_BUFFER_USAGE_STREAM:   return GL_STREAM_DRAW;
        case GS_GRAPHICS_BUFFER_USAGE_DYNAMIC:  return GL_DYNAMIC_DRAW;

        default:                                return GL_STATIC_DRAW;
    }
}

uint32_t gsgl_access_type_to_gl_access_type(gs_graphics_access_type type)
{
    switch (type)
    {
        case GS_GRAPHICS_ACCESS_WRITE_ONLY: return GL_WRITE_ONLY;
        case GS_GRAPHICS_ACCESS_READ_ONLY:  return GL_READ_ONLY;
        case GS_GRAPHICS_ACCESS_READ_WRITE: return GL_READ_WRITE;

        default:                            return GL_WRITE_ONLY;
    }
}

uint32_t gsgl_texture_format_to_gl_texture_format(gs_graphics_texture_format_type type)
{
    switch (type)
    {
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:      return GL_RGBA8;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:       return GL_RGB8;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:    return GL_RGBA16F;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:    return GL_RGBA32F;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:         return GL_R8;

        default:                                    return GL_RGBA32F;
    }
}

uint32_t gsgl_shader_stage_to_gl_stage(gs_graphics_shader_stage_type type)
{
    switch (type)
    {
        case GS_GRAPHICS_SHADER_STAGE_VERTEX:   return GL_VERTEX_SHADER;
        case GS_GRAPHICS_SHADER_STAGE_FRAGMENT: return GL_FRAGMENT_SHADER;
        case GS_GRAPHICS_SHADER_STAGE_COMPUTE:  return GL_COMPUTE_SHADER;

        default:                                return GL_VERTEX_SHADER;
    }
}

uint32_t gsgl_primitive_to_gl_primitive(gs_graphics_primitive_type type)
{ 
    switch (type)
    {
        case GS_GRAPHICS_PRIMITIVE_TRIANGLES:   return GL_TRIANGLES;
        case GS_GRAPHICS_PRIMITIVE_LINES:       return GL_LINES;
        case GS_GRAPHICS_PRIMITIVE_QUADS:       return GL_QUADS;

        default:                                return GL_TRIANGLES;
    }
}

uint32_t gsgl_blend_equation_to_gl_blend_eq(gs_graphics_blend_equation_type eq)
{
    switch (eq)
    {
        case GS_GRAPHICS_BLEND_EQUATION_ADD:                return GL_FUNC_ADD;
        case GS_GRAPHICS_BLEND_EQUATION_SUBTRACT:           return GL_FUNC_SUBTRACT;
        case GS_GRAPHICS_BLEND_EQUATION_REVERSE_SUBTRACT:   return GL_FUNC_REVERSE_SUBTRACT;
        case GS_GRAPHICS_BLEND_EQUATION_MIN:                return GL_MIN;
        case GS_GRAPHICS_BLEND_EQUATION_MAX:                return GL_MAX;

        default:                                            return GL_FUNC_ADD;
    };
}

uint32_t gsgl_blend_mode_to_gl_blend_mode(gs_graphics_blend_mode_type type, uint32_t def)
{
    switch (type) {
        case GS_GRAPHICS_BLEND_MODE_ZERO:                       return GL_ZERO;
        case GS_GRAPHICS_BLEND_MODE_ONE:                        return GL_ONE;
        case GS_GRAPHICS_BLEND_MODE_SRC_COLOR:                  return GL_SRC_COLOR;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_COLOR:        return GL_ONE_MINUS_SRC_COLOR;
        case GS_GRAPHICS_BLEND_MODE_DST_COLOR:                  return GL_DST_COLOR;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_COLOR:        return GL_ONE_MINUS_DST_COLOR;
        case GS_GRAPHICS_BLEND_MODE_SRC_ALPHA:                  return GL_SRC_ALPHA;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_SRC_ALPHA:        return GL_ONE_MINUS_SRC_ALPHA;
        case GS_GRAPHICS_BLEND_MODE_DST_ALPHA:                  return GL_DST_ALPHA;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_DST_ALPHA:        return GL_ONE_MINUS_DST_ALPHA;
        case GS_GRAPHICS_BLEND_MODE_CONSTANT_COLOR:             return GL_CONSTANT_COLOR;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_COLOR:   return GL_ONE_MINUS_CONSTANT_COLOR;
        case GS_GRAPHICS_BLEND_MODE_CONSTANT_ALPHA:             return GL_CONSTANT_ALPHA;
        case GS_GRAPHICS_BLEND_MODE_ONE_MINUS_CONSTANT_ALPHA:   return GL_ONE_MINUS_CONSTANT_ALPHA;
        default:                                                return def;
    }
}

uint32_t gsgl_cull_face_to_gl_cull_face(gs_graphics_face_culling_type type)
{
    switch (type) {
        case GS_GRAPHICS_FACE_CULLING_BACK:             return GL_BACK;
        case GS_GRAPHICS_FACE_CULLING_FRONT:            return GL_FRONT;
        case GS_GRAPHICS_FACE_CULLING_FRONT_AND_BACK:   return GL_FRONT_AND_BACK;
        default:                                        return GL_BACK;
    }
}   

uint32_t gsgl_winding_order_to_gl_winding_order(gs_graphics_winding_order_type type)
{  
    switch (type)
    {
        case GS_GRAPHICS_WINDING_ORDER_CCW: return GL_CCW;
        case GS_GRAPHICS_WINDING_ORDER_CW:  return GL_CW;;
        default:                            return GL_CCW;
    }
}

uint32_t gsgl_depth_func_to_gl_depth_func(gs_graphics_depth_func_type type)
{
    switch (type) {
        case GS_GRAPHICS_DEPTH_FUNC_LESS:       return GL_LESS;
        case GS_GRAPHICS_DEPTH_FUNC_NEVER:      return GL_NEVER; 
        case GS_GRAPHICS_DEPTH_FUNC_EQUAL:      return GL_EQUAL;
        case GS_GRAPHICS_DEPTH_FUNC_LEQUAL:     return GL_LEQUAL;
        case GS_GRAPHICS_DEPTH_FUNC_GREATER:    return GL_GREATER;
        case GS_GRAPHICS_DEPTH_FUNC_NOTEQUAL:   return GL_NOTEQUAL;
        case GS_GRAPHICS_DEPTH_FUNC_GEQUAL:     return GL_GEQUAL;
        case GS_GRAPHICS_DEPTH_FUNC_ALWAYS:     return GL_ALWAYS;
        default:                                return GL_LESS;
    }
}

uint32_t gsgl_stencil_func_to_gl_stencil_func(gs_graphics_stencil_func_type type)
{
    switch (type) {
        case GS_GRAPHICS_STENCIL_FUNC_LESS:     return GL_LESS;
        case GS_GRAPHICS_STENCIL_FUNC_NEVER:    return GL_NEVER; 
        case GS_GRAPHICS_STENCIL_FUNC_EQUAL:    return GL_EQUAL;
        case GS_GRAPHICS_STENCIL_FUNC_LEQUAL:   return GL_LEQUAL;
        case GS_GRAPHICS_STENCIL_FUNC_GREATER:  return GL_GREATER;
        case GS_GRAPHICS_STENCIL_FUNC_NOTEQUAL: return GL_NOTEQUAL;
        case GS_GRAPHICS_STENCIL_FUNC_GEQUAL:   return GL_GEQUAL;
        case GS_GRAPHICS_STENCIL_FUNC_ALWAYS:   return GL_ALWAYS;

        default:                                return GL_ALWAYS;
    }
}

uint32_t gsgl_stencil_op_to_gl_stencil_op(gs_graphics_stencil_op_type type)
{
    switch (type) {
        case GS_GRAPHICS_STENCIL_OP_KEEP:       return GL_KEEP;
        case GS_GRAPHICS_STENCIL_OP_ZERO:       return GL_ZERO;
        case GS_GRAPHICS_STENCIL_OP_REPLACE:    return GL_REPLACE;
        case GS_GRAPHICS_STENCIL_OP_INCR:       return GL_INCR;
        case GS_GRAPHICS_STENCIL_OP_INCR_WRAP:  return GL_INCR_WRAP;
        case GS_GRAPHICS_STENCIL_OP_DECR:       return GL_DECR;
        case GS_GRAPHICS_STENCIL_OP_DECR_WRAP:  return GL_DECR_WRAP;
        case GS_GRAPHICS_STENCIL_OP_INVERT:     return GL_INVERT;

        default:                                return GL_KEEP;
    }
}

gsgl_uniform_type gsgl_uniform_type_to_gl_uniform_type(gs_graphics_uniform_type gstype)
{
    switch (gstype) {
        case GS_GRAPHICS_UNIFORM_FLOAT: return GSGL_UNIFORMTYPE_FLOAT;
        case GS_GRAPHICS_UNIFORM_INT:   return GSGL_UNIFORMTYPE_INT;
        case GS_GRAPHICS_UNIFORM_VEC2:  return GSGL_UNIFORMTYPE_VEC2;
        case GS_GRAPHICS_UNIFORM_VEC3:  return GSGL_UNIFORMTYPE_VEC3;
        case GS_GRAPHICS_UNIFORM_VEC4:  return GSGL_UNIFORMTYPE_VEC4;
        case GS_GRAPHICS_UNIFORM_MAT4:  return GSGL_UNIFORMTYPE_MAT4;

        default:                        return GSGL_UNIFORMTYPE_FLOAT;
    }
} 

uint32_t gsgl_index_buffer_size_to_gl_index_type(size_t sz)
{
    switch (sz) {
        case 4:     return GL_UNSIGNED_INT; 
        case 2:     return GL_UNSIGNED_SHORT;
        case 1:     return GL_UNSIGNED_BYTE;

        default:    return GL_UNSIGNED_INT;
    }
}

size_t gsgl_get_byte_size_of_vertex_attribute(gs_graphics_vertex_attribute_type type)
{
    switch (type) {
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4:   return sizeof(float32_t) * 4;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3:   return sizeof(float32_t) * 3;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2:   return sizeof(float32_t) * 2;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:    return sizeof(float32_t) * 1;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:    return sizeof(uint32_t) * 4;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:    return sizeof(uint32_t) * 3;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:    return sizeof(uint32_t) * 2;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT:     return sizeof(uint32_t) * 1;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:    return sizeof(uint8_t) * 4;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:    return sizeof(uint8_t) * 3;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:    return sizeof(uint8_t) * 2;
        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:     return sizeof(uint8_t) * 1;

        default:                                    return 0;
    }
}


size_t gsgl_calculate_vertex_size_in_bytes(gs_graphics_vertex_attribute_desc_t* layout, uint32_t count)
{
    // Iterate through all formats in delcarations and calculate total size
    size_t sz = 0;
    for (uint32_t i = 0; i < count; ++i) {
        gs_graphics_vertex_attribute_type type = layout[i].format;
        sz += gsgl_get_byte_size_of_vertex_attribute(type);
    }

    return sz;
}

size_t  gsgl_get_vertex_attr_byte_offest(gs_dyn_array(gs_graphics_vertex_attribute_desc_t) layout, uint32_t idx)
{
    // Recursively calculate offset
    size_t total_offset = 0;

    // Base case
    if (idx == 0) {
        return total_offset;
    } 

    // Calculate total offset up to this point
    for (uint32_t i = 0; i < idx; ++i) {
        total_offset += gsgl_get_byte_size_of_vertex_attribute(layout[i].format);
    } 

    return total_offset;
}

size_t gsgl_uniform_data_size_in_bytes(gs_graphics_uniform_type type)
{
    switch (type) {
        case GS_GRAPHICS_UNIFORM_FLOAT: return sizeof(float);
        case GS_GRAPHICS_UNIFORM_INT:   return sizeof(int32_t);
        case GS_GRAPHICS_UNIFORM_VEC2:  return 2 * sizeof(float);
        case GS_GRAPHICS_UNIFORM_VEC3:  return 3 * sizeof(float);
        case GS_GRAPHICS_UNIFORM_VEC4:  return 4 * sizeof(float);
        case GS_GRAPHICS_UNIFORM_MAT4:  return 16 * sizeof(float);

        default:                        return 0;
    }
}

/* Graphics Interface Creation / Initialization / Shutdown / Destruction */
gs_graphics_i* gs_graphics_create()
{
    // Construct new graphics interface
    gs_graphics_i* gfx = gs_malloc_init(gs_graphics_i);

    // Construct internal data for opengl
    gfx->user_data = gs_malloc_init(gsgl_data_t);

    return gfx;
}

void gs_graphics_destroy(gs_graphics_i* graphics)
{
    // Free all resources (assuming they've been freed from the GPU already)
    if (graphics == NULL) return;

    gsgl_data_t* ogl = (gsgl_data_t*)graphics->user_data;

    // Free all pipeline data
    if (ogl->pipelines) {
        for (uint32_t i = 1; i < (uint32_t)gs_slot_array_size(ogl->pipelines); ++i) {
            gs_dyn_array_free(ogl->pipelines->data[i].layout);
        }
    }

    // Free all render pass data
    if (ogl->render_passes) {
        for (uint32_t i = 1; i < (uint32_t)gs_slot_array_size(ogl->render_passes); ++i) {
            gs_dyn_array_free(ogl->render_passes->data[i].color);
        }
    }

    gs_slot_array_free(ogl->shaders);
    gs_slot_array_free(ogl->vertex_buffers);
    gs_slot_array_free(ogl->index_buffers);
    gs_slot_array_free(ogl->frame_buffers);
    gs_slot_array_free(ogl->uniforms);
    gs_slot_array_free(ogl->textures);
    gs_slot_array_free(ogl->pipelines);
    gs_slot_array_free(ogl->render_passes);

    gs_free(graphics);
    graphics = NULL;
}

gs_result gs_graphics_init(gs_graphics_i* graphics)
{
    // Push back 0 handles into slot arrays (for 0 init validation)
    gsgl_data_t* ogl = (gsgl_data_t*)graphics->user_data;

    gs_slot_array_insert(ogl->shaders, 0);  
    gs_slot_array_insert(ogl->vertex_buffers, 0);   
    gs_slot_array_insert(ogl->index_buffers, 0);    
    gs_slot_array_insert(ogl->frame_buffers, 0);    
    gs_slot_array_insert(ogl->textures, 0);

    gsgl_uniform_t uni = gs_default_val();
    gsgl_pipeline_t pip = gs_default_val();
    gsgl_render_pass_t rp = gs_default_val();

    gs_slot_array_insert(ogl->uniforms, uni);
    gs_slot_array_insert(ogl->pipelines, pip);
    gs_slot_array_insert(ogl->render_passes, rp);

    // Construct vao then bind
    glGenVertexArrays(1, &ogl->cache.vao);      
    glBindVertexArray(ogl->cache.vao);

    // Reset data cache for rendering ops
    gsgl_reset_data_cache(&ogl->cache);

    // Init info object
    gs_graphics_info_t* info = &gs_engine_subsystem(graphics)->info;

    // Major/Minor version
    glGetIntegerv(GL_MAJOR_VERSION, (GLint*)&info->major_version);
    glGetIntegerv(GL_MINOR_VERSION, (GLint*)&info->minor_version);

    // Compute shader info
    info->compute.available = info->major_version >= 4 && info->minor_version >= 3;
    if (info->compute.available)
    {
        // Work group counts
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, (GLint*)&info->compute.max_work_group_count[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, (GLint*)&info->compute.max_work_group_count[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, (GLint*)&info->compute.max_work_group_count[2]);
        // Work group sizes
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, (GLint*)&info->compute.max_work_group_size[0]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, (GLint*)&info->compute.max_work_group_size[1]);
        glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, (GLint*)&info->compute.max_work_group_size[2]);
        // Work group invocations
        glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, (GLint*)&info->compute.max_work_group_invocations);
    }

    return GS_RESULT_SUCCESS;
}

gs_result gs_graphics_shutdown(gs_graphics_i* graphics)
{
    return GS_RESULT_SUCCESS;
}

/* Resource Creation */
gs_handle(gs_graphics_texture_t) gs_graphics_texture_create(gs_graphics_texture_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    gsgl_texture_t tex;
    uint32_t width = desc->width;
    uint32_t height = desc->height;
    void* data = desc->data;

    // TODO(john): allow for user to specify explicitly whether to allocate texture as 'render buffer storage'

    // Construct 'normal' texture
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);

    // Construct texture based on appropriate format
    switch(desc->format) 
    {
        case GS_GRAPHICS_TEXTURE_FORMAT_A8:                 glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA, width, height, 0, GL_ALPHA, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_R8:                 glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGB8:               glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA8:              glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA16F:            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, width, height, 0, GL_RGBA, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_RGBA32F:            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH8:             glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH16:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT16, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;              
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F:           glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH24_STENCIL8:   glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, data);
        case GS_GRAPHICS_TEXTURE_FORMAT_DEPTH32F_STENCIL8:  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH32F_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_FLOAT_32_UNSIGNED_INT_24_8_REV, data); break;

        // NOTE(john): Because Apple is a shit company, I have to section this off and provide support for 4.1 only features.
        // case GS_GRAPHICS_TEXTURE_FORMAT_STENCIL8:            glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT8, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, data); break;
        default: break;
    }

    int32_t mag_filter = desc->mag_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;
    int32_t min_filter = desc->min_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST : GL_LINEAR;

    if (desc->num_mips) {
        if (desc->min_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST) {
            min_filter = desc->mip_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_NEAREST_MIPMAP_NEAREST : 
                GL_NEAREST_MIPMAP_LINEAR;
        } 
        else {
            min_filter = desc->mip_filter == GS_GRAPHICS_TEXTURE_FILTER_NEAREST ? GL_LINEAR_MIPMAP_NEAREST : 
                GL_NEAREST_MIPMAP_LINEAR;
        }
    }

    int32_t texture_wrap_s = desc->wrap_s == GS_GRAPHICS_TEXTURE_WRAP_REPEAT ? GL_REPEAT : 
                         desc->wrap_s == GS_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : 
                         desc->wrap_s == GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE : 
                         GL_CLAMP_TO_BORDER;
    int32_t texture_wrap_t = desc->wrap_t == GS_GRAPHICS_TEXTURE_WRAP_REPEAT ? GL_REPEAT : 
                         desc->wrap_t == GS_GRAPHICS_TEXTURE_WRAP_MIRRORED_REPEAT ? GL_MIRRORED_REPEAT : 
                         desc->wrap_t == GS_GRAPHICS_TEXTURE_WRAP_CLAMP_TO_EDGE ? GL_CLAMP_TO_EDGE : 
                         GL_CLAMP_TO_BORDER;

    if (desc->num_mips) {
        glGenerateMipmap(GL_TEXTURE_2D);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, texture_wrap_s);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, texture_wrap_t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);

    // Unbind buffers
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);

    // Add texture to internal resource pool and return handle
    return (gs_handle_create(gs_graphics_texture_t, gs_slot_array_insert(ogl->textures, tex)));
}

gs_handle(gs_graphics_buffer_t) gs_graphics_buffer_create(gs_graphics_buffer_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    gs_handle(gs_graphics_buffer_t) hndl = gs_default_val();
    gsgl_buffer_t buffer = 0;

    switch (desc->type)
    {
        // Vertex Buffer
        default:
        case GS_GRAPHICS_BUFFER_VERTEX:
        {
            // Assert if data isn't filled for vertex data when static draw enabled
            if (desc->usage == GS_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
                gs_println("Error: Vertex buffer desc must contain data when GS_GRAPHICS_BUFFER_USAGE_STATIC set.");
                gs_assert(false);
            } 

            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ARRAY_BUFFER, buffer);
            glBufferData(GL_ARRAY_BUFFER, desc->size, desc->data, gsgl_buffer_usage_to_gl_enum(desc->usage));
            glBindBuffer(GL_ARRAY_BUFFER, 0);
            hndl = gs_handle_create(gs_graphics_buffer_t, gs_slot_array_insert(ogl->vertex_buffers, buffer));

        } break;

        // Index Buffer
        case GS_GRAPHICS_BUFFER_INDEX:
        {
            // Assert if data isn't filled for vertex data when static draw enabled
            if (desc->usage == GS_GRAPHICS_BUFFER_USAGE_STATIC && !desc->data) {
                gs_println("Error: Index buffer desc must contain data when GS_GRAPHICS_BUFFER_USAGE_STATIC set.");
                gs_assert(false);
            }

            glGenBuffers(1, &buffer);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, desc->size, desc->data, gsgl_buffer_usage_to_gl_enum(desc->usage));
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
            hndl = gs_handle_create(gs_graphics_buffer_t, gs_slot_array_insert(ogl->index_buffers, buffer));

        } break;

        case GS_GRAPHICS_BUFFER_FRAME:
        {
            // Construct and bind frame buffer
            glGenFramebuffers(1, &buffer);
            hndl = gs_handle_create(gs_graphics_buffer_t, gs_slot_array_insert(ogl->frame_buffers, buffer));

        } break;

        case GS_GRAPHICS_BUFFER_UNIFORM:
        {
            // Assert if data isn't named
            if (desc->name == NULL) {
                gs_println("Warning: Uniform buffer must be named for OpenGL.");
            }

            uint32_t ct = (uint32_t)desc->size / (uint32_t)sizeof(gs_graphics_uniform_desc_t);
            if (ct < 1) {
                gs_println("Warning: Uniform buffer description is empty.");
                return gs_handle_invalid(gs_graphics_buffer_t);
            }

            gsgl_uniform_t u = gs_default_val();
            u.name = desc->name;
            // If size > 1, store type as uniform buffer, other store type as type of first element of data description
            u.type = ct > 1 ? GSGL_UNIFORMTYPE_UNIFORM_BLOCK : 
                gsgl_uniform_type_to_gl_uniform_type((gs_graphics_uniform_type)((gs_graphics_uniform_desc_t*)desc->data)[0].type); 
            u.location = UINT32_MAX;

            // Calculate size
            for (uint32_t i = 0; i < ct; ++i) {
                u.size += gsgl_uniform_data_size_in_bytes((gs_graphics_uniform_type)((gs_graphics_uniform_desc_t*)desc->data)[i].type);
            }

            hndl = gs_handle_create(gs_graphics_buffer_t, gs_slot_array_insert(ogl->uniforms, u));

        } break;

        case GS_GRAPHICS_BUFFER_SAMPLER:
        {
            // Assert if data isn't named
            if (desc->name == NULL) {
                gs_println("Warning: Uniform buffer must be named for OpenGL.");
            }

            gsgl_uniform_t u = gs_default_val();
            u.name = desc->name;
            u.type = GSGL_UNIFORMTYPE_SAMPLER2D;
            u.location = UINT32_MAX;

            hndl = gs_handle_create(gs_graphics_buffer_t, gs_slot_array_insert(ogl->uniforms, u));
        } break;
    }

    return hndl;
}

#define GSGL_GRAPHICS_SHADER_PIPELINE_GFX       0x01
#define GSGL_GRAPHICS_SHADER_PIPELINE_COMPUTE   0x02
#define GSGL_GRAPHICS_MAX_SID                   128

gs_handle(gs_graphics_shader_t) gs_graphics_shader_create(gs_graphics_shader_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
    gsgl_shader_t shader = 0;
    uint32_t pip = 0x00;

    uint32_t sid_ct = 0;
    uint32_t sids[GSGL_GRAPHICS_MAX_SID] = gs_default_val();

    // Create shader program
    shader = glCreateProgram();

    uint32_t ct = (uint32_t)desc->size / (uint32_t)sizeof(gs_graphics_shader_source_desc_t);
    for (uint32_t i = 0; i < ct; ++i) 
    {
        if (desc->sources[i].type == GS_GRAPHICS_SHADER_STAGE_VERTEX) pip |= GSGL_GRAPHICS_SHADER_PIPELINE_GFX;
        if (desc->sources[i].type == GS_GRAPHICS_SHADER_STAGE_COMPUTE) pip |= GSGL_GRAPHICS_SHADER_PIPELINE_COMPUTE;

        // Validity Check: Desc must have vertex source if compute not selected. All other source is optional.
        if ((pip & GSGL_GRAPHICS_SHADER_PIPELINE_COMPUTE) && ((pip & GSGL_GRAPHICS_SHADER_PIPELINE_GFX))) {
            gs_println("Error: Cannot have compute and graphics stages for shader program.");
            gs_assert(false);
        }

        uint32_t stage = gsgl_shader_stage_to_gl_stage(desc->sources[i].type);
        uint32_t sid = glCreateShader(stage);

        if (!sid) {
            gs_println ("Error: Failed to allocate memory for shader: '%s': stage: {put stage id here}.", desc->name);
            gs_assert(sid);
        }

        // Set source
        glShaderSource(sid, 1, &desc->sources[i].source, NULL);

        // Compile shader
        glCompileShader(sid);

        //Check for errors
        GLint success = 0;
        glGetShaderiv(sid, GL_COMPILE_STATUS, &success);

        if (success == GL_FALSE)
        {
            GLint max_len = 0;
            glGetShaderiv(sid, GL_INFO_LOG_LENGTH, &max_len);

            char* log = (char*)gs_malloc(max_len);
            memset(log, 0, max_len);

            //The max_len includes the NULL character
            glGetShaderInfoLog(sid, max_len, &max_len, log);
            
            // Delete shader.
            glDeleteShader(shader);

            //Provide the infolog
            gs_println("Opengl::opengl_compile_shader::shader: '%s'\nFAILED_TO_COMPILE: %s\n %s", desc->name, log, desc->sources[i].source);

            free(log);
            log = NULL;

            gs_assert(false);
        }

        // Attach shader to program
         glAttachShader(shader, sid);

        // Add to shader array
         sids[sid_ct++] = sid;
    }

    // Link shaders into final program
    glLinkProgram(shader);

    //Create info log for errors
    s32 is_linked = 0;
    glGetProgramiv(shader, GL_LINK_STATUS, (s32*)&is_linked);
    if (is_linked == GL_FALSE)
    {
        GLint max_len = 0;
        glGetProgramiv(shader, GL_INFO_LOG_LENGTH, &max_len);

        char* log = (char*)gs_malloc(max_len);
        memset(log, 0, max_len);
        glGetProgramInfoLog(shader, max_len, &max_len, log); 

        // Print error
        gs_println("Error: Fail To Link::opengl_link_shaders::shader: '%s', \n%s", desc->name, log);

        // //We don't need the program anymore.
        glDeleteProgram(shader);

        free(log);
        log = NULL;

        // Just assert for now
        gs_assert(false);
    }

    // Free shaders after use
    for (uint32_t i = 0; i < sid_ct; ++i) {
        glDeleteShader(sids[i]);
    }

    // Add to pool and return handle
    return (gs_handle_create(gs_graphics_shader_t, gs_slot_array_insert(ogl->shaders, shader)));
}

gs_handle(gs_graphics_render_pass_t) gs_graphics_render_pass_create(gs_graphics_render_pass_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    gsgl_render_pass_t pass = gs_default_val();

    // Set fbo
    pass.fbo = desc->fbo;

    // Set color attachments
    uint32_t ct = (uint32_t)desc->color_size / (uint32_t)sizeof(gs_handle(gs_graphics_texture_t));
    for (uint32_t i = 0; i < ct; ++i) 
    {
        gs_dyn_array_push(pass.color, desc->color[i]);
    }
    // Set depth attachment
    pass.depth = desc->depth;

    // Create handle and return
    return (gs_handle_create(gs_graphics_render_pass_t, gs_slot_array_insert(ogl->render_passes, pass)));
}

gs_handle(gs_graphics_pipeline_t) gs_graphics_pipeline_create(gs_graphics_pipeline_desc_t* desc)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    gsgl_pipeline_t pipe = gs_default_val();

    // Add states
    pipe.blend = desc->blend;
    pipe.depth = desc->depth;
    pipe.raster = desc->raster;
    pipe.stencil = desc->stencil;
    pipe.compute = desc->compute;

    // Add layout
    uint32_t ct = (uint32_t)desc->layout.size / (uint32_t)sizeof(gs_graphics_vertex_attribute_desc_t);
    gs_dyn_array_reserve(pipe.layout, ct);
    for (uint32_t i = 0; i < ct; ++i) {
        gs_dyn_array_push(pipe.layout, desc->layout.attrs[i]);
    }

    // Create handle and return
    return (gs_handle_create(gs_graphics_pipeline_t, gs_slot_array_insert(ogl->pipelines, pipe)));
}

/* Resource Destruction */
void gs_graphics_texture_destroy(gs_handle(gs_graphics_texture_t) hndl)
{
}

void gs_graphics_buffer_destroy(gs_handle(gs_graphics_buffer_t) hndl)
{
}

void gs_graphics_shader_destroy(gs_handle(gs_graphics_shader_t) hndl)
{
}

void gs_graphics_uniform_block_destroy(gs_handle(gs_graphics_uniform_block_t) hndl)
{
}

void gs_graphics_render_pass_destroy(gs_handle(gs_graphics_render_pass_t) hndl)
{
}

void gs_graphics_pipeline_destroy(gs_handle(gs_graphics_pipeline_t) hndl)
{
}

/* Resource Update*/
void gs_graphics_texture_update(gs_handle(gs_graphics_texture_t) hndl, gs_graphics_texture_desc_t* desc)
{
}

void gs_graphics_buffer_update(gs_handle(gs_graphics_buffer_t) hndl, gs_graphics_buffer_desc_t* desc)
{
}

#define __ogl_push_command(CB, OP_CODE, ...)\
do {\
    gs_byte_buffer_write(&CB->commands, u32, (u32)OP_CODE);\
    __VA_ARGS__\
    CB->num_commands++;\
} while (0)

/* Command Buffer Ops: Pipeline / Pass / Bind / Draw */
void gs_graphics_begin_render_pass(gs_command_buffer_t* cb, gs_handle(gs_graphics_render_pass_t) hndl, gs_graphics_render_pass_action_t* actions, size_t actions_size)
{
    __ogl_push_command(cb, GS_OPENGL_OP_BEGIN_RENDER_PASS, {
        gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
        uint32_t count = (uint32_t)((size_t)actions_size / (size_t)sizeof(gs_graphics_render_pass_action_t));
        gs_byte_buffer_write(&cb->commands, uint32_t, count);
        for (uint32_t i = 0; i < count; ++i) {
            gs_byte_buffer_write(&cb->commands, gs_graphics_render_pass_action_t, actions[i]);
        }
    });
}

void gs_graphics_end_render_pass(gs_command_buffer_t* cb)
{
    __ogl_push_command(cb, GS_OPENGL_OP_END_RENDER_PASS, {
        // Nothing...
    });
}

void gs_graphics_set_viewport(gs_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    __ogl_push_command(cb, GS_OPENGL_OP_SET_VIEWPORT, {
        gs_byte_buffer_write(&cb->commands, uint32_t, x);
        gs_byte_buffer_write(&cb->commands, uint32_t, y);
        gs_byte_buffer_write(&cb->commands, uint32_t, w);
        gs_byte_buffer_write(&cb->commands, uint32_t, h);
    });
}

void gs_graphics_set_view_scissor(gs_command_buffer_t* cb, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
    __ogl_push_command(cb, GS_OPENGL_OP_SET_VIEW_SCISSOR, {
        gs_byte_buffer_write(&cb->commands, uint32_t, x);
        gs_byte_buffer_write(&cb->commands, uint32_t, y);
        gs_byte_buffer_write(&cb->commands, uint32_t, w);
        gs_byte_buffer_write(&cb->commands, uint32_t, h);
    });
}

void gs_graphics_texture_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_texture_t) hndl, gs_graphics_texture_desc_t* desc)
{
}

void gs_graphics_buffer_request_update(gs_command_buffer_t* cb, gs_handle(gs_graphics_buffer_t) hndl, gs_graphics_buffer_desc_t* desc)
{
    // Need to just store all buffers into single buffer slot array. This is goofy.
    if (!hndl.id) return;

    // Write type
    __ogl_push_command(cb, GS_OPENGL_OP_REQUEST_BUFFER_UPDATE, {

        // Write handle id
        gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
        // Write type
        gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_type, desc->type);
        // Write usage
        gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_usage_type, desc->usage);
        // Write data size
        gs_byte_buffer_write(&cb->commands, size_t, desc->size);
        // Write data offset
        gs_byte_buffer_write(&cb->commands, size_t, desc->update.offset);
        // Write data update type
        gs_byte_buffer_write(&cb->commands, gs_graphics_buffer_update_type, desc->update.type);
        // Write data
        gs_byte_buffer_write_bulk(&cb->commands, desc->data, desc->size);
    });
}

// struct {
//     struct {
//         gs_graphics_bind_buffer_desc_t* decl;
//         size_t size;
//     } image_buffers;
// } compute;

void gs_graphics_bind_bindings(gs_command_buffer_t* cb, gs_graphics_bind_desc_t* binds)
{
    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
 
    __ogl_push_command(cb, GS_OPENGL_OP_BIND_BINDINGS,
    {
        // Get counts from buffers
        uint32_t vct = binds->vertex_buffers.decl ? binds->vertex_buffers.size ? binds->vertex_buffers.size / sizeof(gs_graphics_bind_buffer_desc_t) : 1 : 0;
        uint32_t ict = binds->index_buffers.decl ? binds->index_buffers.size ? binds->index_buffers.size / sizeof(gs_graphics_bind_buffer_desc_t) : 1 : 0;
        uint32_t uct = binds->uniform_buffers.decl ? binds->uniform_buffers.size ? binds->uniform_buffers.size / sizeof(gs_graphics_bind_buffer_desc_t) : 1 : 0;
        uint32_t sct = binds->sampler_buffers.decl ? binds->sampler_buffers.size ? binds->sampler_buffers.size / sizeof(gs_graphics_bind_buffer_desc_t) : 1 : 0;
        uint32_t ibc = binds->compute.image_buffers.decl ? binds->compute.image_buffers.size ? binds->compute.image_buffers.size / sizeof(gs_graphics_bind_buffer_desc_t) : 1 : 0;

        // Determine total count to write into command buffer
        uint32_t ct = vct + ict + uct + sct + ibc;
        gs_byte_buffer_write(&cb->commands, uint32_t, ct);

        // Determine if need to clear any previous vertex buffers (if vct != 0)
        gs_byte_buffer_write(&cb->commands, bool, (vct != 0));

        // Vertex buffers
        for (uint32_t i = 0; i < vct; ++i)
        {
            gs_graphics_bind_buffer_desc_t* decl = &binds->vertex_buffers.decl[i];
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_VERTEX_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
            gs_byte_buffer_write(&cb->commands, size_t, decl->offset);
            gs_byte_buffer_write(&cb->commands, gs_graphics_vertex_data_type, decl->data_type);
        }

        // Index buffers
        for (uint32_t i = 0; i < ict; ++i)
        {
            gs_graphics_bind_buffer_desc_t* decl = &binds->index_buffers.decl[i];
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_INDEX_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
        }

        // Uniform buffers
        for (uint32_t i = 0; i < uct; ++i)
        {
            gs_graphics_bind_buffer_desc_t* decl = &binds->uniform_buffers.decl[i];

            // if (id && gs_slot_array_exists(ogl->uniforms, id))
            {
                uint32_t id = decl->buffer.id;
                size_t sz = (size_t)(gs_slot_array_getp(ogl->uniforms, id))->size;
                gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_UNIFORM_BUFFER);
                gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
                gs_byte_buffer_write(&cb->commands, size_t, sz);
                gs_byte_buffer_write_bulk(&cb->commands, decl->data, sz);
            }
        }

        // Sampler buffers
        for (uint32_t i = 0; i < sct; ++i)
        {
            gs_graphics_bind_buffer_desc_t* decl = &binds->sampler_buffers.decl[i];
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_SAMPLER_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->buffer.id);
            gs_byte_buffer_write(&cb->commands, uint32_t, ((gs_handle(gs_graphics_texture_t)*)decl->data)->id);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->binding);
        }

        // Image buffers
        for (uint32_t i = 0; i < ibc; ++i)
        {
            gs_graphics_bind_buffer_desc_t* decl = &binds->compute.image_buffers.decl[i];
            gs_byte_buffer_write(&cb->commands, gs_graphics_bind_type, GS_GRAPHICS_BIND_IMAGE_BUFFER);
            gs_byte_buffer_write(&cb->commands, uint32_t, ((gs_handle(gs_graphics_texture_t)*)decl->data)->id);
            gs_byte_buffer_write(&cb->commands, uint32_t, decl->binding);
            gs_byte_buffer_write(&cb->commands, gs_graphics_access_type, decl->access);
            gs_byte_buffer_write(&cb->commands, gs_graphics_texture_format_type, decl->format);
        }
    });
}

void gs_graphics_bind_pipeline(gs_command_buffer_t* cb, gs_handle(gs_graphics_pipeline_t) hndl)
{
    // NOTE(john): Not sure if this is safe in the future, since the data for pipelines is on the main thread and MIGHT be tampered with on a separate thread.
    __ogl_push_command(cb, GS_OPENGL_OP_BIND_PIPELINE, {
        gs_byte_buffer_write(&cb->commands, uint32_t, hndl.id);
    });
}

void gs_graphics_draw(gs_command_buffer_t* cb, uint32_t start, uint32_t count, uint32_t instance_count)
{
    __ogl_push_command(cb, GS_OPENGL_OP_DRAW, {
        gs_byte_buffer_write(&cb->commands, uint32_t, start);
        gs_byte_buffer_write(&cb->commands, uint32_t, count);
        gs_byte_buffer_write(&cb->commands, uint32_t, instance_count);
    });
}

void gs_graphics_dispatch_compute(gs_command_buffer_t* cb, uint32_t num_x_groups, uint32_t num_y_groups, uint32_t num_z_groups)
{
    __ogl_push_command(cb, GS_OPENGL_OP_DISPATCH_COMPUTE, {
        gs_byte_buffer_write(&cb->commands, uint32_t, num_x_groups);
        gs_byte_buffer_write(&cb->commands, uint32_t, num_y_groups);
        gs_byte_buffer_write(&cb->commands, uint32_t, num_z_groups);
    });
}

/* Submission (Main Thread) */
void gs_graphics_submit_command_buffer(gs_command_buffer_t* cb)
{
    /*
        // Structure of command: 
            - Op code
            - Data packet
    */

    gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

    // Set read position of buffer to beginning
    gs_byte_buffer_seek_to_beg(&cb->commands);

    // For each command in buffer
    gs_for_range(cb->num_commands)
    {
        // Read in op code of command
        gs_byte_buffer_readc(&cb->commands, gs_opengl_op_code_type, op_code);

        switch (op_code)
        {
            case GS_OPENGL_OP_BEGIN_RENDER_PASS:
            {
                // Bind render pass stuff
                gs_byte_buffer_readc(&cb->commands, uint32_t, rpid);

                // If render pass exists, then we'll bind frame buffer and attachments 
                if (rpid && gs_slot_array_exists(ogl->render_passes, rpid)) 
                {
                    gsgl_render_pass_t* rp = gs_slot_array_getp(ogl->render_passes, rpid);

                    // Bind frame buffer since it actually exists
                    if (rp->fbo.id && gs_slot_array_exists(ogl->frame_buffers, rp->fbo.id)) 
                    {
                        // Bind frame buffer
                        glBindFramebuffer(GL_FRAMEBUFFER, gs_slot_array_get(ogl->frame_buffers, rp->fbo.id));

                        // Bind color attachments
                        for (uint32_t r = 0; r < gs_dyn_array_size(rp->color); ++r)
                        {
                            uint32_t cid = rp->color[r].id;
                            if (cid && gs_slot_array_exists(ogl->textures, cid)) 
                            {
                                gsgl_texture_t rt = gs_slot_array_get(ogl->textures, cid);
                                glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + r, rt, 0);
                            }
                        }
                    }
                }

                // Actions
                gs_byte_buffer_readc(&cb->commands, uint32_t, action_count);
                for (uint32_t j = 0; j < action_count; ++j)
                {
                    gs_byte_buffer_readc(&cb->commands, gs_graphics_render_pass_action_t, action);

                    // No clear
                    if (action.flag & GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_NONE) {
                        continue;
                    }

                    uint32_t bit = 0x00;

                    if (action.flag & GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_COLOR || action.flag == 0x00) {
                        glClearColor(action.color[0], action.color[1], action.color[2], action.color[3]);
                        bit |= GL_COLOR_BUFFER_BIT;
                    }
                    if (action.flag & GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_DEPTH || action.flag == 0x00) {
                        bit |= GL_DEPTH_BUFFER_BIT;
                    }
                    if (action.flag & GS_GRAPHICS_RENDER_PASS_ACTION_CLEAR_STENCIL || action.flag == 0x00) {
                        bit |= GL_STENCIL_BUFFER_BIT;
                    }

                    glClear(bit);
                }
            } break;

            case GS_OPENGL_OP_END_RENDER_PASS:
            {
                gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;
                gsgl_reset_data_cache(&ogl->cache);

                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                glDisable(GL_SCISSOR_TEST);
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_STENCIL_TEST);
                glDisable(GL_BLEND);
            } break;

            case GS_OPENGL_OP_SET_VIEWPORT:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, x);
                gs_byte_buffer_readc(&cb->commands, uint32_t, y);
                gs_byte_buffer_readc(&cb->commands, uint32_t, w);
                gs_byte_buffer_readc(&cb->commands, uint32_t, h);

                glViewport(x, y, w, h);
            } break;

            case GS_OPENGL_OP_SET_VIEW_SCISSOR:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, x);
                gs_byte_buffer_readc(&cb->commands, uint32_t, y);
                gs_byte_buffer_readc(&cb->commands, uint32_t, w);
                gs_byte_buffer_readc(&cb->commands, uint32_t, h);

                glEnable(GL_SCISSOR_TEST);
                glScissor(x, y, w, h);
            } break;

            case GS_OPENGL_OP_BIND_BINDINGS:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, ct);

                // Determine if need to clear any previous vertex buffers here
                gs_byte_buffer_readc(&cb->commands, bool, clear_vertex_buffers);

                // Clear previous vertex decls if necessary
                if (clear_vertex_buffers) {
                    gs_dyn_array_clear(ogl->cache.vdecls);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                }

                for (uint32_t i = 0; i < ct; ++i) 
                {
                    gs_byte_buffer_readc(&cb->commands, gs_graphics_bind_type, type);
                    switch (type)
                    {
                        case GS_GRAPHICS_BIND_VERTEX_BUFFER:
                        {
                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                            gs_byte_buffer_readc(&cb->commands, size_t, offset);
                            gs_byte_buffer_readc(&cb->commands, gs_graphics_vertex_data_type, data_type);

                            if (!id || !gs_slot_array_exists(ogl->vertex_buffers, id)) 
                            {
                                gs_timed_action(60, 
                                {
                                    gs_println("Warning:Opengl:BindBindings:VertexBuffer %d does not exist.", id);
                                    continue;
                                });
                            }

                            // Grab vbo to bind
                            gsgl_buffer_t vbo = gs_slot_array_get(ogl->vertex_buffers, id);

                            // If the data type is non-interleaved, then push size into vertex buffer decl
                            gsgl_vertex_buffer_decl_t vbo_decl = gs_default_val();
                            vbo_decl.vbo = vbo;
                            vbo_decl.data_type = data_type;
                            vbo_decl.offset = offset;

                            // Cache vertex buffer for later use
                            gs_dyn_array_push(ogl->cache.vdecls, vbo_decl);

                        } break;

                        case GS_GRAPHICS_BIND_INDEX_BUFFER:
                        {
                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);

                            if (!gs_slot_array_exists(ogl->index_buffers, id)) 
                            {
                                gs_timed_action(60, 
                                {
                                    gs_println("Warning:Opengl:BindBindings:IndexBuffer %d does not exist.", id);
                                });
                            } 
                            else 
                            {
                                // Store in cache
                                ogl->cache.ibo = id;
                            }
                        } break;

                        case GS_GRAPHICS_BIND_UNIFORM_BUFFER:
                        {
                            // Read slot id of uniform buffer
                            gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                            // Read data size
                            gs_byte_buffer_readc(&cb->commands, size_t, sz);

                            // Check buffer id. If invalid, then we can't operate, and instead just need to pass over the data.
                            if (!id || !gs_slot_array_exists(ogl->uniforms, id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Uniform Buffer:Uniform %d does not exist.", ogl->cache.pipeline.id);
                                });
                                gs_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !gs_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)){
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Uniform Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id);
                                });
                                gs_byte_buffer_advance_position(&cb->commands, sz);
                                continue;
                            }

                            gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get uniform
                            gsgl_uniform_t* u = gs_slot_array_getp(ogl->uniforms, id);

                            // Get bound shader from pipeline (either compute or raster)
                            uint32_t sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location. If UINT32_T max, then must construct and place location
                            if (u->location == UINT32_MAX && u->location != UINT32_MAX - 1) 
                            {
                                if (!sid || !gs_slot_array_exists(ogl->shaders, sid)) {
                                    gs_timed_action(60, {
                                        gs_println("Warning:Bind Uniform Buffer:Shader %d does not exist.", sid);
                                    });
                                    gs_byte_buffer_advance_position(&cb->commands, sz);
                                    continue;
                                }

                                gsgl_shader_t shader = gs_slot_array_get(ogl->shaders, sid);

                                // Grab location of uniform
                                u->location = glGetUniformLocation(shader, u->name ? u->name : "__EMPTY_UNIFORM_NAME");

                                if (u->location >= UINT32_MAX) {
                                    gs_println("Warning: uniform not found: \"%s\"", u->name);
                                    u->location = UINT32_MAX - 1;
                                }
                            }

                            // Switch on uniform type to upload data
                            switch (u->type) 
                            {
                                case GSGL_UNIFORMTYPE_FLOAT: 
                                {
                                    gs_assert(sz == sizeof(float));
                                    gs_byte_buffer_read_bulkc(&cb->commands, float, v, sz);
                                    glUniform1f(u->location, v);
                                } break;

                                case GSGL_UNIFORMTYPE_INT: 
                                {
                                    gs_assert(sz == sizeof(int32_t));
                                    gs_byte_buffer_read_bulkc(&cb->commands, int32_t, v, sz);
                                    glUniform1i(u->location, v);
                                } break;

                                case GSGL_UNIFORMTYPE_VEC2: 
                                {
                                    gs_assert(sz == sizeof(gs_vec2));
                                    gs_byte_buffer_read_bulkc(&cb->commands, gs_vec2, v, sz);
                                    glUniform2f(u->location, v.x, v.y);
                                } break;

                                case GSGL_UNIFORMTYPE_VEC3: 
                                {
                                    gs_assert(sz == sizeof(gs_vec3));
                                    gs_byte_buffer_read_bulkc(&cb->commands, gs_vec3, v, sz);
                                    glUniform3f(u->location, v.x, v.y, v.z);
                                } break;

                                case GSGL_UNIFORMTYPE_VEC4: 
                                {
                                    gs_assert(sz == sizeof(gs_vec4));
                                    gs_byte_buffer_read_bulkc(&cb->commands, gs_vec4, v, sz);
                                    glUniform4f(u->location, v.x, v.y, v.z, v.w);
                                } break;

                                case GSGL_UNIFORMTYPE_MAT4: 
                                {
                                    gs_assert(sz == sizeof(gs_mat4));
                                    gs_byte_buffer_read_bulkc(&cb->commands, gs_mat4, v, sz);
                                    glUniformMatrix4fv(u->location, 1, false, (float*)(v.elements));
                                } break;

                                default: {
                                    // Shouldn't hit here
                                } break;
                            }

                        } break;

                        case GS_GRAPHICS_BIND_SAMPLER_BUFFER:
                        {
                            // Read slot id of sampler buffer
                            gs_byte_buffer_readc(&cb->commands, uint32_t, sampler_slot_id);
                            // Read id texture
                            gs_byte_buffer_readc(&cb->commands, uint32_t, tex_slot_id);
                            // Read binding
                            gs_byte_buffer_readc(&cb->commands, uint32_t, binding);

                            // Grab uniform from sampler id
                            if (!sampler_slot_id || !gs_slot_array_exists(ogl->uniforms, sampler_slot_id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Sampler Buffer:Sampler %d does not exist.", sampler_slot_id);
                                });
                                continue;
                            }

                            // Grab texture from sampler id
                            if (!tex_slot_id || !gs_slot_array_exists(ogl->textures, tex_slot_id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Sampler Buffer:Texture %d does not exist.", tex_slot_id);
                                });
                                continue;
                            }

                            // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                            if (!ogl->cache.pipeline.id || !gs_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)){
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Sampler Buffer:Pipeline %d does not exist.", ogl->cache.pipeline.id);
                                });
                                continue;
                            }

                            gsgl_uniform_t* u = gs_slot_array_getp(ogl->uniforms, sampler_slot_id);
                            gsgl_texture_t tex = gs_slot_array_get(ogl->textures, tex_slot_id);
                            gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                            // Get bound shader from pipeline (either compute or raster)
                            uint32_t sid = pip->compute.shader.id ? pip->compute.shader.id : pip->raster.shader.id;

                            // Check uniform location. If UINT32_T max, then must construct and place location
                            if (u->location == UINT32_MAX && u->location != UINT32_MAX - 1) 
                            {
                                if (!sid || !gs_slot_array_exists(ogl->shaders, sid)) {
                                    gs_timed_action(60, {
                                        gs_println("Warning:Bind Uniform Buffer:Shader %d does not exist.", sid);
                                    });
                                    continue;
                                }

                                gsgl_shader_t shader = gs_slot_array_get(ogl->shaders, sid);

                                // Grab location of uniform
                                u->location = glGetUniformLocation(shader, u->name ? u->name : "__EMPTY_UNIFORM_NAME");

                                if (u->location >= UINT32_MAX) {
                                    gs_println("Warning: uniform not found: \"%s\"", u->name);
                                    u->location = UINT32_MAX - 1;
                                }
                            }

                            // Invalid texture
                            if (u->location == UINT32_MAX - 1) {
                                continue;
                            }

                            // Activate texture slot
                            glActiveTexture(GL_TEXTURE0 + binding);
                            // Bind texture
                            glBindTexture(GL_TEXTURE_2D, tex);
                            // Bind uniform
                            glUniform1i(u->location, binding);

                        } break;

                        case GS_GRAPHICS_BIND_IMAGE_BUFFER:
                        {
                            gs_byte_buffer_readc(&cb->commands, uint32_t, tex_slot_id);
                            gs_byte_buffer_readc(&cb->commands, uint32_t, binding);
                            gs_byte_buffer_readc(&cb->commands, gs_graphics_access_type, access);
                            gs_byte_buffer_readc(&cb->commands, gs_graphics_texture_format_type, format);

                            // Grab texture from sampler id
                            if (!tex_slot_id || !gs_slot_array_exists(ogl->textures, tex_slot_id)) {
                                gs_timed_action(60, {
                                    gs_println("Warning:Bind Image Buffer:Texture %d does not exist.", tex_slot_id);
                                });
                                continue;
                            }

                            gsgl_texture_t tex = gs_slot_array_get(ogl->textures, tex_slot_id);
                            uint32_t gl_access = gsgl_access_type_to_gl_access_type(access);
                            uint32_t gl_format = gsgl_texture_format_to_gl_texture_format(format);

                            // Bind image texture
                            // Activate texture slot (don't know if these are necessary)
                            // glActiveTexture(GL_TEXTURE0 + binding);
                            // glBindTexture(GL_TEXTURE_2D, tex);
                            glBindImageTexture(0, tex, 0, GL_FALSE, 0, gl_access, gl_format);
                        } break;

                        default: gs_assert(false); break;
                    }
                }

            } break;

            case GS_OPENGL_OP_BIND_PIPELINE:
            {
                // Bind pipeline stuff
                gs_byte_buffer_readc(&cb->commands, uint32_t, pipid);

                // Make sure pipeline exists
                if (!pipid || !gs_slot_array_exists(ogl->pipelines, pipid)) {
                    gs_println("Warning: Pipeline %d does not exist.", pipid);
                    continue;
                }
                
                // Reset cache
                gsgl_reset_data_cache(&ogl->cache);

               // Reset state as well
                gsgl_pipeline_state();

                /* Cache pipeline id */
                ogl->cache.pipeline = gs_handle_create(gs_graphics_pipeline_t, pipid);

                gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, pipid);

                /* Compute */ 
                // Early out if compute, since we're not doing a rasterization stage
                if (pip->compute.shader.id)
                {
                    /* Shader */
                    if (pip->compute.shader.id && gs_slot_array_exists(ogl->shaders, pip->compute.shader.id)) {
                        glUseProgram(gs_slot_array_get(ogl->shaders, pip->compute.shader.id));
                    } 
                    else {
                        gs_timed_action(60, {
                            gs_println("Warning:Opengl:BindPipeline:Compute:Shader %d does not exist.", pip->compute.shader.id);
                        });
                    }

                    continue;
                }

                /* Depth */
                if (!pip->depth.func) {
                    // If no depth function (default), then disable
                    glDisable(GL_DEPTH_TEST);
                }
                else {
                    glEnable(GL_DEPTH_TEST);    
                    gsgl_depth_func_to_gl_depth_func(pip->depth.func);
                }

                /* Stencil */
                if (!pip->stencil.func) {
                    // If no stencil function (default), then disable
                    glDisable(GL_STENCIL_TEST); 
                } else {
                    glEnable(GL_STENCIL_TEST);  
                    uint32_t func = gsgl_stencil_func_to_gl_stencil_func(pip->stencil.func);
                    uint32_t sfail = gsgl_stencil_op_to_gl_stencil_op(pip->stencil.sfail);
                    uint32_t dpfail = gsgl_stencil_op_to_gl_stencil_op(pip->stencil.dpfail);
                    uint32_t dppass = gsgl_stencil_op_to_gl_stencil_op(pip->stencil.dppass);
                    glStencilFunc(func, pip->stencil.ref, pip->stencil.mask);
                    glStencilOp(sfail, dpfail, dppass);
                }

                /* Blend */
                if (!pip->blend.func) {
                    glDisable(GL_BLEND);
                } else {
                    glEnable(GL_BLEND);
                    glBlendEquation(gsgl_blend_equation_to_gl_blend_eq(pip->blend.func));
                    glBlendFunc(gsgl_blend_mode_to_gl_blend_mode(pip->blend.src, GL_ONE), 
                        gsgl_blend_mode_to_gl_blend_mode(pip->blend.dst, GL_ZERO));

                    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                }

                /* Raster */
                // Face culling
                if (!pip->raster.face_culling) {
                    glDisable(GL_CULL_FACE);    
                } else {
                    glEnable(GL_CULL_FACE); 
                    glCullFace(gsgl_cull_face_to_gl_cull_face(pip->raster.face_culling));   
                }

                // Winding order
                glFrontFace(gsgl_winding_order_to_gl_winding_order(pip->raster.winding_order));

                /* Shader */
                if (pip->raster.shader.id && gs_slot_array_exists(ogl->shaders, pip->raster.shader.id)) {
                    glUseProgram(gs_slot_array_get(ogl->shaders, pip->raster.shader.id));
                } 
                else {
                    gs_timed_action(60, {
                        gs_println("Warning:Opengl:BindPipeline:Shader %d does not exist.", pip->raster.shader.id);
                    });
                }
            } break;

            case GS_OPENGL_OP_DISPATCH_COMPUTE:
            {
                gs_byte_buffer_readc(&cb->commands, uint32_t, num_x_groups);
                gs_byte_buffer_readc(&cb->commands, uint32_t, num_y_groups);
                gs_byte_buffer_readc(&cb->commands, uint32_t, num_z_groups);

                // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                if (ogl->cache.pipeline.id == 0 || !gs_slot_array_exists(ogl->pipelines, ogl->cache.pipeline.id)) {
                    gs_timed_action(60, {
                        gs_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    continue;
                }

                gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // If pipeline does not have a compute state bound, then leave
                if (!pip->compute.shader.id) {
                    gs_timed_action(60, {
                        gs_println("Warning:Opengl:DispatchCompute:Compute Pipeline not bound.");
                    });
                    continue;
                }

                // Dispatch shader 
                glDispatchCompute(num_x_groups, num_y_groups, num_z_groups); 
                // Memory barrier (make this specifically set in the pipeline state)
                glMemoryBarrier(GL_ALL_BARRIER_BITS);
            } break;

            case GS_OPENGL_OP_DRAW:
            {
                // Grab currently bound pipeline (TODO(john): assert if this isn't valid)
                gsgl_pipeline_t* pip = gs_slot_array_getp(ogl->pipelines, ogl->cache.pipeline.id);

                // Must have a vertex buffer bound to draw
                if (gs_dyn_array_empty(ogl->cache.vdecls)) {
                    gs_println("Error:Opengl:Draw: No vertex buffer bound.");
                    gs_assert(false);
                }

                // Keep track whether or not the data is to be instanced
                bool is_instanced = false;

                for (uint32_t i = 0; i < gs_dyn_array_size(pip->layout); ++i)
                {
                    // Vertex buffer to bind
                    uint32_t vbo_idx = pip->layout[i].buffer_idx;
                    gsgl_vertex_buffer_decl_t vdecl = vbo_idx < gs_dyn_array_size(ogl->cache.vdecls) ? ogl->cache.vdecls[vbo_idx] : ogl->cache.vdecls[0];
                    gsgl_buffer_t vbo = vdecl.vbo;

                    bool is_manual = pip->layout[i].stride || pip->layout[i].divisor || pip->layout[i].offset || vdecl.data_type == GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED;

                    // Bind buffer
                    glBindBuffer(GL_ARRAY_BUFFER, vbo);

                    // Stride of vertex attribute
                    size_t stride = is_manual ? pip->layout[i].stride : 
                                        gsgl_calculate_vertex_size_in_bytes(pip->layout, gs_dyn_array_size(pip->layout));

                    // Byte offset of vertex attribute (if non-interleaved data, then grab offset from decl instead)
                    size_t offset = vdecl.data_type == GS_GRAPHICS_VERTEX_DATA_NONINTERLEAVED ? vdecl.offset : is_manual ? pip->layout[i].offset : 
                                        gsgl_get_vertex_attr_byte_offest(pip->layout, i);

                    // If there is a vertex divisor for this layout, then we'll draw instanced
                    is_instanced |= (pip->layout[i].divisor != 0);

                    // Enable the vertex attribute pointer
                    glEnableVertexAttribArray(i);

                    switch (pip->layout[i].format)
                    {
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT4: glVertexAttribPointer(i, 4, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT3: glVertexAttribPointer(i, 3, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT2: glVertexAttribPointer(i, 2, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_FLOAT:  glVertexAttribPointer(i, 1, GL_FLOAT, GL_FALSE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT4:  glVertexAttribIPointer(i, 4, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT3:  glVertexAttribIPointer(i, 3, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT2:  glVertexAttribIPointer(i, 2, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_UINT:   glVertexAttribIPointer(i, 1, GL_UNSIGNED_INT, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE:   glVertexAttribPointer(i, 1, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE2:  glVertexAttribPointer(i, 2, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE3:  glVertexAttribPointer(i, 3, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;
                        case GS_GRAPHICS_VERTEX_ATTRIBUTE_BYTE4:  glVertexAttribPointer(i, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, gs_int2voidp(offset)); break;

                        // Shouldn't get here
                        default: {
                            gs_assert(false);
                        } break;
                    }
                    // Set up divisor (for instancing)
                    glVertexAttribDivisor(i, pip->layout[i].divisor);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);
                } 

				// Bind element buffer
				if (ogl->cache.ibo) { 
					glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gs_slot_array_get(ogl->index_buffers, ogl->cache.ibo));
				}

                // Bind all vertex buffers after setting up data and pointers
                for (uint32_t i = 0; i < gs_dyn_array_size(ogl->cache.vdecls); ++i) {
                    glBindBuffer(GL_ARRAY_BUFFER, ogl->cache.vdecls[i].vbo);
                } 

                // Draw based on bound primitive type in raster 
                gs_byte_buffer_readc(&cb->commands, uint32_t, start);
                gs_byte_buffer_readc(&cb->commands, uint32_t, count);
                gs_byte_buffer_readc(&cb->commands, uint32_t, instance_count);

                uint32_t prim = gsgl_primitive_to_gl_primitive(pip->raster.primitive);
                uint32_t itype = gsgl_index_buffer_size_to_gl_index_type(pip->raster.index_buffer_element_size);

                // Draw
                if (ogl->cache.ibo) { 
                    if (is_instanced)   glDrawElementsInstanced(prim, count, itype, gs_int2voidp(start), instance_count);
                    else                glDrawElements(prim, count, itype, gs_int2voidp(start));
                } 
                else {
                    if (is_instanced)   glDrawArraysInstanced(prim, start, count, instance_count);
                    else                glDrawArrays(prim, start, count);
                }

            } break;

            case GS_OPENGL_OP_REQUEST_BUFFER_UPDATE:
            {
                gsgl_data_t* ogl = (gsgl_data_t*)gs_engine_subsystem(graphics)->user_data;

                // Read handle id
                gs_byte_buffer_readc(&cb->commands, uint32_t, id);
                // Read type
                gs_byte_buffer_readc(&cb->commands, gs_graphics_buffer_type, type);
                // Read usage
                gs_byte_buffer_readc(&cb->commands, gs_graphics_buffer_usage_type, usage);
                // Read data size
                gs_byte_buffer_readc(&cb->commands, size_t, sz);
                // Read data offset
                gs_byte_buffer_readc(&cb->commands, size_t, offset);
                // Read update type
                gs_byte_buffer_readc(&cb->commands, gs_graphics_buffer_update_type, update_type);

                switch (type)
                {
                    // Vertex Buffer
                    default:
                    case GS_GRAPHICS_BUFFER_VERTEX:
                    {
                        gsgl_buffer_t buffer = gs_slot_array_get(ogl->vertex_buffers, id);
                        glBindBuffer(GL_ARRAY_BUFFER, buffer);
                        if (update_type == GS_GRAPHICS_BUFFER_UPDATE_SUBDATA) 
                        {
                            glBufferSubData(GL_ARRAY_BUFFER, offset, sz, (u8*)(cb->commands.data + cb->commands.position));
                        } 
                        else 
                        {
                            glBufferData(GL_ARRAY_BUFFER, sz, (u8*)(cb->commands.data + cb->commands.position), gsgl_buffer_usage_to_gl_enum(usage));
                        }
                        glBindBuffer(GL_ARRAY_BUFFER, 0);
                    } break;

                    case GS_GRAPHICS_BUFFER_INDEX:
                    {
                        gsgl_buffer_t buffer = gs_slot_array_get(ogl->index_buffers, id);
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
                        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sz, (u8*)(cb->commands.data + cb->commands.position), gsgl_buffer_usage_to_gl_enum(usage));
                        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
                    } break;
                }

                // Advance past data
                gs_byte_buffer_advance_position(&cb->commands, sz);

            } break;

            default:
            {
                // Op code not supported yet!
                gs_println("Op code not supported yet: %zu", (uint32_t)op_code);
                gs_assert(false);
            }
        }
    }
    
    // Clear byte buffer of commands
    gs_byte_buffer_clear(&cb->commands);

    // Set num commands to 0
    cb->num_commands = 0;
}

void gs_graphics_submit_frame()
{
}




























#endif // GS_GRAPHICS_IMPL_OPENGL

#endif // __GS_GRAPHICS_IMPL_H__

