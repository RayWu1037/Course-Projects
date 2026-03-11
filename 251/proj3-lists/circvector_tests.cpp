#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdexcept>
#include "circvector.h"

using namespace std;
using namespace testing;

TEST(CircVectorCore, EmptyVectorInitially) {
    CircVector<int> cv;
    EXPECT_TRUE(cv.empty());
    EXPECT_EQ(cv.size(), 0);
}

TEST(CircVectorCore, PushBackAddsElements) {
    CircVector<int> cv;
    cv.push_back(1);
    cv.push_back(2);
    cv.push_back(3);
    EXPECT_EQ(cv.size(), 3);
    EXPECT_EQ(cv.at(0), 1);
    EXPECT_EQ(cv.at(1), 2);
    EXPECT_EQ(cv.at(2), 3);
}

TEST(CircVectorCore, PushFrontAddsElements) {
    CircVector<int> cv;
    cv.push_front(10);
    cv.push_front(20);
    cv.push_front(30);
    EXPECT_EQ(cv.size(), 3);
    EXPECT_EQ(cv.at(0), 30);
    EXPECT_EQ(cv.at(1), 20);
    EXPECT_EQ(cv.at(2), 10);
}

TEST(CircVectorCore, PopFrontRemovesElements) {
    CircVector<int> cv;
    cv.push_back(1);
    cv.push_back(2);
    cv.push_back(3);
    cv.pop_front();
    EXPECT_EQ(cv.size(), 2);
    EXPECT_EQ(cv.at(0), 2);
}

TEST(CircVectorCore, PopBackRemovesElements) {
    CircVector<int> cv;
    cv.push_back(1);
    cv.push_back(2);
    cv.push_back(3);
    cv.pop_back();
    EXPECT_EQ(cv.size(), 2);
    EXPECT_EQ(cv.at(1), 2);
}

TEST(CircVectorCore, AtThrowsOnInvalidIndex) {
    CircVector<int> cv;
    EXPECT_THROW(cv.at(0), out_of_range);
}

TEST(CircVectorCore, ClearRemovesAllElements) {
    CircVector<int> cv;
    cv.push_back(10);
    cv.push_back(20);
    cv.push_back(30);
    cv.clear();
    EXPECT_TRUE(cv.empty());
}

TEST(CircVectorExtras, InsertAfterInsertsCorrectly) {
    CircVector<int> cv;
    cv.push_back(10);
    cv.push_back(20);
    cv.push_back(30);

    cv.insert_after(1, 25);
    EXPECT_EQ(cv.size(), 4);
    EXPECT_EQ(cv.at(0), 10);
    EXPECT_EQ(cv.at(1), 20);
    EXPECT_EQ(cv.at(2), 25);
    EXPECT_EQ(cv.at(3), 30);

    EXPECT_THROW(cv.insert_after(10, 100), out_of_range);
}

TEST(CircVectorExtras, RemoveEvensRemovesCorrectly) {
    CircVector<int> cv;
    cv.push_back(3);
    cv.push_back(9);
    cv.push_back(7);
    cv.push_back(6);
    cv.push_back(8);
    cv.remove_evens();
    EXPECT_EQ(cv.size(), 2);
    EXPECT_EQ(cv.at(0), 9);
    EXPECT_EQ(cv.at(1), 6);

    CircVector<int> empty;
    empty.remove_evens();
    EXPECT_TRUE(empty.empty());
}
