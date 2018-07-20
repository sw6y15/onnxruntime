#include "activations.h"

namespace Lotus {
namespace Cuda {

#define REGISTER_ACTIVATION_KERNEL_ALIAS(alias, x, ver, T)                                         \
  ONNX_OPERATOR_TYPED_KERNEL_EX(                                                                   \
    alias,                                                                                         \
    kOnnxDomain,                                                                                   \
    ver,                                                                                           \
    T,                                                                                             \
    kCudaExecutionProvider,                                                                        \
    KernelDefBuilder().TypeConstraint("T", DataTypeImpl::GetTensorType<T>()).MayInplace(0, 0),     \
    x<T>);

#define REGISTER_ACTIVATION_KERNEL(x, ver, T) \
  REGISTER_ACTIVATION_KERNEL_ALIAS(x, x, ver, T)

#define UNARY_ACTIVATION_COMPUTE(x, T)                                                            \
  template <>                                                                                     \
  Status x<T>::Compute(OpKernelContext* context) const {                                          \
    UnaryElementwisePreparation p;                                                                \
    UnaryElementwise::Prepare(context, &p);                                                       \
    CudaAsyncBuffer<Ctx##x> func_ctx(provider_, MakeFuncCtx());                                   \
    if (!std::is_same<CtxNull, Ctx##x>::value)                                                    \
      LOTUS_RETURN_IF_ERROR(func_ctx.CopyToGpu());                                                \
    Impl_##x<typename ToCudaType<T>::MappedType>(                                                 \
        reinterpret_cast<const typename ToCudaType<T>::MappedType*>(p.input_tensor->Data<T>()),   \
        reinterpret_cast<typename ToCudaType<T>::MappedType*>(p.output_tensor->MutableData<T>()), \
        func_ctx.GpuPtr(),                                                                        \
        p.output_tensor->Shape().Size());                                                         \
                                                                                                  \
    return Status::OK();                                                                          \
  }

#define UNARY_ACTIVATION_OP_TYPED(name, ver, T) \
  REGISTER_ACTIVATION_KERNEL(name, ver, T)      \
  UNARY_ACTIVATION_COMPUTE(name, T)

#define UNARY_ACTIVATION_OP_HFD(name, ver)        \
  UNARY_ACTIVATION_OP_TYPED(name, ver, MLFloat16) \
  UNARY_ACTIVATION_OP_TYPED(name, ver, float)     \
  UNARY_ACTIVATION_OP_TYPED(name, ver, double)

UNARY_ACTIVATION_OP_HFD(Elu, 6)
UNARY_ACTIVATION_OP_HFD(HardSigmoid, 6);
UNARY_ACTIVATION_OP_HFD(LeakyRelu, 6);
UNARY_ACTIVATION_OP_HFD(ParametricSoftplus, 1);
UNARY_ACTIVATION_OP_HFD(Relu, 6);
UNARY_ACTIVATION_OP_HFD(ScaledTanh, 1);
UNARY_ACTIVATION_OP_HFD(Selu, 6);
UNARY_ACTIVATION_OP_HFD(Sigmoid, 6);
UNARY_ACTIVATION_OP_HFD(Softsign, 1);
UNARY_ACTIVATION_OP_HFD(Tanh, 6);
UNARY_ACTIVATION_OP_HFD(ThresholdedRelu, 1);

// SoftPlus is the default case for ParametricSoftPlus
REGISTER_ACTIVATION_KERNEL_ALIAS(Softplus, ParametricSoftplus, 1, MLFloat16)
REGISTER_ACTIVATION_KERNEL_ALIAS(Softplus, ParametricSoftplus, 1, float)
REGISTER_ACTIVATION_KERNEL_ALIAS(Softplus, ParametricSoftplus, 1, double)

//REGISTER_ACTIVATION_KERNEL(PRelu, 7);

}  // namespace Cuda
}  // namespace Lotus