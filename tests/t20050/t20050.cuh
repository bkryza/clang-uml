namespace clanguml {
namespace t20050 {

__device__ float square(float a);

__global__ void vector_square_add(float *out, float *a, float *b, int n);

}
}