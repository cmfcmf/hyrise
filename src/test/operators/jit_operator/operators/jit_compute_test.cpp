#include "operators/jit_operator/operators/jit_compute.hpp"
#include "../../../base_test.hpp"

namespace opossum {

// Mock JitOperator that passes individual tuples into the chain and ignores consumed tuples
// This operator is used as both the tuple source and sink in this test
class MockOperator : public AbstractJittable {
 public:
  std::string description() const final { return "MockOperator"; }

  void emit(JitRuntimeContext& context) { _emit(context); }

 private:
  void _consume(JitRuntimeContext& context) const final {}
};

class JitComputeTest : public BaseTest {};

TEST_F(JitComputeTest, TriggersComputationOfNestedExpression) {
  JitRuntimeContext context;
  context.tuple.resize(5);

  // This test computes the expression "D = A + B > C"

  // Create tuple values for inputs
  JitTupleValue a_value{DataType::Int, false, 0};
  JitTupleValue b_value{DataType::Int, false, 1};
  JitTupleValue c_value{DataType::Int, false, 2};

  // Construct expression tree
  auto a_expression = std::make_shared<JitExpression>(a_value);
  auto b_expression = std::make_shared<JitExpression>(b_value);
  auto c_expression = std::make_shared<JitExpression>(c_value);
  auto a_plus_b = std::make_shared<JitExpression>(a_expression, ExpressionType::Addition, b_expression, 3);
  auto expression = std::make_shared<JitExpression>(a_plus_b, ExpressionType::GreaterThan, c_expression, 4);

  // Construct operator chain
  auto mock_op = std::make_shared<MockOperator>();
  auto compute = std::make_shared<JitCompute>(expression);
  mock_op->set_next_operator(compute);
  compute->set_next_operator(mock_op);

  // We test the correct computation of the expression ten times with random values
  for (auto i = 0; i < 10; ++i) {
    // Create input tuple values
    auto a = static_cast<int32_t>(std::rand());
    auto b = static_cast<int32_t>(std::rand());
    auto c = static_cast<int32_t>(std::rand());

    // Set input values in tuple
    a_value.set<int32_t>(a, context);
    b_value.set<int32_t>(b, context);
    c_value.set<int32_t>(c, context);

    mock_op->emit(context);
    ASSERT_EQ(a + b > c, context.tuple.get<bool>(4));
  }
}

}  // namespace opossum
