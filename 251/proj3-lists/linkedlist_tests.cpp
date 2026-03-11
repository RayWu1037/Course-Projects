#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "linkedlist.h"
#include <stdexcept>

using namespace std;
using namespace testing;

TEST(LinkedListCore, EmptyListInitially) {
    LinkedList<int> ll;
    EXPECT_TRUE(ll.empty());
    EXPECT_EQ(ll.size(), 0);
}

TEST(LinkedListCore, PushFrontAddsElements) {
    LinkedList<int> ll;
    ll.push_front(10);
    ll.push_front(20);
    EXPECT_FALSE(ll.empty());
    EXPECT_EQ(ll.size(), 2);
    EXPECT_EQ(ll.at(0), 20);
    EXPECT_EQ(ll.at(1), 10);
}

TEST(LinkedListCore, PushBackAddsElements) {
    LinkedList<int> ll;
    ll.push_back(1);
    ll.push_back(2);
    ll.push_back(3);
    EXPECT_EQ(ll.size(), 3);
    EXPECT_EQ(ll.at(0), 1);
    EXPECT_EQ(ll.at(1), 2);
    EXPECT_EQ(ll.at(2), 3);
}

TEST(LinkedListCore, PopFrontRemovesHead) {
    LinkedList<int> ll;
    ll.push_back(10);
    ll.push_back(20);
    ll.push_back(30);
    ll.pop_front();
    EXPECT_EQ(ll.size(), 2);
    EXPECT_EQ(ll.at(0), 20);
    EXPECT_EQ(ll.at(1), 30);
}

TEST(LinkedListCore, PopBackRemovesTail) {
    LinkedList<int> ll;
    ll.push_back(100);
    ll.push_back(200);
    ll.push_back(300);
    ll.pop_back();
    EXPECT_EQ(ll.size(), 2);
    EXPECT_EQ(ll.at(1), 200);
}

TEST(LinkedListCore, ClearRemovesAll) {
    LinkedList<int> ll;
    ll.push_back(1);
    ll.push_back(2);
    ll.clear();
    EXPECT_TRUE(ll.empty());
    EXPECT_EQ(ll.size(), 0);
}

TEST(LinkedListCore, AtThrowsOnBadIndex) {
    LinkedList<int> ll;
    ll.push_back(5);
    EXPECT_THROW(ll.at(1), out_of_range);
    EXPECT_THROW(ll.at(-1), out_of_range);
}

TEST(LinkedListAugmented, CopyConstructorCreatesDeepCopy) {
    LinkedList<int> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);

    LinkedList<int> b(a);

    EXPECT_EQ(b.size(), 3);
    EXPECT_EQ(b.at(0), 1);
    EXPECT_EQ(b.at(1), 2);
    EXPECT_EQ(b.at(2), 3);

    a.pop_back();
    EXPECT_EQ(b.size(), 3);
}

TEST(LinkedListAugmented, AssignmentOperatorCopiesCorrectly) {
    LinkedList<int> a;
    a.push_back(5);
    a.push_back(6);

    LinkedList<int> b;
    b.push_back(10);
    b = a;

    EXPECT_EQ(b.size(), 2);
    EXPECT_EQ(b.at(0), 5);
    EXPECT_EQ(b.at(1), 6);

    a.clear();
    EXPECT_EQ(b.size(), 2);
}

TEST(LinkedListAugmented, ToStringOutputsProperFormat) {
    LinkedList<int> a;
    a.push_back(1);
    a.push_back(2);
    a.push_back(3);
    EXPECT_EQ(a.to_string(), "[1, 2, 3]");
}

TEST(LinkedListAugmented, FindReturnsCorrectIndex) {
    LinkedList<int> a;
    a.push_back(4);
    a.push_back(7);
    a.push_back(9);
    EXPECT_EQ(a.find(7), 1);
    EXPECT_EQ(a.find(10), static_cast<size_t>(-1));
}

TEST(LinkedListAugmented, RemoveAtDeletesCorrectNode) {
    LinkedList<int> a;
    a.push_back(11);
    a.push_back(22);
    a.push_back(33);
    a.remove_at(1);
    EXPECT_EQ(a.size(), 2);
    EXPECT_EQ(a.at(0), 11);
    EXPECT_EQ(a.at(1), 33);
    EXPECT_THROW(a.remove_at(5), out_of_range);
}

TEST(LinkedListExtras, InsertAfterAddsCorrectly) {
    LinkedList<int> a;
    a.push_back(10);
    a.push_back(20);
    a.push_back(30);

    a.insert_after(1, 25);
    EXPECT_EQ(a.size(), 4);
    EXPECT_EQ(a.at(0), 10);
    EXPECT_EQ(a.at(1), 20);
    EXPECT_EQ(a.at(2), 25);
    EXPECT_EQ(a.at(3), 30);

    EXPECT_THROW(a.insert_after(10, 5), out_of_range);
}

TEST(LinkedListExtras, RemoveEvensRemovesCorrectNodes) {
    LinkedList<int> a;
    a.push_back(3);
    a.push_back(9);
    a.push_back(7);
    a.push_back(6);
    a.push_back(8);
    a.remove_evens();  
    EXPECT_EQ(a.size(), 2);
    EXPECT_EQ(a.at(0), 9);
    EXPECT_EQ(a.at(1), 6);

    LinkedList<int> b;
    b.remove_evens();
    EXPECT_TRUE(b.empty());
}
