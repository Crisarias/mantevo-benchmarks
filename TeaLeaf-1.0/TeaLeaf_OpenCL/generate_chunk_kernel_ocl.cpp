#include "ocl_common.hpp"

extern "C" void generate_chunk_kernel_ocl_
(const int* number_of_states,

const double* state_density,
const double* state_energy,
const double* state_xmin,
const double* state_xmax,
const double* state_ymin,
const double* state_ymax,
const double* state_radius,
const int* state_geometry,

const int* g_rect,
const int* g_circ,
const int* g_point)
{
    chunk.generate_chunk_kernel(
        * number_of_states, state_density, state_energy,
        state_xmin, state_xmax, state_ymin, state_ymax,
        state_radius, state_geometry, * g_rect, * g_circ, *g_point);
}

void CloverChunk::generate_chunk_kernel
(const int number_of_states, 
const double* state_density, const double* state_energy,
const double* state_xmin, const double* state_xmax,
const double* state_ymin, const double* state_ymax,
const double* state_radius, const int* state_geometry,
const int g_rect, const int g_circ, const int g_point)
{
    #define TEMP_ALLOC(arr)                                         \
        cl::Buffer tmp_state_##arr;                                 \
        try                                                         \
        {                                                           \
            tmp_state_##arr = cl::Buffer(context,                   \
                CL_MEM_READ_ONLY,                                   \
                number_of_states*sizeof(*state_##arr));             \
            queue.enqueueWriteBuffer(tmp_state_##arr,               \
                CL_TRUE,                                            \
                0,                                                  \
                number_of_states*sizeof(*state_##arr),              \
                state_##arr);                                       \
        }                                                           \
        catch (cl::Error e)                                         \
        {                                                           \
            DIE("Error in creating %s buffer %d\n", #arr, e.err()); \
        }

    TEMP_ALLOC(density);
    TEMP_ALLOC(energy);
    TEMP_ALLOC(xmin);
    TEMP_ALLOC(xmax);
    TEMP_ALLOC(ymin);
    TEMP_ALLOC(ymax);
    TEMP_ALLOC(radius);
    TEMP_ALLOC(geometry);

    #undef TEMP_ALLOC

    generate_chunk_init_device.setArg(2, tmp_state_density);
    generate_chunk_init_device.setArg(3, tmp_state_energy);

    //ENQUEUE(generate_chunk_init_device);
    ENQUEUE_OFFSET(generate_chunk_init_device);

    generate_chunk_device.setArg(7, tmp_state_density);
    generate_chunk_device.setArg(8, tmp_state_energy);
    generate_chunk_device.setArg(9, tmp_state_xmin);
    generate_chunk_device.setArg(10, tmp_state_xmax);
    generate_chunk_device.setArg(11, tmp_state_ymin);
    generate_chunk_device.setArg(12, tmp_state_ymax);
    generate_chunk_device.setArg(13, tmp_state_radius);
    generate_chunk_device.setArg(14, tmp_state_geometry);

    generate_chunk_device.setArg(15, g_rect);
    generate_chunk_device.setArg(16, g_circ);
    generate_chunk_device.setArg(17, g_point);

    for (int state = 1; state < number_of_states; state++)
    {
        generate_chunk_device.setArg(18, state);

        //ENQUEUE(generate_chunk_device);
        ENQUEUE_OFFSET(generate_chunk_device);
    }
}

