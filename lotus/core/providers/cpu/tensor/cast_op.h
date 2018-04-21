﻿#pragma once

#include "core/common/common.h"
#include "core/framework/op_kernel.h"

namespace Lotus {

template <typename T>
class Cast final : public OpKernel {
 public:
  Cast(const OpKernelInfo& info) : OpKernel(info) {
    int64_t to;
    Status status = info.GetAttr("to", &to);
    LOTUS_ENFORCE(status.IsOK(), "Attribute to is not set.");
    to_ = gsl::narrow_cast<TensorProto_DataType>(to);
  }

  Status Compute(OpKernelContext* context) const override;

 private:
  template <typename SrcType,
            typename DstType>
  void CastData(const Tensor* in, Tensor* out, const TensorShape& shape) const {
    for (int64_t i = 0; i < shape.Size(); ++i) {
      out->MutableData<DstType>()[i] = static_cast<DstType>(in->Data<SrcType>()[i]);
    }
  }

  TensorProto_DataType to_;
};
}  //namespace Lotus