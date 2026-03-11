#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <algorithm>
#include <map>
#include <random>
#include <string>
#include <vector>
#include <sstream>
#include "bstmap.h"

using namespace testing;

template <class K, class V>
static std::vector<K> inorderKeys(BSTMap<K,V>& m) {
  std::vector<K> keys;
  m.begin();
  K k; V v;
  while (m.next(k, v)) keys.push_back(k);
  return keys;
}
template <class K, class V>
static std::vector<std::pair<K,V>> inorderPairs(BSTMap<K,V>& m) {
  std::vector<std::pair<K,V>> out;
  m.begin();
  K k; V v;
  while (m.next(k, v)) out.emplace_back(k, v);
  return out;
}

struct Counted {
  static int live;
  int x;
  Counted(int v=0): x(v) { ++live; }
  Counted(const Counted& o): x(o.x) { ++live; }
  Counted& operator=(const Counted& o) { x=o.x; return *this; }
  ~Counted() { --live; }
  bool operator==(const Counted& o) const { return x == o.x; }
};
int Counted::live = 0;

TEST(BSTMapCore, CtorEmptyAndSizeZero) {  
  BSTMap<int,int> m;
  EXPECT_TRUE(m.empty());
  EXPECT_EQ(m.size(), 0u);
}

TEST(BSTMapCore, InsertAndContainsAndSize) { 
  BSTMap<int,int> m;
  m.insert(2,20);
  EXPECT_TRUE(m.contains(2));
  EXPECT_FALSE(m.contains(3));
  EXPECT_EQ(m.size(), 1u);
}

TEST(BSTMapCore, InsertDuplicateDoesNotOverwrite) {
  BSTMap<int,int> m;
  m.insert(5,50);
  m.insert(5,500); 
  EXPECT_EQ(m.size(), 1u);
  EXPECT_EQ(m.at(5), 50);
}

TEST(BSTMapCore, AtThrowsOnMissingAndNotThrowOnPresent) { 
  BSTMap<int,int> m;
  m.insert(1,10);
  EXPECT_NO_THROW( (void)m.at(1) );
  EXPECT_THROW( (void)m.at(99), std::out_of_range );
}

TEST(BSTMapCore, ToStringIsInOrderByKeys) {
  BSTMap<int,std::string> m;
  m.insert(5,"five"); m.insert(2,"two"); m.insert(8,"eight");
  m.insert(1,"one");  m.insert(3,"three");

  auto keys = inorderKeys(m);
  EXPECT_THAT(keys, ElementsAre(1,2,3,5,8));

  std::string s = m.to_string();
  std::vector<std::string> got;
  std::stringstream ss(s);
  std::string line;
  while (std::getline(ss, line)) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    got.push_back(line);
  }
  if (!got.empty() && got.back().empty()) got.pop_back();

  std::vector<std::string> want = {
    "1: one","2: two","3: three","5: five","8: eight"
  };
  ASSERT_EQ(got.size(), want.size());
  for (size_t i = 0; i < want.size(); ++i) {
    EXPECT_EQ(got[i], want[i]);
  }
}


TEST(BSTMapCore, ClearResetsStateAndDeletesNodes) { 
  Counted::live = 0;
  {
    BSTMap<int,Counted> m;
    m.insert(3, Counted{30});
    m.insert(1, Counted{10});
    m.insert(4, Counted{40});
    EXPECT_EQ(m.size(), 3u);
    m.clear();
    EXPECT_TRUE(m.empty());
    EXPECT_EQ(m.size(), 0u);
  } 
  EXPECT_EQ(Counted::live, 0);
}

TEST(BSTMapCore, DestructorDeletesAllNodes) { 
  Counted::live = 0;
  {
    BSTMap<int,Counted> m;
    for (int k: {5,2,8,1,3}) m.insert(k, Counted{k});
    EXPECT_GT(Counted::live, 0);
  }
  EXPECT_EQ(Counted::live, 0);
}

TEST(BSTMapCore, CopyConstructorDeepCopy) {     
  BSTMap<int,int> a;
  for (int k: {5,2,8,1,3}) a.insert(k,k*10);
  BSTMap<int,int> b = a;  
  EXPECT_TRUE(b.contains(2));
  EXPECT_EQ(b.size(), a.size());
  a.erase(2); 
  EXPECT_FALSE(a.contains(2));
  EXPECT_TRUE(b.contains(2));
}

TEST(BSTMapCore, AssignmentDeepCopyAndSelfAssign) {
  BSTMap<int,int> a; for (int k: {5,2,8}) a.insert(k,k);
  BSTMap<int,int> b; for (int k: {1,3,4}) b.insert(k,k);
  size_t a0 = a.size();
  b = a;
  EXPECT_EQ(b.size(), a0);
  EXPECT_TRUE(b.contains(5));
  BSTMap<int,int>* pb = &b;
  b = *pb;
  EXPECT_EQ(b.size(), a0);
  EXPECT_TRUE(b.contains(5));
  BSTMap<int,int> empty;
  a = empty;
  EXPECT_TRUE(a.empty());
  EXPECT_EQ(a.size(), 0u);
}

TEST(BSTMapCore, EmptyReflectsContents) { 
  BSTMap<int,int> m;
  EXPECT_TRUE(m.empty()); 
  m.insert(42, 420);
  EXPECT_FALSE(m.empty());
  m.clear();
  EXPECT_TRUE(m.empty()); 
}

TEST(BSTMapCore, CopyOfEmptyIsEmpty) {  
  BSTMap<int,int> a;  
  BSTMap<int,int> b = a;   
  EXPECT_TRUE(b.empty());
  EXPECT_EQ(b.size(), 0u);

  b.insert(7, 70);
  EXPECT_TRUE(a.empty());
  EXPECT_FALSE(a.contains(7));
  EXPECT_TRUE(b.contains(7));
}

TEST(BSTMapAugmented, RemoveMinThrowsOnEmpty) {
  BSTMap<int,int> m;
  EXPECT_THROW(m.remove_min(), std::runtime_error);
}

TEST(BSTMapAugmented, RemoveMinDecrementsSizeAndReturnsPair) {
  BSTMap<int,std::string> m;
  m.insert(6,"six"); m.insert(3,"three"); m.insert(8,"eight");
  size_t sz0 = m.size();
  auto p = m.remove_min();
  EXPECT_EQ(p.first, 3);
  EXPECT_EQ(p.second, "three");
  EXPECT_FALSE(m.contains(3));
  EXPECT_EQ(m.size(), sz0 - 1);
}

TEST(BSTMapAugmented, RemoveMinRootPromotion) {
  BSTMap<int,int> m;
  m.insert(1,10); m.insert(2,20); m.insert(3,30);
  auto p = m.remove_min();
  EXPECT_EQ(p.first, 1);
  EXPECT_FALSE(m.contains(1));
  auto keys = inorderKeys(m);
  EXPECT_THAT(keys, ElementsAre(2,3));
}

TEST(BSTMapAugmented, RemoveMinKeepsRemainingStructure) {
  BSTMap<double,int> m;
  m.insert(4,0); m.insert(2,0); m.insert(7,0); m.insert(1,0); m.insert(3,0);
  m.insert(1.5,0);
  auto p = m.remove_min();              
  EXPECT_EQ(p.first, 1);
  EXPECT_FALSE(m.contains(1));
  for (double k: {1.5,2.0,3.0,4.0,7.0}) EXPECT_TRUE(m.contains(k));
  auto keys = inorderKeys(m);
  EXPECT_TRUE(std::is_sorted(keys.begin(), keys.end()));
  EXPECT_EQ(keys.size(), 5u);
}

TEST(BSTMapAugmented, EqualityChecksSizeAndValues) {
  BSTMap<int,int> a,b,c;
  a.insert(1,10); a.insert(2,20);
  b.insert(1,10);      
  c.insert(1,999); c.insert(2,20); 
  EXPECT_FALSE(a == b); 
  EXPECT_FALSE(b == a); 
  EXPECT_FALSE(a == c);  
  BSTMap<int,int> d;
  d.insert(2,20); d.insert(1,10); 
  EXPECT_TRUE(a == d);
}

TEST(BSTMapAugmented, BeginNextOnEmptyReturnsFalse) {
  BSTMap<int,int> m;
  m.begin();
  int k,v;
  EXPECT_FALSE(m.next(k,v));
}

TEST(BSTMapAugmented, BeginThenFirstNextIsMinimum) {
  BSTMap<int,int> m;
  for (int x: {5,2,8,1,3}) m.insert(x,x);
  m.begin();
  int k,v;
  ASSERT_TRUE(m.next(k,v));
  EXPECT_EQ(k, 1);
}

TEST(BSTMapAugmented, IterateVisitsAllAndStopsNoSkipNoRevisit) {
  BSTMap<int,int> m;
  for (int x: {10,5,15,3,7,12,18,1,4,6,8}) m.insert(x,x);
  auto keys = inorderKeys(m);
  EXPECT_EQ(keys.size(), m.size());
  EXPECT_TRUE(std::is_sorted(keys.begin(), keys.end()));
  EXPECT_EQ(std::vector<int>(keys.begin(), std::unique(keys.begin(), keys.end())).size(),
            keys.size());
  int k,v;
  EXPECT_FALSE(m.next(k,v));
  m.begin();
  EXPECT_TRUE(m.next(k,v));
  EXPECT_EQ(k, 1);
}

TEST(BSTMapErase, EraseLeafAndOneChild) {
  BSTMap<int,int> m;
  for (int k: {5,2,8,1,3,4}) m.insert(k,k);
  int v1 = m.erase(1);
  EXPECT_EQ(v1, 1);
  EXPECT_FALSE(m.contains(1));
  int v2 = m.erase(3);
  EXPECT_EQ(v2, 3);
  EXPECT_FALSE(m.contains(3));
  EXPECT_THAT(inorderKeys(m), ElementsAre(2,4,5,8));
}

TEST(BSTMapErase, EraseTwoChildren_SuccessorImmediate) {
  BSTMap<int,int> m;
  m.insert(2,20); m.insert(1,10); m.insert(3,30);
  int v = m.erase(2);
  EXPECT_EQ(v, 20);
  EXPECT_FALSE(m.contains(2));
  EXPECT_THAT(inorderKeys(m), ElementsAre(1,3));
}

TEST(BSTMapErase, EraseTwoChildren_SuccessorHasRightChild) {
  BSTMap<int,int> m;
  for (int k: {55,29,60,87,93,96,99,98,72,91}) m.insert(k,k);
  int v = m.erase(55);
  EXPECT_EQ(v, 55);
  EXPECT_FALSE(m.contains(55));
  EXPECT_TRUE(m.contains(60));
  auto keys = inorderKeys(m);
  EXPECT_TRUE(std::is_sorted(keys.begin(), keys.end()));
}

TEST(BSTMapErase, EraseThrowsOnMissingKey) {
  BSTMap<int,int> m;
  m.insert(1,10);
  EXPECT_THROW(m.erase(42), std::out_of_range);
}

TEST(BSTMapErase, EraseRootOnlyNode) {
  BSTMap<int,int> m;
  m.insert(5,50);
  int v = m.erase(5);
  EXPECT_EQ(v, 50);
  EXPECT_TRUE(m.empty());
  EXPECT_EQ(m.size(), 0u);
}

TEST(BSTMapErase, EraseTwoChildren_SuccessorNotRightChild) {
  BSTMap<int,int> m;
  for (int k : {50,20,70,60}) m.insert(k,k);
  int v = m.erase(50);
  EXPECT_EQ(v, 50);
  EXPECT_FALSE(m.contains(50));
  auto keys = inorderKeys(m);
  EXPECT_THAT(keys, ElementsAre(20,60,70));
}
