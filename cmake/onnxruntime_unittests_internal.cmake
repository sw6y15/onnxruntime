# Copyright (c) Microsoft Corporation. All rights reserved.
# Licensed under the MIT License.

# unittests for all internal providers
# this file is meant to be included inside the parent onnxruntime_unittests.cmake file
# and hence it depends on variables defined in the parent file.

# nuphar
if(lotus_USE_TVM)
  list(APPEND lotus_test_framework_src_patterns  ${TEST_SRC_DIR}/framework/nuphar/*)
  list(APPEND lotus_test_framework_libs onnxruntime_providers_nuphar)
  list(APPEND lotus_test_providers_dependencies onnxruntime_providers_nuphar)
  list(APPEND lotus_test_providers_libs onnxruntime_providers_nuphar)
endif()
