namespace clanguml {
namespace t20049 {

template <typename T> __device__ T add(T a, T b) { return a + b; }

__device__ float square(float a);

__global__ void vector_square_add(float *out, float *a, float *b, int n);

}
}