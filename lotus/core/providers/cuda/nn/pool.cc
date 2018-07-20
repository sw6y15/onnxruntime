
#include "core/providers/common.h"
#include "core/providers/cuda/cudnn_common.h"
#include "core/providers/cuda/nn/pool.h"
using namespace Lotus::Common;
namespace Lotus {
namespace Cuda {

#define POOLING_KERNEL(op_name, data_type, pool_type, since_version)                  \
  ONNX_OPERATOR_TYPED_KERNEL_EX(                                                      \
    op_name,                                                                          \
    kOnnxDomain,                                                                      \
    since_version,                                                                    \
    data_type,                                                                        \
    kCudaExecutionProvider,                                                           \
    KernelDefBuilder().TypeConstraint("T", DataTypeImpl::GetTensorType<data_type>()), \
    Pool<data_type, pool_type>);

POOLING_KERNEL(AveragePool, float, AveragePool, 7)
POOLING_KERNEL(AveragePool, double, AveragePool, 7)
POOLING_KERNEL(GlobalAveragePool, float, AveragePool, 1)
POOLING_KERNEL(GlobalAveragePool, double, AveragePool, 1)
POOLING_KERNEL(MaxPool, float, MaxPool, 1)
POOLING_KERNEL(MaxPool, double, MaxPool, 1)
POOLING_KERNEL(GlobalMaxPool, float, MaxPool, 1)
POOLING_KERNEL(GlobalMaxPool, double, MaxPool, 1)

class CudnnPoolingDescriptor final {
 public:
  CudnnPoolingDescriptor() : desc_(nullptr) {
  }

  ~CudnnPoolingDescriptor() {
    if (desc_ != nullptr) {
      cudnnDestroyPoolingDescriptor(desc_);
      desc_ = nullptr;
    }
  }

  Status Set(cudnnPoolingMode_t mode,
             const std::vector<int64_t>& kernel_shape,
             const std::vector<int64_t>& pads,
             const std::vector<int64_t>& strides) {
    if (!desc_)
      CUDNN_RETURN_IF_ERROR(cudnnCreatePoolingDescriptor(&desc_));

    int rank = gsl::narrow_cast<int>(kernel_shape.size());
    std::vector<int> window(rank);
    std::vector<int> padding(rank);
    std::vector<int> stride(rank);
    for (int i = 0; i < rank; i++) {
      window[i] = gsl::narrow_cast<int>(kernel_shape[i]);
    }
    for (int i = 0; i < rank; i++) {
      padding[i] = gsl::narrow_cast<int>(pads[i]);
    }
    for (int i = 0; i < rank; i++) {
      stride[i] = gsl::narrow_cast<int>(strides[i]);
    }
    CUDNN_RETURN_IF_ERROR(cudnnSetPoolingNdDescriptor(
        desc_,
        mode,
        CUDNN_PROPAGATE_NAN,
        rank,
        window.data(),
        padding.data(),
        stride.data()));

    return Status::OK();
  }

  operator cudnnPoolingDescriptor_t() const { return desc_; }

 private:
  cudnnPoolingDescriptor_t desc_;
};

template <typename T, PoolType type>
Status Pool<T, type>::Compute(OpKernelContext* context) const {
  typedef typename ToCudaType<T>::MappedType CudaT;
  const Tensor* X = context->Input<Tensor>(0);
  const TensorShape& x_shape = X->Shape();
  const auto& x_dims = x_shape.GetDims();

  if (x_shape.NumDimensions() < 3) {
    return LOTUS_MAKE_STATUS(LOTUS, FAIL, "Input dimension cannot be less than 3.");
  }

  std::vector<int64_t> kernel_shape = kernel_shape_;
  std::vector<int64_t> pads = pads_;
  std::vector<int64_t> strides = strides_;

  if (global_pooling_) {
    kernel_shape.assign(x_dims.begin() + 2, x_dims.end());
    pads.assign(kernel_shape.size(), 0);
    strides.assign(kernel_shape.size(), 1);
  }

  std::vector<int64_t> y_dims = PoolBase::SetOutputSize(x_shape, x_shape[1], &pads);
  Tensor* Y = context->Output(0, TensorShape(y_dims));

  auto x_data = reinterpret_cast<const CudaT*>(X->Data<T>());
  auto y_data = reinterpret_cast<CudaT*>(Y->MutableData<T>());

  const auto alpha = Consts<CudaT>::One;
  const auto beta = Consts<CudaT>::Zero;
  CudnnTensor x_tensor;
  CudnnTensor y_tensor;
  LOTUS_RETURN_IF_ERROR(x_tensor.Set(x_dims, CudnnTensor::GetDataType<CudaT>()));
  LOTUS_RETURN_IF_ERROR(y_tensor.Set(y_dims, CudnnTensor::GetDataType<CudaT>()));

  cudnnPoolingMode_t mode = CUDNN_POOLING_MAX;
  if (type == Lotus::Cuda::PoolType::AveragePool) {
    mode = count_include_pad_ ? CUDNN_POOLING_AVERAGE_COUNT_INCLUDE_PADDING : CUDNN_POOLING_AVERAGE_COUNT_EXCLUDE_PADDING;
  }
  CudnnPoolingDescriptor pooling_desc;
  LOTUS_RETURN_IF_ERROR(pooling_desc.Set(mode, kernel_shape, pads, strides));

  CUDNN_RETURN_IF_ERROR(cudnnPoolingForward(CudnnHandle(), pooling_desc, &alpha, x_tensor, x_data, &beta, y_tensor, y_data));

  return Status::OK();
}

}  // namespace Cuda
}  // namespace Lotus