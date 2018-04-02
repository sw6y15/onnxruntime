#pragma once

#include "core/common/common.h"
#include "core/framework/op_kernel.h"
#include "core/util/math_cpuonly.h"

namespace Lotus {

template <typename T>
struct Constant final : OpKernel {
  Constant(const OpKernelInfo& info) : OpKernel(info) {
    if (!info.GetAttr("value", &value_).IsOK()) {
      LOTUS_ENFORCE(false, "Must have valid 'value' attribute");
    }
  }

  Status Compute(OpKernelContext* context) const override;

 private:
  TensorProto value_;
};

}  // namespace Lotus