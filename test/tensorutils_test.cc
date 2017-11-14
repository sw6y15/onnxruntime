#include "gtest/gtest.h"
#include "core/graph/tensorutils.h"
#include "external/onnx/onnx/onnx-ml.pb.h"

using namespace Lotus::Utils;
using namespace onnx;

namespace Lotus
{
    namespace Test
    {
        TEST(TensorParseTest, TensorUtilsTest)
        {
            TensorProto boolTensorProto;
            boolTensorProto.set_data_type(TensorProto_DataType_BOOL);
            boolTensorProto.add_int32_data(1);

            std::vector<bool> boolData;
            auto status = TensorUtils::UnpackTensor(boolTensorProto, &boolData);
            EXPECT_TRUE(status.Ok());
            EXPECT_EQ(1, boolData.size());
            EXPECT_TRUE(boolData[0]);

            std::vector<float> floatData;
            status = TensorUtils::UnpackTensor(boolTensorProto, &floatData);
            EXPECT_FALSE(status.Ok());

            TensorProto floatTensorProto;
            floatTensorProto.set_data_type(TensorProto_DataType_FLOAT);
            float f[4] = { 1.1f, 2.2f, 3.3f, 4.4f };
            char rawdata[sizeof(float) * 4 + 1];
            for (int i = 0; i < 4; ++i)
            {
                memcpy(rawdata + i * sizeof(float), &(f[i]), sizeof(float));
            }

            rawdata[sizeof(float) * 4] = '\0';
            floatTensorProto.set_raw_data(rawdata);
            TensorUtils::UnpackTensor(floatTensorProto, &floatData);
            EXPECT_EQ(4, floatData.size());
            EXPECT_EQ(1.1f, floatData[0]);
            EXPECT_EQ(2.2f, floatData[1]);
            EXPECT_EQ(3.3f, floatData[2]);
            EXPECT_EQ(4.4f, floatData[3]);

            TensorProto stringTensorProto;
            stringTensorProto.set_data_type(TensorProto_DataType_STRING);
            stringTensorProto.add_string_data("a");
            stringTensorProto.add_string_data("b");

            std::vector<std::string> stringData;
            status = TensorUtils::UnpackTensor(stringTensorProto, &stringData);
            EXPECT_TRUE(status.Ok());
            EXPECT_EQ(2, stringData.size());
            EXPECT_EQ("a", stringData[0]);
            EXPECT_EQ("b", stringData[1]);

            status = TensorUtils::UnpackTensor(boolTensorProto, &stringData);
            EXPECT_FALSE(status.Ok());
        }
    }
}