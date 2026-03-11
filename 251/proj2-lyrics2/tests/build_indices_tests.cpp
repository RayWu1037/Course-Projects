#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <set>
#include <string>

#include "include/lyrics.h"

using namespace std;
using namespace testing;

namespace {

TEST(BuildIndices, TinierTxt) {
  string filename = "data/tinier.txt";
  map<string, set<string>> expectedInvertedIndex = {
      {"another", {"Song 2"}},        {"for", {"Song 1", "Song 2"}},
      {"purposes", {"Song 1"}},       {"some", {"Song 1"}},
      {"song", {"Song 1", "Song 2"}}, {"testing", {"Song 1", "Song 2"}}};

  map<string, string> expectedArtistIndex = {{"Song 1", "Someone"},
                                             {"Song 2", "Someone Else"}};

  map<string, set<string>> studentInvertedIndex;
  map<string, string> studentArtistIndex;
  int studentNumProcesed =
      buildIndices(filename, studentInvertedIndex, studentArtistIndex);

  string invertedIndexTestFeedback =
      "buildIndex(\"" + filename + "\", ...) inverted index incorrect\n";
  EXPECT_THAT(studentInvertedIndex, ContainerEq(expectedInvertedIndex))
      << invertedIndexTestFeedback;

  string artistIndexTestFeedback =
      "buildIndices(\"" + filename + "\", ...) artist index incorrect\n";
  EXPECT_THAT(studentArtistIndex, ContainerEq(expectedArtistIndex))
      << artistIndexTestFeedback;

  string retTestFeedback =
      "buildIndex(\"" + filename + "\", ...) return value incorrect\n";
  EXPECT_THAT(studentNumProcesed, Eq(2)) << retTestFeedback;
}

}  // namespace
