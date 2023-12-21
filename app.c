#include "app.h"

#include "debug.h"
#include "helper_macros.h"
#include "system.h"
#include "glfw.h"
#include "gfx.h"
#include "game.h"

#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "app_internal.c"

app_t app__create(int argc, char* argv[]) {
    (void) argc;
    (void) argv;

    app_t result = calloc(1, sizeof(*result));
    memset(result, 0, sizeof(*result));

    result->frame_info_sample_size = 128;
    result->frame_info_sample      = malloc(result->frame_info_sample_size * sizeof(*result->frame_info_sample));

    app__push_stage(result, &loop_stage__collect_previous_frame_info);
    app__push_stage(result, &loop_stage__pre_update);
    app__push_stage(result, &loop_stage__update_loop);
    app__push_stage(result, &loop_stage__render);
    app__push_stage(result, &loop_stage__sleep_till_end_of_frame);

    if (!glfw__init()) {
        return false;
    }

    uint32_t number_of_monitors;
    monitor_t* monitors = monitor__get_monitors(&number_of_monitors);
    if (number_of_monitors == 0) {
        glfw__deinit();
        return false;
    }

    result->monitor = monitors[0];
    int32_t  screen_x;
    int32_t  screen_y;
    uint32_t screen_width;
    uint32_t screen_height;
    monitor__get_work_area(result->monitor, &screen_x, &screen_y, &screen_width, &screen_height);
    const uint32_t window_width = screen_width * 9 / 10;
    const uint32_t window_height = screen_height * 9 / 10;
    // const int32_t window_x = screen_x + (screen_width - window_width) / 2;
    // const int32_t window_y = screen_y + (screen_height - window_height) / 2;
    // window__set_windowed_state_content_area(self->window, window_x, window_y, );

    result->window = window__create(result->monitor, "Title", window_width, window_height);
    if (!result->window) {
        glfw__deinit();
        return false;
    }

    // result->debug_window = window__create(result->monitor, "Debug window", 100, 300);

    window__button_register_action(result->window, BUTTON_FPS_LOCK_INC, (void*) result, app__button_default_action_fps_lock_inc);
    window__button_register_action(result->window, BUTTON_FPS_LOCK_DEC, (void*) result, app__button_default_action_fps_lock_dec);

    result->game = game__create(result->window);
    if (!result->game) {
        glfw__deinit();
        return false;
    }

    return result;
}

void app__destroy(app_t self) {
    game__destroy(self->game);
    window__destroy(self->window);
    glfw__deinit();
}

void app__run(app_t self) {
    // debug__set_message_module_availability(DEBUG_MODULE_APP, false);
    // debug__set_message_module_availability(DEBUG_MODULE_GL, false);
    debug__set_message_module_availability(DEBUG_MODULE_GLFW, false);

    // window__set_current_window(self->debug_window);
    // window__destroy(self->debug_window);

    window__set_current_window(self->window);
    // window__set_window_opacity(self->window, 0.89);

    ///////////////////////////////////////////////////////////////////////
    // const float positions[] = {
    //     -0.5f, -0.5f, 0.0f,
    //     -0.5f, 0.5f, 0.0f,
    //     0.5f, 0.5f, 0.0f,
    //     0.5f, -0.5f, 0.0f
    // };
    // gl_buffer_t position_buffer;
    // gl_buffer__create(
    //     &position_buffer, "positions",
    //     (void*) positions, sizeof(positions),
    //     GL_BUFFER_TYPE_VERTEX,
    //     GL_BUFFER_ACCESS_TYPE_WRITE
    // );

    // const uint32_t indices[] = {
    //     0, 1, 2,
    //     2, 0, 3
    // };

    // gl_buffer_t index_buffer;
    //     gl_buffer__create(
    //     &index_buffer, "indices",
    //     (void*) indices, sizeof(indices),
    //     GL_BUFFER_TYPE_INDEX,
    //     GL_BUFFER_ACCESS_TYPE_WRITE
    // );

    // shader_object_t vertex_shader_object;
    // shader_object__create(
    //     &vertex_shader_object,
    //     SHADER_TYPE_VERTEX,
    //     "#version 460 core\n"
    //     "layout (location = 0) in vec3 pos;\n"
    //     "void main() {\n"
    //     "  gl_Position = vec4(pos.x, pos.y, pos.z, 1.0);\n"
    //     "}\n"
    // );
    // shader_object_t tess_control_shader_object;
    // shader_object__create(
    //     &tess_control_shader_object,
    //     SHADER_TYPE_TESS_CONTROL,
    //     "#version 460 core\n"
    //     "layout (vertices = 3) out;\n"
    //     "void main() {\n"
    //     "  if (gl_InvocationID == 0) {\n"
    //     "    gl_TessLevelInner[0] = 30.0;\n"
    //     "    gl_TessLevelOuter[0] = 5.0;\n"
    //     "    gl_TessLevelOuter[1] = 5.0;\n"
    //     "    gl_TessLevelOuter[2] = 5.0;\n"
    //     "  }\n"
    //     "  gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;\n"
    //     "}\n"
    // );
    // shader_object_t tess_eval_shader_object;
    // shader_object__create(
    //     &tess_eval_shader_object,
    //     SHADER_TYPE_TESS_EVAL,
    //     "#version 460 core\n"
    //     "layout (triangles, equal_spacing, cw) in;\n"
    //     "void main() {\n"
    //     "  gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position + gl_TessCoord.y * gl_in[1].gl_Position + gl_TessCoord.z * gl_in[2].gl_Position);\n"
    //     "}\n"
    // );
    // shader_object_t geometry_shader_object;
    // shader_object__create(
    //     &geometry_shader_object,
    //     SHADER_TYPE_GEOMETRY,
    //     "#version 460 core\n"
    //     "layout (triangles) in;\n"
    //     "layout (points, max_vertices = 3) out;\n"
    //     "void main() {\n"
    //     "  for (int i = 0; i < gl_in.length(); i++) {\n"
    //     "    gl_Position = gl_in[i].gl_Position;\n"
    //     "    EmitVertex();\n"
    //     "  }\n"
    //     "}\n"
    // );
    // shader_object_t fragment_shader_object;
    // shader_object__create(
    //     &fragment_shader_object,
    //     SHADER_TYPE_FRAGMENT,
    //     "#version 460 core\n"
    //     "out vec4 color;\n"
    //     "void main() {\n"
    //     "  color = vec4("
    //     "    sin(gl_FragCoord.x * 0.25) * 0.5 + 0.5,"
    //     "    cos(gl_FragCoord.y * 0.25) * 0.5 + 0.5,"
    //     "    sin(gl_FragCoord.x * 0.15) * cos(gl_FragCoord.y * 0.15),"
    //     "    1.0"
    //     "  );\n"
    //     "}\n"
    // );

    // shader_program_t shader_program;
    // shader_program__create(&shader_program);
    // shader_program__attach(&shader_program, &vertex_shader_object);
    // shader_program__attach(&shader_program, &tess_control_shader_object);
    // shader_program__attach(&shader_program, &tess_eval_shader_object);
    // // shader_program__attach(&shader_program, &geometry_shader_object);
    // shader_program__attach(&shader_program, &fragment_shader_object);
    // if (!shader_program__link(&shader_program)) {
    //     return ;
    // }

    // geometry_object_t geometry_object;
    // geometry_object__create(&geometry_object);
    // geometry_object__attach_vertex(
    //     &geometry_object,
    //     &position_buffer,
    //     vertex_specification(GL_TYPE_R32, GL_CHANNEL_COUNT_3, false),
    //     0,
    //     3 * sizeof(float)
    // );
    // // geometry_object__attach_index_buffer(
    // //     &geometry_object,
    // //     &index_buffer
    // // );

    // // gl__set_polygon_mode(POLYGON_RASTERIZATION_MODE_LINE);

    // gl__set_point_size(5.0f);

    ///////////////////////////////////////////////////////////////////////

    const double target_fps = 10.0;
    debug__write_and_flush(DEBUG_MODULE_APP, DEBUG_INFO, "target fps: %lf", target_fps);

    self->previous_frame_info.time_frame_expected = 1.0 / target_fps;
    self->time_game_update_fixed = game__update_upper_bound(self->game);
    system__init();
    self->previous_frame_info.time_end = -self->previous_frame_info.time_frame_expected;
    bool stage_failed = false;
    ASSERT(self->loop_stages_top > 0);
    while (!stage_failed) {
        loop_stage_t* loop_stage = 0;
        for (uint32_t stage_id = 0; stage_id < self->loop_stages_top; ++stage_id) {
            loop_stage = &self->loop_stages[stage_id];
            const double loop_stage_time_start = system__get_time();
            loop_stage->time_start = loop_stage_time_start;
            if (stage_id > 0) {
                self->loop_stages[stage_id - 1].time_elapsed = loop_stage_time_start - self->loop_stages[stage_id - 1].time_start;
            }
            loop_stage = &self->loop_stages[stage_id];
            if (!loop_stage->loop_stage__execute(loop_stage, self)) {
                stage_failed = true;
                break ;
            }
        }
        loop_stage->time_elapsed = system__get_time() - loop_stage->time_start;
    }
}
