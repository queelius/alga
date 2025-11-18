/**
 * @file utf8_alpha_test.cpp
 * @brief Comprehensive tests for UTF-8 Unicode support
 *
 * Tests UTF-8 encoding/decoding, validation, and utf8_alpha type.
 */

#include <gtest/gtest.h>
#include "parsers/utf8_alpha.hpp"
#include <string>
#include <vector>

using namespace alga;
using namespace alga::utf8;

// ============================================================================
// UTF-8 Utility Tests
// ============================================================================

TEST(UTF8UtilsTest, SequenceLength) {
    EXPECT_EQ(utf8_sequence_length(0x41), 1UL);      // 'A'
    EXPECT_EQ(utf8_sequence_length(0xC3), 2UL);      // Start of 2-byte
    EXPECT_EQ(utf8_sequence_length(0xE2), 3UL);      // Start of 3-byte
    EXPECT_EQ(utf8_sequence_length(0xF0), 4UL);      // Start of 4-byte
    EXPECT_EQ(utf8_sequence_length(0x80), 0UL);      // Invalid (continuation byte)
}

TEST(UTF8UtilsTest, ValidateSequenceASCII) {
    EXPECT_TRUE(is_valid_utf8_sequence("A"));
    EXPECT_TRUE(is_valid_utf8_sequence("z"));
}

TEST(UTF8UtilsTest, ValidateSequence2Byte) {
    // é (U+00E9) = C3 A9
    std::string e_acute = "\xC3\xA9";
    EXPECT_TRUE(is_valid_utf8_sequence(e_acute));
}

TEST(UTF8UtilsTest, ValidateSequence3Byte) {
    // € (U+20AC) = E2 82 AC
    std::string euro = "\xE2\x82\xAC";
    EXPECT_TRUE(is_valid_utf8_sequence(euro));
}

TEST(UTF8UtilsTest, ValidateSequence4Byte) {
    // 𝕳 (U+1D573) = F0 9D 95 B3
    std::string math_h = "\xF0\x9D\x95\xB3";
    EXPECT_TRUE(is_valid_utf8_sequence(math_h));
}

TEST(UTF8UtilsTest, ValidateInvalidSequence) {
    // Invalid continuation byte
    std::string invalid = "\xC3\x28";
    EXPECT_FALSE(is_valid_utf8_sequence(invalid));
}

TEST(UTF8UtilsTest, DecodeASCII) {
    auto cp = decode_utf8("A");
    ASSERT_TRUE(cp.has_value());
    EXPECT_EQ(*cp, 'A');
}

TEST(UTF8UtilsTest, Decode2Byte) {
    // é (U+00E9)
    std::string e_acute = "\xC3\xA9";
    auto cp = decode_utf8(e_acute);
    ASSERT_TRUE(cp.has_value());
    EXPECT_EQ(*cp, 0x00E9U);
}

TEST(UTF8UtilsTest, Decode3Byte) {
    // € (U+20AC)
    std::string euro = "\xE2\x82\xAC";
    auto cp = decode_utf8(euro);
    ASSERT_TRUE(cp.has_value());
    EXPECT_EQ(*cp, 0x20ACU);
}

TEST(UTF8UtilsTest, Decode4Byte) {
    // 🔥 (U+1F525)
    std::string fire = "\xF0\x9F\x94\xA5";
    auto cp = decode_utf8(fire);
    ASSERT_TRUE(cp.has_value());
    EXPECT_EQ(*cp, 0x1F525U);
}

TEST(UTF8UtilsTest, EncodeASCII) {
    auto encoded = encode_utf8('A');
    ASSERT_TRUE(encoded.has_value());
    EXPECT_EQ(*encoded, "A");
}

TEST(UTF8UtilsTest, Encode2Byte) {
    auto encoded = encode_utf8(0x00E9);  // é
    ASSERT_TRUE(encoded.has_value());
    EXPECT_EQ(*encoded, "\xC3\xA9");
}

TEST(UTF8UtilsTest, Encode3Byte) {
    auto encoded = encode_utf8(0x20AC);  // €
    ASSERT_TRUE(encoded.has_value());
    EXPECT_EQ(*encoded, "\xE2\x82\xAC");
}

TEST(UTF8UtilsTest, Encode4Byte) {
    auto encoded = encode_utf8(0x1F525);  // 🔥
    ASSERT_TRUE(encoded.has_value());
    EXPECT_EQ(*encoded, "\xF0\x9F\x94\xA5");
}

TEST(UTF8UtilsTest, EncodeInvalidCodepoint) {
    auto encoded = encode_utf8(0x110000);  // Beyond valid range
    EXPECT_FALSE(encoded.has_value());
}

TEST(UTF8UtilsTest, IsUnicodeAlphaASCII) {
    EXPECT_TRUE(is_unicode_alpha('A'));
    EXPECT_TRUE(is_unicode_alpha('z'));
    EXPECT_FALSE(is_unicode_alpha('0'));
    EXPECT_FALSE(is_unicode_alpha('!'));
}

TEST(UTF8UtilsTest, IsUnicodeAlphaLatin) {
    EXPECT_TRUE(is_unicode_alpha(0x00E9));  // é
    EXPECT_TRUE(is_unicode_alpha(0x00C0));  // À
    EXPECT_FALSE(is_unicode_alpha(0x00D7)); // × (multiplication sign)
}

TEST(UTF8UtilsTest, IsUnicodeAlphaGreek) {
    EXPECT_TRUE(is_unicode_alpha(0x0391));  // Α (Alpha)
    EXPECT_TRUE(is_unicode_alpha(0x03C9));  // ω (omega)
}

TEST(UTF8UtilsTest, IsUnicodeAlphaCyrillic) {
    EXPECT_TRUE(is_unicode_alpha(0x0410));  // А
    EXPECT_TRUE(is_unicode_alpha(0x044F));  // я
}

TEST(UTF8UtilsTest, IsUnicodeAlphaHebrew) {
    EXPECT_TRUE(is_unicode_alpha(0x05D0));  // א (aleph)
    EXPECT_TRUE(is_unicode_alpha(0x05EA));  // ת (tav)
}

TEST(UTF8UtilsTest, IsUnicodeAlphaArabic) {
    EXPECT_TRUE(is_unicode_alpha(0x0627));  // ا
    EXPECT_TRUE(is_unicode_alpha(0x064A));  // ي
}

TEST(UTF8UtilsTest, ToLowercaseASCII) {
    EXPECT_EQ(to_lowercase('A'), 'a');
    EXPECT_EQ(to_lowercase('Z'), 'z');
    EXPECT_EQ(to_lowercase('a'), 'a');  // Already lowercase
}

TEST(UTF8UtilsTest, ToLowercaseLatin) {
    EXPECT_EQ(to_lowercase(0x00C0), 0x00E0);  // À → à
    EXPECT_EQ(to_lowercase(0x00DE), 0x00FE);  // Þ → þ
}

TEST(UTF8UtilsTest, ToLowercaseGreek) {
    EXPECT_EQ(to_lowercase(0x0391), 0x03B1);  // Α → α
    EXPECT_EQ(to_lowercase(0x03A9), 0x03C9);  // Ω → ω
}

TEST(UTF8UtilsTest, ValidateUTF8String) {
    EXPECT_TRUE(is_valid_utf8("hello"));
    EXPECT_TRUE(is_valid_utf8("café"));
    EXPECT_TRUE(is_valid_utf8("Привет"));  // Russian
    EXPECT_TRUE(is_valid_utf8("こんにちは"));  // Japanese
}

TEST(UTF8UtilsTest, ValidateInvalidUTF8String) {
    // Invalid UTF-8 sequences
    std::string invalid1 = "hello\xC3\x28world";  // Invalid continuation
    EXPECT_FALSE(is_valid_utf8(invalid1));
}

// ============================================================================
// utf8_alpha Tests
// ============================================================================

class UTF8AlphaTest : public ::testing::Test {
protected:
    void SetUp() override {
        ascii_input = "hello";
        latin_input = "café";
        greek_input = "αβγδ";    // alpha beta gamma delta
        cyrillic_input = "привет"; // Russian "hello"
        mixed_case = "HeLLo";
        invalid_numeric = "hello123";
        invalid_punctuation = "hello!";
    }

    std::string ascii_input;
    std::string latin_input;
    std::string greek_input;
    std::string cyrillic_input;
    std::string mixed_case;
    std::string invalid_numeric;
    std::string invalid_punctuation;
};

TEST_F(UTF8AlphaTest, FactoryASCII) {
    auto result = make_utf8_alpha(ascii_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->str(), "hello");
}

TEST_F(UTF8AlphaTest, FactoryLatin) {
    auto result = make_utf8_alpha(latin_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->str(), "café");
}

TEST_F(UTF8AlphaTest, FactoryGreek) {
    auto result = make_utf8_alpha(greek_input);
    ASSERT_TRUE(result.has_value());
    // Should preserve Greek characters
    EXPECT_FALSE(result->empty());
}

TEST_F(UTF8AlphaTest, FactoryCyrillic) {
    auto result = make_utf8_alpha(cyrillic_input);
    ASSERT_TRUE(result.has_value());
    EXPECT_FALSE(result->empty());
}

TEST_F(UTF8AlphaTest, FactoryMixedCase) {
    auto result = make_utf8_alpha(mixed_case);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result->str(), "hello");  // Converted to lowercase
}

TEST_F(UTF8AlphaTest, FactoryInvalidNumeric) {
    auto result = make_utf8_alpha(invalid_numeric);
    EXPECT_FALSE(result.has_value());
}

TEST_F(UTF8AlphaTest, FactoryInvalidPunctuation) {
    auto result = make_utf8_alpha(invalid_punctuation);
    EXPECT_FALSE(result.has_value());
}

TEST_F(UTF8AlphaTest, FactoryEmpty) {
    auto result = make_utf8_alpha("");
    ASSERT_TRUE(result.has_value());
    EXPECT_TRUE(result->empty());
}

TEST_F(UTF8AlphaTest, DefaultConstructor) {
    utf8_alpha ua;
    EXPECT_TRUE(ua.empty());
    EXPECT_EQ(ua.size(), 0UL);
}

TEST_F(UTF8AlphaTest, ValueSemantics) {
    auto original = *make_utf8_alpha("test");

    // Copy constructor
    utf8_alpha copy(original);
    EXPECT_EQ(copy.str(), "test");

    // Move constructor
    utf8_alpha moved(std::move(copy));
    EXPECT_EQ(moved.str(), "test");

    // Copy assignment
    utf8_alpha assigned;
    assigned = original;
    EXPECT_EQ(assigned.str(), "test");

    // Move assignment
    utf8_alpha move_assigned;
    move_assigned = std::move(assigned);
    EXPECT_EQ(move_assigned.str(), "test");
}

TEST_F(UTF8AlphaTest, MonoidIdentity) {
    auto empty = utf8_alpha{};
    auto value = *make_utf8_alpha("test");

    // Left identity
    EXPECT_EQ(empty * value, value);

    // Right identity
    EXPECT_EQ(value * empty, value);
}

TEST_F(UTF8AlphaTest, MonoidAssociativity) {
    auto a = *make_utf8_alpha("hello");
    auto b = *make_utf8_alpha("world");
    auto c = *make_utf8_alpha("test");

    EXPECT_EQ((a * b) * c, a * (b * c));
}

TEST_F(UTF8AlphaTest, MonoidComposition) {
    auto a = *make_utf8_alpha("hello");
    auto b = *make_utf8_alpha("world");

    auto result = a * b;
    EXPECT_EQ(result.str(), "helloworld");
}

TEST_F(UTF8AlphaTest, MonoidCompositionUnicode) {
    auto a = *make_utf8_alpha("café");
    auto b = *make_utf8_alpha("résumé");

    auto result = a * b;
    // Both strings should be concatenated
    EXPECT_FALSE(result.empty());
}

TEST_F(UTF8AlphaTest, ComparisonOperators) {
    auto small = *make_utf8_alpha("abc");
    auto large = *make_utf8_alpha("xyz");
    auto equal = *make_utf8_alpha("abc");

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

TEST_F(UTF8AlphaTest, ChoiceOperator) {
    auto empty = utf8_alpha{};
    auto value = *make_utf8_alpha("test");

    EXPECT_EQ((empty | value).str(), "test");
    EXPECT_EQ((value | empty).str(), "test");

    auto left = *make_utf8_alpha("left");
    auto right = *make_utf8_alpha("right");
    EXPECT_EQ((left | right).str(), "left");
}

TEST_F(UTF8AlphaTest, RepetitionOperator) {
    auto base = *make_utf8_alpha("hi");

    auto times_0 = base ^ 0;
    EXPECT_TRUE(times_0.empty());

    auto times_1 = base ^ 1;
    EXPECT_EQ(times_1.str(), "hi");

    auto times_3 = base ^ 3;
    EXPECT_EQ(times_3.str(), "hihihi");
}

TEST_F(UTF8AlphaTest, CharCount) {
    auto ascii = *make_utf8_alpha("hello");
    EXPECT_EQ(ascii.char_count(), 5UL);

    auto unicode = *make_utf8_alpha("café");
    EXPECT_EQ(unicode.char_count(), 4UL);  // 4 characters

    // Size in bytes vs character count
    EXPECT_GT(unicode.size(), unicode.char_count());  // Bytes > chars for non-ASCII
}

TEST_F(UTF8AlphaTest, Codepoints) {
    auto value = *make_utf8_alpha("abc");
    auto cps = value.codepoints();

    ASSERT_EQ(cps.size(), 3UL);
    EXPECT_EQ(cps[0], 'a');
    EXPECT_EQ(cps[1], 'b');
    EXPECT_EQ(cps[2], 'c');
}

TEST_F(UTF8AlphaTest, CodepointsUnicode) {
    // Create string with known Unicode points
    auto value = *make_utf8_alpha("café");
    auto cps = value.codepoints();

    ASSERT_EQ(cps.size(), 4UL);
    EXPECT_EQ(cps[0], 'c');
    EXPECT_EQ(cps[1], 'a');
    EXPECT_EQ(cps[2], 'f');
    EXPECT_EQ(cps[3], 0x00E9U);  // é
}

TEST_F(UTF8AlphaTest, StreamOutput) {
    auto value = *make_utf8_alpha("hello");
    std::ostringstream oss;
    oss << value;
    EXPECT_EQ(oss.str(), "hello");
}

TEST_F(UTF8AlphaTest, FunctionApplication) {
    auto value = *make_utf8_alpha("test");

    auto char_count = value % [](utf8_alpha const& ua) { return ua.char_count(); };
    EXPECT_EQ(char_count, 4UL);

    auto opt_value = make_utf8_alpha("hello");
    auto opt_result = opt_value % [](utf8_alpha const& ua) { return ua.size(); };
    ASSERT_TRUE(opt_result.has_value());
    EXPECT_EQ(*opt_result, 5UL);
}

TEST_F(UTF8AlphaTest, ContainerStorage) {
    std::vector<utf8_alpha> words;
    words.push_back(*make_utf8_alpha("hello"));
    words.push_back(*make_utf8_alpha("world"));
    words.push_back(*make_utf8_alpha("café"));

    EXPECT_EQ(words.size(), 3UL);
    EXPECT_EQ(words[0].str(), "hello");
    EXPECT_EQ(words[1].str(), "world");
    EXPECT_EQ(words[2].str(), "café");
}

// ============================================================================
// Integration Tests
// ============================================================================

TEST(UTF8IntegrationTest, MultiScriptSupport) {
    // Test multiple scripts
    auto english = make_utf8_alpha("hello");
    auto french = make_utf8_alpha("bonjour");
    auto greek = make_utf8_alpha("γεια");
    auto russian = make_utf8_alpha("привет");

    EXPECT_TRUE(english.has_value());
    EXPECT_TRUE(french.has_value());
    EXPECT_TRUE(greek.has_value());
    EXPECT_TRUE(russian.has_value());
}

TEST(UTF8IntegrationTest, CaseConversion) {
    auto upper = make_utf8_alpha("HELLO");
    auto lower = make_utf8_alpha("hello");

    ASSERT_TRUE(upper.has_value());
    ASSERT_TRUE(lower.has_value());

    // Both should be converted to lowercase
    EXPECT_EQ(upper->str(), lower->str());
}

TEST(UTF8IntegrationTest, CompositionChain) {
    auto a = *make_utf8_alpha("hello");
    auto b = *make_utf8_alpha("beautiful");
    auto c = *make_utf8_alpha("world");

    auto result = a * b * c;
    EXPECT_EQ(result.str(), "hellobeautifulworld");
}

// ============================================================================
// Main
// ============================================================================

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
