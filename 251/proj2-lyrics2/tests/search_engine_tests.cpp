#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <regex>
#include <string>
#include <vector>

#include "include/lyrics.h"

using namespace std;
using namespace testing;

namespace {

// GoogleTest uses printf, which doesn't rely on cout -- so capturing
// works fine!
class CaptureCinCout : public testing::Test {
 protected:
  std::stringstream input;
  std::stringstream output;

  std::streambuf* saved_cin{};
  std::streambuf* saved_cout{};

  CaptureCinCout() {
    saved_cin = std::cin.rdbuf();
    saved_cout = std::cout.rdbuf();
    std::cin.rdbuf(input.rdbuf());
    std::cout.rdbuf(output.rdbuf());
  }

  ~CaptureCinCout() {
    std::cin.rdbuf(saved_cin);
    std::cout.rdbuf(saved_cout);
    saved_cin = nullptr;
    saved_cout = nullptr;
  }

  // Debugging tool to exfiltrate despite redirection
  // Could also use printf
  void write_to_cout(std::string s) {
    saved_cout->sputn(s.c_str(), s.length());
  }
};

// This should be a matcher, but I couldn't figure it out
bool strWhitespaceEq(string actual, string expected) {
  // Account for ending newline
  actual.push_back('\n');
  expected.push_back('\n');
  string expected_ = regex_replace(expected + "\n", regex("\\s+"), "_");
  string actual_ = regex_replace(actual + "\n", regex("\\s+"), "_");
  return actual_ == expected_;
}

class SearchEngine : public CaptureCinCout {};

TEST_F(SearchEngine, MissingFile) {
  input << "query" << endl;
  input << endl;
  input << endl;
  searchEngine("missingfile.txt");

  vector<string> expectedLines = {
      // Response to missing file
      "Invalid filename.",
      // Info output
      "Stand by while building indices...",
      "Indexed 0 songs containing 0 unique terms and 0 artists.",
      // Prompt, response to "query"
      "Enter query sentence (press enter to quit): Found 0 matching songs",
      // Prompt, exit message for ""
      "Enter query sentence (press enter to quit): Thank you for searching our "
      "Lyrics DB!",
  };
  ostringstream oss;
  for (const auto& s : expectedLines) {
    oss << s << endl;
  }
  string expectedJoined = oss.str();
  string actualOutput = output.str();

  EXPECT_TRUE(strWhitespaceEq(actualOutput, expectedJoined))
      << "See the test for the expected output. We ignore whitespace "
         "differences."
      << endl
      << "Your output:" << endl
      << actualOutput;
}

TEST_F(SearchEngine, beyonce) {
  input << "happy" << endl;
  input << "happy +me" << endl;
  input << "happy +me +still" << endl;
  input << "happy +me -still" << endl;
  input << "happy name day" << endl;
  input << "good +evil" << endl;
  input << endl;
  input << endl;

  searchEngine("data/beyonce.txt");

  vector<string> expectedLines = {
      // Info output
      "Stand by while building indices...",
      "Indexed 95 songs containing 2486 unique terms and 2 artists.",
      // Query "happy"
      "Enter query sentence (press enter to quit): Found 7 matching songs",
      "Broken-Hearted Girl by Beyonce",
      "Daddy by Beyonce",
      "Dreamgirls by Beyonce",
      "Lay Up Under Me by Beyonce",
      "Pretty Hurts by Beyonce",
      "Resentment by Beyonce",
      "Start Over by Beyonce",
      // Query "happy +me"
      "Enter query sentence (press enter to quit): Found 4 matching songs",
      "Broken-Hearted Girl by Beyonce",
      "Daddy by Beyonce",
      "Lay Up Under Me by Beyonce",
      "Resentment by Beyonce",
      // Query "happy +me +still"
      "Enter query sentence (press enter to quit): Found 3 matching songs",
      "Broken-Hearted Girl by Beyonce",
      "Daddy by Beyonce",
      "Lay Up Under Me by Beyonce",
      // Query "happy +me -still"
      "Enter query sentence (press enter to quit): Found 1 matching songs",
      "Resentment by Beyonce",
      // Query "happy bool ratio"
      "Enter query sentence (press enter to quit): Found 25 matching songs",
      "Baby Boy by Beyonce",
      "Be With You by Beyonce",
      "Broken-Hearted Girl by Beyonce",
      "Daddy by Beyonce",
      "Dance for You by Beyonce",
      "Dreamgirls by Beyonce",
      "Fever by Beyonce",
      "Gift from Virgo by Beyonce",
      "I Was Here by Beyonce",
      "If I Were a Boy by Beyonce",
      "Irreplaceable by Beyonce",
      "Lay Up Under Me by Beyonce",
      "Me, Myself and I by Beyonce",
      "Naughty Girl [Calderone Quayle Club Mix] by Beyonce",
      "Once In A Lifetime by Beyonce",
      "Pretty Hurts by Beyonce",
      "Resentment by Beyonce",
      "Sandcastles by Beyonce",
      "Save the Hero by Beyonce",
      "Say My Name by Destiny's Child",
      "Slow Love by Beyonce",
      "Start Over by Beyonce",
      "Stop That! by Beyonce",
      "Summertime by Beyonce",
      "Sweet Dreams by Beyonce",
      // Query "good +evil"
      "Enter query sentence (press enter to quit): Found 0 matching songs",
      // Exit on empty query
      "Enter query sentence (press enter to quit): Thank you for searching our "
      "Lyrics DB!",
  };
  ostringstream oss;
  for (const auto& s : expectedLines) {
    oss << s << endl;
  }
  string expectedJoined = oss.str();
  string actualOutput = output.str();

  EXPECT_TRUE(strWhitespaceEq(actualOutput, expectedJoined))
      << "See the test for the expected output. We ignore whitespace "
         "differences."
      << endl
      << "Your output:" << endl
      << actualOutput;
}

TEST_F(SearchEngine, various) {
  input << "dance" << endl;
  input << "dance +kiss" << endl;
  input << "dance -kiss" << endl;
  input << "kiss -dance" << endl;
  input << "kiss +dance" << endl;
  input << "kiss +dance grow" << endl;
  input << "kiss +dance grow -best" << endl;
  input << "kiss +dance grow -best -grow -kiss -dance" << endl;
  input << endl;
  input << endl;

  searchEngine("data/various.txt");

  // clang-format off
  vector<string> expectedLines = {
      // Info output
      "Stand by while building indices...",
      "Indexed 305 songs containing 4664 unique terms and 4 artists.",
      // Query "dance"
      "Enter query sentence (press enter to quit): Found 15 matching songs",
      "Birthday by Katy Perry",
      "Breakout by Katy Perry",
      "Chained to the Rhythm by Katy Perry",
      "Dancing with Our Hands Tied by Taylor Swift",
      "Drops of Jupiter (Live/2011) by Taylor Swift",
      "Fearless by Taylor Swift",
      "Holy Ground by Taylor Swift",
      "Lose Yourself / Smile by Taylor Swift",
      "Love Somebody by Maroon 5",
      "Never Grow Up by Taylor Swift",
      "Never Worn White by Katy Perry",
      "Out of the Woods by Taylor Swift",
      "Pendulum by Katy Perry",
      "Teenage Dream by Katy Perry",
      "Tommie Sunshine’s Megasix Smash-Up by Katy Perry",
      // Query "dance +kiss"
      "Enter query sentence (press enter to quit): Found 3 matching songs",
      "Dancing with Our Hands Tied by Taylor Swift",
      "Fearless by Taylor Swift",
      "Tommie Sunshine’s Megasix Smash-Up by Katy Perry",
      // Query "dance -kiss"
      "Enter query sentence (press enter to quit): Found 12 matching songs",
      "Birthday by Katy Perry",
      "Breakout by Katy Perry",
      "Chained to the Rhythm by Katy Perry",
      "Drops of Jupiter (Live/2011) by Taylor Swift",
      "Holy Ground by Taylor Swift",
      "Lose Yourself / Smile by Taylor Swift",
      "Love Somebody by Maroon 5",
      "Never Grow Up by Taylor Swift",
      "Never Worn White by Katy Perry",
      "Out of the Woods by Taylor Swift",
      "Pendulum by Katy Perry",
      "Teenage Dream by Katy Perry",
      // Query "kiss -dance"
      "Enter query sentence (press enter to quit): Found 19 matching songs",
      "Am I Ready for Love by Taylor Swift",
      "Beautiful Eyes by Taylor Swift",
      "Better Off by Taylor Swift",
      "Christmas Must Be Something More by Taylor Swift",
      "Come Back... Be Here by Taylor Swift",
      "Crocodile Tears by Katy Perry",
      "Down Came The Rain by Taylor Swift",
      "E.T. by Katy Perry",
      "Ghost by Katy Perry",
      "Hey Stephen by Taylor Swift",
      "Hook Up by Katy Perry",
      "Last Christmas by Taylor Swift",
      "Last Kiss by Taylor Swift",
      "Our Song by Taylor Swift",
      "Paper Rings by Taylor Swift",
      "Sad Beautiful Tragic by Taylor Swift",
      "Time’s Up by Katy Perry",
      "Whiskey by Maroon 5",
      "Who I Am by Maroon 5",
      // Query "kiss +dance"
      "Enter query sentence (press enter to quit): Found 3 matching songs",
      "Dancing with Our Hands Tied by Taylor Swift",
      "Fearless by Taylor Swift",
      "Tommie Sunshine’s Megasix Smash-Up by Katy Perry",
      // Query "kiss +dance grow"
      "Enter query sentence (press enter to quit): Found 10 matching songs",
      "Dancing with Our Hands Tied by Taylor Swift",
      "Drops of Jupiter (Live/2011) by Taylor Swift",
      "Fearless by Taylor Swift",
      "It Takes Two by Katy Perry",
      "Never Grow Up by Taylor Swift",
      "Plastic Rose by Maroon 5",
      "Resilient by Katy Perry",
      "Rita My love by Katy Perry",
      "Ronan by Taylor Swift",
      "Tommie Sunshine’s Megasix Smash-Up by Katy Perry",
      // Query "kiss +dance grow -best"
      "Enter query sentence (press enter to quit): Found 7 matching songs",
      "Dancing with Our Hands Tied by Taylor Swift",
      "It Takes Two by Katy Perry",
      "Never Grow Up by Taylor Swift",
      "Plastic Rose by Maroon 5",
      "Resilient by Katy Perry",
      "Rita My love by Katy Perry",
      "Tommie Sunshine’s Megasix Smash-Up by Katy Perry",
      // Query "kiss +dance grow -best -grow -kiss -dance"
      "Enter query sentence (press enter to quit): Found 0 matching songs",
      // Exit on empty query
      "Enter query sentence (press enter to quit): Thank you for searching our Lyrics DB!",
  };
  // clang-format on
  ostringstream oss;
  for (const auto& s : expectedLines) {
    oss << s << endl;
  }
  string expectedJoined = oss.str();
  string actualOutput = output.str();

  EXPECT_TRUE(strWhitespaceEq(actualOutput, expectedJoined))
      << "See the test for the expected output. We ignore whitespace "
         "differences."
      << endl
      << "Your output:" << endl
      << actualOutput;
}

}  // namespace
