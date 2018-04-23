#include "gtest/gtest.h"
#include "test/providers/provider_test_utils.h"

namespace Lotus {
namespace Test {


template <typename T> 
void TestIntCategory(std::vector<T> &input)
{
  OpTester test("OneHotEncoder", LotusIR::kMLDomain);
  std::vector<int64_t> categories{ 0, 1, 2, 3, 4, 5, 6, 7 };
  test.AddAttribute("cats_int64s", categories);
  
  vector<float> expected_output;
  for (size_t i = 0; i < input.size(); ++i)
    for (size_t j = 0; j < categories.size(); ++j)
      if (static_cast<int64_t>(input[i]) != categories[j]) expected_output.push_back(0.0);
      else expected_output.push_back(1.0);

  test.AddInput<T>("X", { 1, 7 }, input);
  test.AddOutput<float>("Y", { 1, 7, 8 }, expected_output);
  test.Run();

  test.AddAttribute("zeros", 0LL);
  test.Run(true);
}
  
TEST(OneHotEncoderOpTest, IntegerWithInt64) {
  vector<int64_t> input{ 8, 1, 0, 0, 3, 7, 4  };
  TestIntCategory<int64_t>(input);
}

/*
TEST(OneHotEncoderOpTest, IntegerWithInt32) {
  vector<int> input{ 8, 1, 0, 0, 3, 7, 4 };
  TestIntCategory<int>(input);
}

TEST(OneHotEncoderOpTest, IntegerWithDouble) {
  vector<double> input{ 8.1f, 1.2f, 0.0f, 0.7f, 3.4f, 7.9f, 4.4f };
  TestIntCategory<double>(input);
}


TEST(OneHotEncoderOpTest, String) {
  OpTester test("OneHotEncoder", LotusIR::kMLDomain);

  std::vector<std::string> categories{ "Apple", "Orange", "Watermelon", "Blueberry", "Coconut", "Mango", "Tangerine" };
  test.AddAttribute("cats_strings", categories);
  test.AddAttribute("zeros", 1LL);

  vector<std::string> input{ "Watermelon", "Orange", "Tangerine", "Apple", "Kit" };
  vector<float> expected_output;

  for (size_t i = 0; i < input.size(); ++i)
    for (size_t j = 0; j < categories.size(); ++j)
      if (input[i] != categories[j]) expected_output.push_back(0.0);
      else expected_output.push_back(1.0);

  test.AddInput<string>("X", { 1, 5 }, input);
  test.AddOutput<float>("Y", { 1, 5, 7 }, expected_output);
  test.Run();

  test.AddAttribute("zeros", 0LL);
  test.Run(true);
}
*/

}  // namespace Test
}  // namespace Lotus
