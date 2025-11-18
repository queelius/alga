/**
 * @file numeric_parsers_test.cpp
 * @brief Comprehensive tests for numeric parser types
 *
 * Tests all four numeric types: unsigned_int, signed_int, floating_point, scientific_notation
 * Validates algebraic laws, value semantics, and edge cases.
 */

#include <gtest/gtest.h>
#include "parsers/numeric_parsers.hpp"
#include <vector>
#include <limits>

using namespace alga;

// ============================================================================
// unsigned_int Tests
// ============================================================================

class UnsignedIntTest : public ::testing::Test {
protected:
    void SetUp() override {
        valid_input = "12345";
        zero_input = "0";
        large_input = "18446744073709551615";  // max uint64_t
        invalid_input = "-123";
        invalid_chars = "123abc";
        empty_input = "";
    }

    std::string valid_input;
    std::string zero_input;
    std::string large_input;
    std::string invalid_input;
    std::string invalid_chars;
    std::string empty_input;
};

TEST_F(UnsignedIntTest, FactoryValidInput) {
    auto result = make_unsigned_int(valid_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), 12345ULL);
}

TEST_F(UnsignedIntTest, FactoryZeroInput) {
    auto result = make_unsigned_int(zero_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), 0ULL);
}

TEST_F(UnsignedIntTest, FactoryLargeInput) {
    auto result = make_unsigned_int(large_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), std::numeric_limits<uint64_t>::max());
}

TEST_F(UnsignedIntTest, FactoryInvalidNegative) {
    auto result = make_unsigned_int(invalid_input);
    EXPECT_FALSE(result.has_value());
}

TEST_F(UnsignedIntTest, FactoryInvalidChars) {
    auto result = make_unsigned_int(invalid_chars);
    EXPECT_FALSE(result.has_value());
}

TEST_F(UnsignedIntTest, FactoryEmptyInput) {
    auto result = make_unsigned_int(empty_input);
    EXPECT_FALSE(result.has_value());
}

TEST_F(UnsignedIntTest, FactoryFromValue) {
    auto result = make_unsigned_int(42ULL);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), 42ULL);
}

TEST_F(UnsignedIntTest, DefaultConstructor) {
    unsigned_int ui;
    EXPECT_EQ(ui.val(), 0ULL);
    EXPECT_TRUE(ui.empty());
}

TEST_F(UnsignedIntTest, ValueSemantics) {
    auto original = *make_unsigned_int("100");

    // Copy constructor
    unsigned_int copy(original);
    EXPECT_EQ(copy.val(), 100ULL);

    // Move constructor
    unsigned_int moved(std::move(copy));
    EXPECT_EQ(moved.val(), 100ULL);

    // Copy assignment
    unsigned_int assigned;
    assigned = original;
    EXPECT_EQ(assigned.val(), 100ULL);

    // Move assignment
    unsigned_int move_assigned;
    move_assigned = std::move(assigned);
    EXPECT_EQ(move_assigned.val(), 100ULL);
}

TEST_F(UnsignedIntTest, MonoidIdentity) {
    auto zero = unsigned_int{};
    auto value = *make_unsigned_int("42");

    // Left identity: 0 + x = x
    EXPECT_EQ((zero * value).val(), value.val());

    // Right identity: x + 0 = x
    EXPECT_EQ((value * zero).val(), value.val());
}

TEST_F(UnsignedIntTest, MonoidAssociativity) {
    auto a = *make_unsigned_int("10");
    auto b = *make_unsigned_int("20");
    auto c = *make_unsigned_int("30");

    // (a + b) + c = a + (b + c)
    EXPECT_EQ((a * b) * c, a * (b * c));
}

TEST_F(UnsignedIntTest, MonoidComposition) {
    auto a = *make_unsigned_int("100");
    auto b = *make_unsigned_int("200");

    auto result = a * b;
    EXPECT_EQ(result.val(), 300ULL);
}

TEST_F(UnsignedIntTest, MonoidOptionalComposition) {
    auto a = make_unsigned_int("100");
    auto b = make_unsigned_int("200");
    auto invalid = make_unsigned_int("invalid");

    auto result = a * b;
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), 300ULL);

    // One invalid operand
    auto result2 = a * invalid;
    EXPECT_FALSE(result2.has_value());

    auto result3 = invalid * b;
    EXPECT_FALSE(result3.has_value());
}

TEST_F(UnsignedIntTest, AdditionOperator) {
    auto a = *make_unsigned_int("50");
    auto b = *make_unsigned_int("75");

    auto result = a + b;
    EXPECT_EQ(result.val(), 125ULL);
}

TEST_F(UnsignedIntTest, ComparisonOperators) {
    auto small = *make_unsigned_int("10");
    auto large = *make_unsigned_int("20");
    auto equal = *make_unsigned_int("10");

    EXPECT_TRUE(small == equal);
    EXPECT_FALSE(small == large);
    EXPECT_TRUE(small != large);
    EXPECT_TRUE(small < large);
    EXPECT_TRUE(small <= large);
    EXPECT_TRUE(small <= equal);
    EXPECT_TRUE(large > small);
    EXPECT_TRUE(large >= small);
    EXPECT_TRUE(equal >= small);
}

TEST_F(UnsignedIntTest, ChoiceOperator) {
    auto zero = unsigned_int{};
    auto value = *make_unsigned_int("42");

    // Empty (zero) should choose non-empty
    EXPECT_EQ((zero | value).val(), 42ULL);
    EXPECT_EQ((value | zero).val(), 42ULL);

    // Both non-empty: left wins
    auto left = *make_unsigned_int("10");
    auto right = *make_unsigned_int("20");
    EXPECT_EQ((left | right).val(), 10ULL);
}

TEST_F(UnsignedIntTest, ChoiceOptionalOperator) {
    auto valid = make_unsigned_int("42");
    auto invalid = make_unsigned_int("invalid");

    EXPECT_EQ((invalid | valid)->val(), 42ULL);
    EXPECT_EQ((valid | invalid)->val(), 42ULL);
}

TEST_F(UnsignedIntTest, RepetitionOperator) {
    auto base = *make_unsigned_int("5");

    auto times_0 = base ^ 0;
    EXPECT_EQ(times_0.val(), 0ULL);

    auto times_1 = base ^ 1;
    EXPECT_EQ(times_1.val(), 5ULL);

    auto times_3 = base ^ 3;
    EXPECT_EQ(times_3.val(), 15ULL);

    auto times_10 = base ^ 10;
    EXPECT_EQ(times_10.val(), 50ULL);
}

TEST_F(UnsignedIntTest, FunctionApplication) {
    auto value = *make_unsigned_int("42");

    // Apply function that doubles the value
    auto doubled = value % [](unsigned_int ui) { return ui.val() * 2; };
    EXPECT_EQ(doubled, 84ULL);

    // Apply function to optional
    auto opt_value = make_unsigned_int("100");
    auto opt_result = opt_value % [](unsigned_int ui) { return ui.val() / 2; };
    ASSERT_TRUE(opt_result.has_value());
    EXPECT_EQ(*opt_result, 50ULL);

    // Apply to nullopt
    auto invalid = make_unsigned_int("invalid");
    auto invalid_result = invalid % [](unsigned_int ui) { return ui.val(); };
    EXPECT_FALSE(invalid_result.has_value());
}

TEST_F(UnsignedIntTest, OverflowHandling) {
    auto max = *make_unsigned_int(std::numeric_limits<uint64_t>::max());
    auto one = *make_unsigned_int(1ULL);

    // Should saturate at max
    auto result = max * one;
    EXPECT_EQ(result.val(), std::numeric_limits<uint64_t>::max());
}

TEST_F(UnsignedIntTest, StringConversion) {
    auto value = *make_unsigned_int("12345");
    EXPECT_EQ(value.str(), "12345");
    EXPECT_EQ(static_cast<std::string>(value), "12345");
}

TEST_F(UnsignedIntTest, StreamOutput) {
    auto value = *make_unsigned_int("999");
    std::ostringstream oss;
    oss << value;
    EXPECT_EQ(oss.str(), "999");
}

TEST_F(UnsignedIntTest, ContainerStorage) {
    std::vector<unsigned_int> numbers;
    numbers.push_back(*make_unsigned_int("1"));
    numbers.push_back(*make_unsigned_int("2"));
    numbers.push_back(*make_unsigned_int("3"));

    EXPECT_EQ(numbers.size(), 3UL);
    EXPECT_EQ(numbers[0].val(), 1ULL);
    EXPECT_EQ(numbers[1].val(), 2ULL);
    EXPECT_EQ(numbers[2].val(), 3ULL);
}

// ============================================================================
// signed_int Tests
// ============================================================================

class SignedIntTest : public ::testing::Test {
protected:
    void SetUp() override {
        positive_input = "12345";
        negative_input = "-12345";
        zero_input = "0";
        plus_prefix = "+100";
    }

    std::string positive_input;
    std::string negative_input;
    std::string zero_input;
    std::string plus_prefix;
};

TEST_F(SignedIntTest, FactoryPositive) {
    auto result = make_signed_int(positive_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), 12345LL);
}

TEST_F(SignedIntTest, FactoryNegative) {
    auto result = make_signed_int(negative_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), -12345LL);
}

TEST_F(SignedIntTest, FactoryZero) {
    auto result = make_signed_int(zero_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), 0LL);
}

TEST_F(SignedIntTest, FactoryPlusPrefix) {
    auto result = make_signed_int(plus_prefix);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->val(), 100LL);
}

TEST_F(SignedIntTest, FactoryInvalidJustSign) {
    auto result = make_signed_int("-");
    EXPECT_FALSE(result.has_value());
}

TEST_F(SignedIntTest, MonoidIdentity) {
    auto zero = signed_int{};
    auto value = *make_signed_int("42");

    EXPECT_EQ((zero * value).val(), value.val());
    EXPECT_EQ((value * zero).val(), value.val());
}

TEST_F(SignedIntTest, MonoidAssociativity) {
    auto a = *make_signed_int("10");
    auto b = *make_signed_int("-20");
    auto c = *make_signed_int("30");

    EXPECT_EQ((a * b) * c, a * (b * c));
}

TEST_F(SignedIntTest, Addition) {
    auto a = *make_signed_int("50");
    auto b = *make_signed_int("-30");

    auto result = a + b;
    EXPECT_EQ(result.val(), 20LL);
}

TEST_F(SignedIntTest, Subtraction) {
    auto a = *make_signed_int("100");
    auto b = *make_signed_int("30");

    auto result = a - b;
    EXPECT_EQ(result.val(), 70LL);
}

TEST_F(SignedIntTest, ComparisonOperators) {
    auto negative = *make_signed_int("-10");
    auto zero = signed_int{};
    auto positive = *make_signed_int("10");

    EXPECT_TRUE(negative < zero);
    EXPECT_TRUE(zero < positive);
    EXPECT_TRUE(negative < positive);
    EXPECT_TRUE(negative != positive);
    EXPECT_TRUE(positive > negative);
}

// ============================================================================
// floating_point Tests
// ============================================================================

class FloatingPointTest : public ::testing::Test {
protected:
    void SetUp() override {
        simple = "3.14";
        negative = "-2.5";
        no_decimal = "42";
        leading_decimal = ".5";
        trailing_decimal = "5.";
        zero = "0.0";
    }

    std::string simple;
    std::string negative;
    std::string no_decimal;
    std::string leading_decimal;
    std::string trailing_decimal;
    std::string zero;
};

TEST_F(FloatingPointTest, FactorySimple) {
    auto result = make_floating_point(simple);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 3.14, 1e-10);
}

TEST_F(FloatingPointTest, FactoryNegative) {
    auto result = make_floating_point(negative);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), -2.5, 1e-10);
}

TEST_F(FloatingPointTest, FactoryNoDecimal) {
    auto result = make_floating_point(no_decimal);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 42.0, 1e-10);
}

TEST_F(FloatingPointTest, FactoryLeadingDecimal) {
    auto result = make_floating_point(leading_decimal);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 0.5, 1e-10);
}

TEST_F(FloatingPointTest, FactoryTrailingDecimal) {
    auto result = make_floating_point(trailing_decimal);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 5.0, 1e-10);
}

TEST_F(FloatingPointTest, FactoryInvalidMultipleDecimals) {
    auto result = make_floating_point("3.14.159");
    EXPECT_FALSE(result.has_value());
}

TEST_F(FloatingPointTest, FactoryInvalidChars) {
    auto result = make_floating_point("3.14abc");
    EXPECT_FALSE(result.has_value());
}

TEST_F(FloatingPointTest, MonoidIdentity) {
    auto zero = floating_point{};
    auto value = *make_floating_point("3.14");

    EXPECT_EQ(zero * value, value);
    EXPECT_EQ(value * zero, value);
}

TEST_F(FloatingPointTest, MonoidAssociativity) {
    auto a = *make_floating_point("1.5");
    auto b = *make_floating_point("2.5");
    auto c = *make_floating_point("3.5");

    EXPECT_EQ((a * b) * c, a * (b * c));
}

TEST_F(FloatingPointTest, Addition) {
    auto a = *make_floating_point("1.5");
    auto b = *make_floating_point("2.5");

    auto result = a + b;
    EXPECT_NEAR(result.val(), 4.0, 1e-10);
}

TEST_F(FloatingPointTest, Subtraction) {
    auto a = *make_floating_point("5.5");
    auto b = *make_floating_point("2.5");

    auto result = a - b;
    EXPECT_NEAR(result.val(), 3.0, 1e-10);
}

TEST_F(FloatingPointTest, ComparisonWithEpsilon) {
    auto a = *make_floating_point("1.0");
    auto b = *make_floating_point("1.00000000001");  // Within epsilon (1e-11 < 1e-10)

    EXPECT_TRUE(a == b);  // Should be equal within epsilon
}

TEST_F(FloatingPointTest, EmptyDetection) {
    auto zero = floating_point{};
    auto small = *make_floating_point("0.00000000001");
    auto large = *make_floating_point("1.0");

    EXPECT_TRUE(zero.empty());
    EXPECT_TRUE(small.empty());  // Below epsilon threshold
    EXPECT_FALSE(large.empty());
}

// ============================================================================
// scientific_notation Tests
// ============================================================================

class ScientificNotationTest : public ::testing::Test {
protected:
    void SetUp() override {
        simple = "1.5e10";
        negative_exp = "3.2e-5";
        capital_e = "2.5E10";
        no_decimal = "5e3";
    }

    std::string simple;
    std::string negative_exp;
    std::string capital_e;
    std::string no_decimal;
};

TEST_F(ScientificNotationTest, FactorySimple) {
    auto result = make_scientific_notation(simple);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 1.5e10, 1e-10);
}

TEST_F(ScientificNotationTest, FactoryNegativeExponent) {
    auto result = make_scientific_notation(negative_exp);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 3.2e-5, 1e-15);
}

TEST_F(ScientificNotationTest, FactoryCapitalE) {
    auto result = make_scientific_notation(capital_e);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 2.5e10, 1e-10);
}

TEST_F(ScientificNotationTest, FactoryNoDecimal) {
    auto result = make_scientific_notation(no_decimal);
    ASSERT_TRUE(result.has_value());
    EXPECT_NEAR(result->val(), 5000.0, 1e-10);
}

TEST_F(ScientificNotationTest, FactoryInvalidNoE) {
    auto result = make_scientific_notation("123.45");
    EXPECT_FALSE(result.has_value());  // Must have 'e' or 'E'
}

TEST_F(ScientificNotationTest, MonoidIdentity) {
    auto zero = scientific_notation{};
    auto value = *make_scientific_notation("1.5e10");

    EXPECT_EQ(zero * value, value);
    EXPECT_EQ(value * zero, value);
}

TEST_F(ScientificNotationTest, MonoidAssociativity) {
    auto a = *make_scientific_notation("1e5");
    auto b = *make_scientific_notation("2e5");
    auto c = *make_scientific_notation("3e5");

    EXPECT_EQ((a * b) * c, a * (b * c));
}

TEST_F(ScientificNotationTest, Addition) {
    auto a = *make_scientific_notation("1e5");
    auto b = *make_scientific_notation("2e5");

    auto result = a + b;
    EXPECT_NEAR(result.val(), 3e5, 1e-5);
}

TEST_F(ScientificNotationTest, StringFormatting) {
    auto value = *make_scientific_notation("1.5e10");
    std::string str = value.str();

    // Should be formatted in scientific notation
    EXPECT_NE(str.find('e'), std::string::npos);
}

TEST_F(ScientificNotationTest, StreamOutput) {
    auto value = *make_scientific_notation("2.5e-3");
    std::ostringstream oss;
    oss << value;

    // Should contain scientific notation format
    EXPECT_NE(oss.str().find('e'), std::string::npos);
}

// ============================================================================
// Integration Tests: Cross-type Interactions
// ============================================================================

TEST(NumericIntegrationTest, VectorOfMixedTypes) {
    std::vector<unsigned_int> uints;
    uints.push_back(*make_unsigned_int("100"));
    uints.push_back(*make_unsigned_int("200"));

    std::vector<signed_int> sints;
    sints.push_back(*make_signed_int("-50"));
    sints.push_back(*make_signed_int("50"));

    std::vector<floating_point> floats;
    floats.push_back(*make_floating_point("3.14"));
    floats.push_back(*make_floating_point("2.71"));

    EXPECT_EQ(uints.size(), 2UL);
    EXPECT_EQ(sints.size(), 2UL);
    EXPECT_EQ(floats.size(), 2UL);
}

TEST(NumericIntegrationTest, CompositionChains) {
    // Chain unsigned ints
    auto u1 = *make_unsigned_int("10");
    auto u2 = *make_unsigned_int("20");
    auto u3 = *make_unsigned_int("30");
    auto sum = u1 * u2 * u3;
    EXPECT_EQ(sum.val(), 60ULL);

    // Chain signed ints
    auto s1 = *make_signed_int("10");
    auto s2 = *make_signed_int("-5");
    auto s3 = *make_signed_int("15");
    auto signed_sum = s1 * s2 * s3;
    EXPECT_EQ(signed_sum.val(), 20LL);

    // Chain floats
    auto f1 = *make_floating_point("1.5");
    auto f2 = *make_floating_point("2.5");
    auto f3 = *make_floating_point("3.0");
    auto float_sum = f1 * f2 * f3;
    EXPECT_NEAR(float_sum.val(), 7.0, 1e-10);
}

TEST(NumericIntegrationTest, OptionalChaining) {
    auto valid1 = make_unsigned_int("100");
    auto valid2 = make_unsigned_int("200");
    auto invalid = make_unsigned_int("invalid");

    // Valid chain
    auto result1 = valid1 * valid2;
    ASSERT_TRUE(result1.has_value());
    EXPECT_EQ(result1->val(), 300ULL);

    // Invalid in chain
    auto result2 = valid1 * invalid * valid2;
    EXPECT_FALSE(result2.has_value());
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
