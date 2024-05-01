#include "t20049.cuh"

namespace clanguml {
namespace t20049 {

constexpr unsigned long N{1000};

__device__ float square(float a) { return a * a; }

__global__ void vector_square_add(float *out, float *a, float *b, int n)
{
    for (int i = 0; i < n; i++) {
        out[i] = add(square(a[i]), square(b[i]));
    }
}

int tmain()
{
    float *a, *b, *out;

    a = (float *)malloc(sizeof(float) * N);
    b = (float *)malloc(sizeof(float) * N);
    out = (float *)malloc(sizeof(float) * N);

    for (int i = 0; i < N; i++) {
        a[i] = 1.0f;
        b[i] = 2.0f;
    }

    vector_square_add<<<1, 1>>>(out, a, b, N);

    return 0;
}

}
}