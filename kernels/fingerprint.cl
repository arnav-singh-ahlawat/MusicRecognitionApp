__kernel void mag_kernel(__global const float2* frames,
                         __global float* power,
                         const int frameSize) {
    int gid = get_global_id(0);
    float2 val = frames[gid];
    float p = val.x*val.x + val.y*val.y;
    power[gid] = p;
}