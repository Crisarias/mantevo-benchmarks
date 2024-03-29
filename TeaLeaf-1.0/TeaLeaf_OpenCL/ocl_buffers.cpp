#include "ocl_common.hpp"

void CloverChunk::initBuffers
(void)
{
    const std::vector<double> zeros(total_cells, 0.0);

    #define BUF_ALLOC(name, buf_sz)                 \
        try                                         \
        {                                           \
            name = cl::Buffer(context,              \
                              CL_MEM_READ_WRITE,    \
                              (buf_sz));            \
            queue.enqueueWriteBuffer(name,          \
                                     CL_TRUE,       \
                                     0,             \
                                     (buf_sz),      \
                                     &zeros[0]);    \
        }                                           \
        catch (cl::Error e)                         \
        {                                           \
            DIE("Error in creating %s buffer %d\n", \
                    #name, e.err());                \
        }

    #define BUF1DX_ALLOC(name, x_e)     \
        BUF_ALLOC(name, (x_max+4+x_e) * sizeof(double))

    #define BUF1DY_ALLOC(name, y_e)     \
        BUF_ALLOC(name, (y_max+4+y_e) * sizeof(double))

    #define BUF2D_ALLOC(name, x_e, y_e) \
        BUF_ALLOC(name, (x_max+4+x_e) * (y_max+4+y_e) * sizeof(double))

    BUF2D_ALLOC(density, 0, 0);
    BUF2D_ALLOC(energy0, 0, 0);
    BUF2D_ALLOC(energy1, 0, 0);

    BUF2D_ALLOC(volume, 0, 0);
    BUF2D_ALLOC(xarea, 1, 0);
    BUF2D_ALLOC(yarea, 0, 1);

    BUF1DX_ALLOC(cellx, 0);
    BUF1DX_ALLOC(celldx, 0);
    BUF1DX_ALLOC(vertexx, 1);
    BUF1DX_ALLOC(vertexdx, 1);

    BUF1DY_ALLOC(celly, 0);
    BUF1DY_ALLOC(celldy, 0);
    BUF1DY_ALLOC(vertexy, 1);
    BUF1DY_ALLOC(vertexdy, 1);

    // work arrays used in various kernels (post_vol, pre_vol, mom_flux, etc)
    BUF2D_ALLOC(work_array_1, 1, 1);
    BUF2D_ALLOC(work_array_2, 1, 1);
    BUF2D_ALLOC(work_array_3, 1, 1);
    BUF2D_ALLOC(work_array_4, 1, 1);
    BUF2D_ALLOC(work_array_5, 1, 1);
    BUF2D_ALLOC(work_array_6, 1, 1);
    BUF2D_ALLOC(work_array_8, 1, 1);

    // tealeaf
    BUF2D_ALLOC(u, 0, 0);
    BUF2D_ALLOC(u0, 0, 0);
    BUF2D_ALLOC(z, 1, 1);

    // allocate enough for 1 item per work group, and then a bit extra for the reduction
    // 1.5 should work even if wg size is 2
    BUF_ALLOC(reduce_buf_1, 1.5*((sizeof(double)*reduced_cells)/(LOCAL_X*LOCAL_Y)));
    BUF_ALLOC(reduce_buf_2, 1.5*((sizeof(double)*reduced_cells)/(LOCAL_X*LOCAL_Y)));
    BUF_ALLOC(reduce_buf_3, 1.5*((sizeof(double)*reduced_cells)/(LOCAL_X*LOCAL_Y)));
    BUF_ALLOC(reduce_buf_4, 1.5*((sizeof(double)*reduced_cells)/(LOCAL_X*LOCAL_Y)));
    BUF_ALLOC(reduce_buf_5, 1.5*((sizeof(double)*reduced_cells)/(LOCAL_X*LOCAL_Y)));
    BUF_ALLOC(reduce_buf_6, 1.5*((sizeof(double)*reduced_cells)/(LOCAL_X*LOCAL_Y)));
    BUF_ALLOC(PdV_reduce_buf, 1.5*((sizeof(int)*reduced_cells)/(LOCAL_X*LOCAL_Y)));

    // set initial (ideal) size for buffers and increment untl it hits alignment
    lr_mpi_buf_sz = sizeof(double)*(y_max + 5);
    bt_mpi_buf_sz = sizeof(double)*(x_max + 5);

    // enough for 1 for each array - overkill, but not that much extra space
    BUF_ALLOC(left_buffer, NUM_BUFFERED_FIELDS*2*lr_mpi_buf_sz);
    BUF_ALLOC(right_buffer, NUM_BUFFERED_FIELDS*2*lr_mpi_buf_sz);
    BUF_ALLOC(bottom_buffer, NUM_BUFFERED_FIELDS*2*bt_mpi_buf_sz);
    BUF_ALLOC(top_buffer, NUM_BUFFERED_FIELDS*2*bt_mpi_buf_sz);

    fprintf(DBGOUT, "Buffers allocated\n");

    #undef BUF2D_ALLOC
    #undef BUF1DX_ALLOC
    #undef BUF1DY_ALLOC
    #undef BUF_ALLOC

 #define ADD_BUFFER_DBG_MAP(name) arr_names[#name] = name;
    ADD_BUFFER_DBG_MAP(u);
    arr_names["p"] = work_array_1;

    ADD_BUFFER_DBG_MAP(work_array_2);
    ADD_BUFFER_DBG_MAP(work_array_3);
    ADD_BUFFER_DBG_MAP(work_array_4);
    ADD_BUFFER_DBG_MAP(work_array_5);
    ADD_BUFFER_DBG_MAP(work_array_6);

    ADD_BUFFER_DBG_MAP(density);
    ADD_BUFFER_DBG_MAP(energy0);
    ADD_BUFFER_DBG_MAP(energy1);

    ADD_BUFFER_DBG_MAP(cellx);
    ADD_BUFFER_DBG_MAP(celly);
    ADD_BUFFER_DBG_MAP(celldx);
    ADD_BUFFER_DBG_MAP(celldy);
    ADD_BUFFER_DBG_MAP(vertexx);
    ADD_BUFFER_DBG_MAP(vertexy);
    ADD_BUFFER_DBG_MAP(vertexdx);
    ADD_BUFFER_DBG_MAP(vertexdy);
#undef ADD_BUFFER_DBG_MAP
}

