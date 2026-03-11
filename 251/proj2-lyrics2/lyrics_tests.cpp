#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include "include/lyrics.h"

using namespace std;
using namespace testing;

TEST(CleanToken, NoCleaning) {
  ASSERT_THAT(cleanToken("same"), StrEq("same"));
  ASSERT_THAT(cleanToken("wander"), StrEq("wander"));
  ASSERT_THAT(cleanToken("l33tcode"), StrEq("l33tcode"));
}

TEST(CleanToken, PrefixCleaning) {
  ASSERT_THAT(cleanToken(".hello"), StrEq("hello"));
  ASSERT_THAT(cleanToken("...hello"), StrEq("hello"));
  ASSERT_THAT(cleanToken(".\"!?hello"), StrEq("hello"));
  ASSERT_THAT(cleanToken(";timesheet"), StrEq("timesheet"));
  ASSERT_THAT(cleanToken(";.!timesheet"), StrEq("timesheet"));
  ASSERT_THAT(cleanToken(".,.!?timesheet"), StrEq("timesheet"));
}

TEST(CleanToken, SuffixCleaning) {
  ASSERT_THAT(cleanToken("hello."), StrEq("hello"));
  ASSERT_THAT(cleanToken("hello..."), StrEq("hello"));
  ASSERT_THAT(cleanToken("hello.\"!?"), StrEq("hello"));
  ASSERT_THAT(cleanToken("timesheet;"), StrEq("timesheet"));
  ASSERT_THAT(cleanToken("timesheet;.!"), StrEq("timesheet"));
  ASSERT_THAT(cleanToken("timesheet.,.!?"), StrEq("timesheet"));
}

TEST(CleanToken, ToEmpty) {
  ASSERT_THAT(cleanToken("23432423"), StrEq(""));
  ASSERT_THAT(cleanToken("....$$$$......"), StrEq(""));
  ASSERT_THAT(cleanToken("....2312^#@@@...."), StrEq(""));
  ASSERT_THAT(cleanToken(""), StrEq(""));
}

TEST(CleanToken, Uppercase) {
  ASSERT_THAT(cleanToken("HELLO."), StrEq("hello"));
  ASSERT_THAT(cleanToken("heLlo..."), StrEq("hello"));
  ASSERT_THAT(cleanToken("hellO.\"!?"), StrEq("hello"));
  ASSERT_THAT(cleanToken(".HELLO"), StrEq("hello"));
  ASSERT_THAT(cleanToken("...Hello"), StrEq("hello"));
  ASSERT_THAT(cleanToken(".\"!?heLLo"), StrEq("hello"));
}

TEST(CleanToken, PunctInMiddleOnly) {
  EXPECT_THAT(cleanToken("co-operate"), StrEq("co-operate"));
  EXPECT_THAT(cleanToken("ash's"), StrEq("ash's"));
}

TEST(CleanToken, PunctInMiddleAndStart) {
  EXPECT_THAT(cleanToken("...co-operate"), StrEq("co-operate"));
  EXPECT_THAT(cleanToken(".'ash's"), StrEq("ash's"));
}

TEST(CleanToken, PunctInMiddleAndEnd) {
  EXPECT_THAT(cleanToken("co-operate!!!"), StrEq("co-operate"));
  EXPECT_THAT(cleanToken("ash's?"), StrEq("ash's"));
}

TEST(GatherTokens, PokemonSentence) {
  string text = "Pikachu, I choose you! Thunder-shock!!!";
  set<string> expected = {"pikachu", "i", "choose", "you", "thunder-shock"};
  EXPECT_THAT(gatherTokens(text), ContainerEq(expected))
      << "text=\"" << text << "\"";
}

TEST(GatherTokens, SimpleColors) {
  string text = "Red & blue & gold";
  set<string> expected = {"red", "blue", "gold"};
  EXPECT_THAT(gatherTokens(text), ContainerEq(expected))
      << "text=\"" << text << "\"";
}

TEST(GatherTokens, MultipleSpacesBetweenTokensOnly) {
  string text = "time   and   time   again";
  set<string> expected = {"time","and","again"};
  EXPECT_THAT(gatherTokens(text), ContainerEq(expected))
      << "text=\"" << text << "\"";
}

TEST(GatherTokens, SpacesVariants) {
  string text = "   time   and   time   again  ";
  set<string> expected = {"time","and","again"};
  EXPECT_THAT(gatherTokens(text), ContainerEq(expected));
}

TEST(BuildIndices, TinierTxt_GivenData) {
  string filename = "data/tinier.txt";
  map<string, set<string>> expectedInvertedIndex = {
      {"another", {"Song 2"}},        {"for", {"Song 1", "Song 2"}},
      {"purposes", {"Song 1"}},       {"some", {"Song 1"}},
      {"song", {"Song 1", "Song 2"}}, {"testing", {"Song 1", "Song 2"}}};

  map<string, string> expectedArtistIndex = {
      {"Song 1", "Someone"}, {"Song 2", "Someone Else"}};

  map<string, set<string>> inv;
  map<string, string> art;
  int n = buildIndices(filename, inv, art);

  EXPECT_THAT(inv, ContainerEq(expectedInvertedIndex))
      << "inverted index mismatch for " << filename;
  EXPECT_THAT(art, ContainerEq(expectedArtistIndex))
      << "artist index mismatch for " << filename;
  EXPECT_THAT(n, Eq(2)) << "processed songs count mismatch";
}

TEST(BuildIndices, MissingFileReturnsZeroAndDoesNotClear) {
  map<string, set<string>> inv{{"keep", {"X"}}};
  map<string, string> art{{"T", "A"}};
  int n = buildIndices("no_such_file_hopefully.txt", inv, art);
  EXPECT_THAT(n, Eq(0));
  EXPECT_THAT(inv.at("keep"), ContainerEq(set<string>{"X"}));
  EXPECT_THAT(art.at("T"), StrEq("A"));
}

TEST(BuildIndices, TinyTxt_Basic) {
  const string filename = "data/tiny.txt";
  map<string, set<string>> inv;
  map<string, string> art;

  int n = buildIndices(filename, inv, art);

  EXPECT_GT(n, 0) << "tiny.txt should contain at least one song";

  EXPECT_FALSE(inv.empty());
  EXPECT_FALSE(art.empty());

  EXPECT_EQ(static_cast<size_t>(n), art.size())
      << "artist_index should map each processed title to exactly one artist";

  for (const auto& [token, titles] : inv) {
    for (const auto& title : titles) {
      EXPECT_TRUE(art.count(title))
          << "title \"" << title
          << "\" from inverted_index not present in artist_index";
    }
  }
}

TEST(FindQueryMatches, PokemonIndex_UnionIntersectionDifference) {
  map<string, set<string>> INDEX = {
      {"pikachu",   {"Kanto Theme", "Battle 1"}},
      {"charizard", {"Battle 1"}},
      {"electric",  {"Kanto Theme"}},
      {"fire",      {"Battle 1", "Lava Ridge"}},
      {"grass",     {"Verdant Forest"}},
      {"thunder",   {"Kanto Theme", "Battle 2"}},
  };

  EXPECT_THAT(findQueryMatches(INDEX, "pikachu"),
              ContainerEq(set<string>({"Battle 1","Kanto Theme"})));


  EXPECT_THAT(findQueryMatches(INDEX, "fire grass"),
              ContainerEq(set<string>({"Battle 1","Lava Ridge","Verdant Forest"})));


  EXPECT_THAT(findQueryMatches(INDEX, "electric +thunder"),
              ContainerEq(set<string>({"Kanto Theme"})));


  EXPECT_THAT(findQueryMatches(INDEX, "fire -charizard"),
              ContainerEq(set<string>({"Lava Ridge"})));

  EXPECT_THAT(findQueryMatches(INDEX, "dragonite +fire"),
              ContainerEq(set<string>({})));


  EXPECT_THAT(findQueryMatches(INDEX, "pikachu -thunder +electric"),
              ContainerEq(set<string>({})));
}

TEST(FindQueryMatches, MissingLaterPlusTerm) {
  map<string, set<string>> IDX = {
      {"fire", {"A", "B"}},
  };
  EXPECT_THAT(findQueryMatches(IDX, "fire +zzz"),
              ContainerEq(set<string>{}));
}

TEST(FindQueryMatches, MissingLaterMinusTerm) {
  map<string, set<string>> IDX = {
      {"fire", {"A", "B"}},
  };
  EXPECT_THAT(findQueryMatches(IDX, "fire -zzz"),
              ContainerEq(set<string>({"A", "B"})));
}

TEST(FindQueryMatches, MissingLaterUnionTerm) {
  map<string, set<string>> IDX = {
      {"fire", {"A", "B"}},
  };
  EXPECT_THAT(findQueryMatches(IDX, "fire zzz"),
              ContainerEq(set<string>({"A", "B"})));
}

class CaptureCinCout : public testing::Test {
 protected:
  stringstream in, out;
  streambuf *old_in = nullptr, *old_out = nullptr;

  void SetUp() override {
    old_in = cin.rdbuf(in.rdbuf());
    old_out = cout.rdbuf(out.rdbuf());
  }
  void TearDown() override {
    cin.rdbuf(old_in);
    cout.rdbuf(old_out);
  }
  static bool WhitespaceEq(const string& a, const string& b) {
    auto squash = [](string s) {
      string t; t.reserve(s.size());
      bool ws = false;
      for (char c: s) {
        if (isspace(static_cast<unsigned char>(c))) {
          if (!ws) { t.push_back('_'); ws = true; }
        } else { t.push_back(c); ws = false; }
      }
      return t;
    };
    return squash(a) == squash(b);
  }
};

TEST_F(CaptureCinCout, MissingFileFlow) {
  in << "pikachu\n"; 
  in << "\n";
  searchEngine("THIS_FILE_DOES_NOT_EXIST.txt");

  vector<string> expected = {
      "Invalid filename.",
      "Stand by while building indices...",
      "Indexed 0 songs containing 0 unique terms and 0 artists.",
      "Enter query sentence (press enter to quit): Found 0 matching songs",
      "Enter query sentence (press enter to quit): Thank you for searching our Lyrics DB!",
  };
  ostringstream oss;
  for (auto& s: expected) oss << s << '\n';
  EXPECT_TRUE(WhitespaceEq(out.str(), oss.str()))
      << "Your output:\n" << out.str();
}

