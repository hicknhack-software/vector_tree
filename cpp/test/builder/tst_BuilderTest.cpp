/* vector_tree
 * Copyright 2016 HicknHack Software GmbH
 *
 * The original code can be found at:
 *    https://github.com/hicknhack-software/vector_tree
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include "vector_tree/drift_tree.h"

#include <QString>
#include <QtTest>

#include <algorithm>

class BuilderTest : public QObject {
    Q_OBJECT
    using int_tree = vt::drift_tree<int>;

public:
    BuilderTest();

private:
    template <typename T>
    void checkInvariant(typename vt::drift_tree<T> tree) const;

private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void pushBackConstruction();
    void pushRootConstruction();
    void subtree();
};

BuilderTest::BuilderTest() {}

template <typename T>
void
BuilderTest::checkInvariant(typename vt::drift_tree<T> tree) const {
    auto sum = std::accumulate(tree.begin(), tree.end(), size_t(),
                               [](auto s, auto n) { return s + n.drift; });
    QVERIFY(sum == tree.size());
    QVERIFY(0 == tree.size() || tree.back().is_leaf());
}

void
BuilderTest::initTestCase() {}

void
BuilderTest::cleanupTestCase() {}

void
BuilderTest::pushBackConstruction() {
    int_tree t;
    /* 1
     *  2    5
     *   3 4  6
     *
     * <0,1> <0,2> <1,3> <2,4> <0,5> <3,6>
     */
    t.push_root(1);
    checkInvariant(t);
    t.push_back_child(2);
    checkInvariant(t);
    t.push_back_child(3);
    checkInvariant(t);
    t.push_back_sibling(4);
    checkInvariant(t);
    t.push_back_level(5, 1);
    checkInvariant(t);
    t.push_back_child(6);
    checkInvariant(t);

    QVERIFY(t.size() == 6);
    QVERIFY(t[0].has_children());
    QVERIFY(!t[0].is_leaf());
    QVERIFY(t[1].has_children());
    QVERIFY(t[2].is_leaf());
    QVERIFY(t[3].is_leaf());
    QVERIFY(t[4].has_children());
    QVERIFY(t[5].is_leaf());

    auto sum = std::accumulate(t.begin(), t.end(), 0u, [](auto s, auto n) { return s + n.data; });
    QVERIFY(sum == 21);

    auto leaf_count = std::accumulate(t.begin(), t.end(), 0u,
                                      [](auto s, auto n) { return s + (n.is_leaf() ? 1 : 0); });
    QVERIFY(leaf_count == 3);

    t.erase_leaf(t.begin() + 5);
    checkInvariant(t);
    QVERIFY(t.size() == 5);

    t.erase_leaf(t.begin() + 3);
    checkInvariant(t);
    QVERIFY(t.size() == 4);
}

void
BuilderTest::pushRootConstruction() {
    int_tree t;
    t.push_root(2);
    checkInvariant(t);
    t.push_root(1);
    checkInvariant(t);

    QVERIFY(t.size() == 2);
    QVERIFY(t[0].data == 1);
    QVERIFY(t[0].has_children());
    QVERIFY(t[1].data == 2);
    QVERIFY(t[1].is_leaf());

    t.erase_leaf(t.begin() + 1);
    checkInvariant(t);
    QVERIFY(t.size() == 1);
}

void
BuilderTest::subtree() {
    int_tree t;
    /* 1
     *  2    5
     *   3 4  6
     */
    t.push_root(1);
    t.push_back_child(2);
    t.push_back_child(3);
    t.push_back_sibling(4);
    t.push_back_level(5, 1);
    t.push_back_child(6);

    using std::begin;
    using std::end;

    auto st0 = vt::subtree<int_tree>(t.begin());
    auto st1 = vt::subtree<int_tree>(t.begin() + 1);
    auto st2 = vt::subtree<int_tree>(t.begin() + 2);

    QCOMPARE(std::distance(begin(st0), end(st0)), 5);
    QCOMPARE(std::distance(begin(st1), end(st1)), 2);
    QCOMPARE(std::distance(begin(st2), end(st2)), 0);

    auto leaf_count = std::accumulate(begin(st1), end(st1), 0,
                                      [](auto s, auto n) { return s + (n.is_leaf() ? 1 : 0); });
    QCOMPARE(leaf_count, 2);

    t.erase_subtree(st2);
    checkInvariant(t);
    QVERIFY(t.size() == 6);
    QVERIFY(t[1].has_children());
    QVERIFY(t[2].is_leaf());
    QCOMPARE(t[2].data, 3);

    t.erase_subtree(st1);
    checkInvariant(t);
    QVERIFY(t.size() == 4);
    QVERIFY(t[1].is_leaf());
    QCOMPARE(t[2].data, 5);

    t.erase_subtree(st0);
    checkInvariant(t);
    QVERIFY(t.size() == 1);
}

QTEST_APPLESS_MAIN(BuilderTest)

#include "tst_BuilderTest.moc"
