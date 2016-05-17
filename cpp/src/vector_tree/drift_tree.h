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
#pragma once

#include <vector>
#include <utility>
#include <cstdint>
#include <stdexcept>
#include <cassert>

namespace vt {

template<typename _tree_t>
struct subtree;

// This is external to make it easier to construct a allocator
template<typename _data_t, typename _drift_t = size_t>
struct drift_node
{
        using data_t = _data_t;
        using drift_t = _drift_t;

        drift_node(drift_t drift, data_t data) noexcept
                : drift(drift), data(data) {}

        bool is_leaf() const noexcept { return drift != 0; }
        bool has_children() const noexcept { return drift == 0; }

        drift_t drift;
        data_t data;
};

/* 1
 *  2    5
 *   3 4  6
 *
 * <0,1> <0,2> <1,3> <2,4> <0,5> <3,6>
 */
/* 1
 *  x 2    5
 *     3 4  6
 *        y
 * <0,1> <1,x> <0,2> <1,3> <0,4> <3,y> <0,5> <3,6>
 */

/*!
 * Stores a free tree data structure in a vector
 * Tree levels are encoded as relative drifts in each node
 *
 * Invariants:
 * - sum of drifts == node count
 * - last node is always a leaf node
 * - all sub sequences from begin() are valid trees (missing the final drift)
 */
template< typename _data_t, typename _drift_t = size_t, typename _alloc_t = std::allocator<drift_node<_data_t, _drift_t>>>
struct drift_tree
{
        using data_t = _data_t;
        using drift_t = _drift_t;
        using node_t = drift_node<_data_t, _drift_t>;
        using vector_t = std::vector<node_t, _alloc_t>;

        using level_t = size_t;
        enum {
                DRIFT_CHILD = 0,
                DRIFT_SIBLING = 1
        };

        // delegate types to vector
        using value_type = typename vector_t::value_type;
        using allocator_type = typename vector_t::allocator_type;
        using size_type = typename vector_t::size_type;
        using difference_type = typename vector_t::difference_type;
        using reference = typename vector_t::reference;
        using const_reference = typename vector_t::const_reference;
        using pointer = typename vector_t::pointer;
        using const_pointer = typename vector_t::const_pointer;
        using iterator = typename vector_t::iterator;
        using const_iterator = typename vector_t::const_iterator;
        using reverse_iterator = typename vector_t::reverse_iterator;
        using const_reverse_iterator = typename vector_t::const_reverse_iterator;

        drift_tree(drift_tree other, const allocator_type& alloc)
                : vector_m(other.vector_m, alloc) {}

        explicit drift_tree(const allocator_type& alloc = allocator_type())
                : vector_m(alloc) {}

        drift_tree(const drift_tree&) = default;
        drift_tree(drift_tree&&) = default;
        ~drift_tree() = default;
        drift_tree& operator =(const drift_tree&) = default;
        drift_tree& operator =(drift_tree&&) = default;

        // delegate methods to vector
        template< class InputIt >
        void assign(InputIt first, InputIt last) { vector_m.assign(first, last); }

        auto get_allocator() const noexcept { return vector_m.get_allocator(); }

        reference at(size_type pos) { return vector_m.at(pos); }
        const_reference at(size_type pos) const { return vector_m.at(pos); }

        reference operator[](size_type pos) noexcept { return vector_m[pos]; }
        const_reference operator[](size_type pos) const noexcept { return vector_m[pos]; }

        reference front() noexcept { return vector_m.front(); }
        const_reference front() const noexcept { return vector_m.front(); }

        reference back() noexcept { return vector_m.back(); }
        const_reference back() const noexcept { return vector_m.back(); }

        auto begin() noexcept { return vector_m.begin(); }
        auto begin() const noexcept { return vector_m.begin(); }
        auto cbegin() const noexcept { return vector_m.cbegin(); }

        auto end() noexcept { return vector_m.end(); }
        auto end() const noexcept { return vector_m.end(); }
        auto cend() const noexcept { return vector_m.cend(); }

        auto rbegin() noexcept { return vector_m.rbegin(); }
        auto rbegin() const noexcept { return vector_m.rbegin(); }
        auto crbegin() const noexcept { return vector_m.crbegin(); }

        auto rend() noexcept { return vector_m.rend(); }
        auto rend() const noexcept { return vector_m.rend(); }
        auto crend() const noexcept { return vector_m.crend(); }

        bool empty() const noexcept { return vector_m.empty(); }

        auto size() const noexcept { return vector_m.size(); }
        auto max_size() const noexcept { return vector_m.max_size(); }
        auto capacity() const noexcept { return vector_m.capacity(); }

        void reserve(size_type new_cap) { return vector_m.reserve(new_cap); }
        void shrink_to_fit() { vector_m.shrink_to_fit(); }
        void clear() noexcept { vector_m.clear(); }

        // make the value the new root
        // HINT: Use this method for the first node!
        // O(n)  n = number of nodes already in the tree
        void push_root(data_t value) {
                auto drift = 0;
                vector_m.emplace(begin(), drift, value);
                back().drift += 1;
        }

        // append a node to the end with a drifted level
        // O(1) + potential reallocation of the vector
        void push_back_drifted(data_t data, drift_t back_drift) {
                assert(0 < size());
                assert(1 + back().drift > back_drift);
                auto drift = 1 + back().drift - back_drift;
                back().drift = back_drift;
                vector_m.emplace_back(drift, data);
        }

        void push_back_child(data_t data) {
                push_back_drifted(data, DRIFT_CHILD);
        }

        void push_back_sibling(data_t data) {
                push_back_drifted(data, DRIFT_SIBLING);
        }

        // append a node at a specific level
        // O(1) + potential reallocation of the vector
        void push_back_level(data_t data, level_t level) {
                assert(0 < size());
                assert(back().drift > level);
                back().drift -= level;
                vector_m.emplace_back(1 + level, data);
        }

        // remove the last node
        void pop_back() noexcept {
                assert(1 < size());
                auto drift = back().drift - 1;
                vector_m.pop_back();
                back().drift += drift;
        }

        // add a node as the first child of i position
        // O(n)  n = nodes behind the iterator
        iterator insert_first_child(iterator i, data_t data) {
                assert(end() != i);
                auto drift = 1 + i->drift;
                i->drift = 0;
                return vector_m.emplace(i+1, drift, data);
        }

        // add a subtree as the first child of i position
        // O(n+m)  n = nodes behind the iterator
        //         m = nodes inserted
        template< class InputIt >
        iterator insert_child_tree(iterator i, InputIt first, InputIt last) {
                assert(end() != i);
                auto old_count = size();
                auto next = vector_m.insert(i+1, first, last);
                auto inserted = size() - old_count;
                if (0 < inserted) {
                        i = next - 1;
                        auto drift = 1 + i->drift;
                        i->drift = 0;
                        while(--inserted) drift += 1 - (next++)->drift;
                        next->drift = drift;
                }
                return next;
        }

        // add left sibling before the node at i position
        // O(n)  n = nodes behind iterator
        iterator insert_sibling(const_iterator i, data_t data) {
                assert(i != end());
                auto drift = 1;
                return vector_m.emplace(i, drift, data);
        }

        // removes a leaf node of the vector
        // O(n)  n = nodes behind iterator
        iterator erase_leaf(iterator i) {
                assert(i != end());
                assert(i->is_leaf());
                auto drift = i->drift - 1;
                (i-1)->drift += drift;
                return vector_m.erase(i);
        }

        // removes the subtree of all children of node at i position
        iterator erase_subtree(subtree<drift_tree> st);

private:
        vector_t vector_m;
};

template<typename _tree_t>
struct subtree {
        using tree_t = _tree_t;
        using level_t = typename tree_t::level_t;
        using node_t = typename tree_t::value_type;
        using iterator_t = typename tree_t::iterator;

        struct iterator : public std::iterator< std::forward_iterator_tag, typename _tree_t::value_type>
        {
                explicit iterator(iterator_t it) noexcept
                        : it_m(it + 1), level_m(it->drift == 0 ? 1 : 0) {
                }

                iterator() = default;
                iterator(const iterator&) = default;
                iterator(iterator&&) = default;
                ~iterator() = default;
                iterator& operator =(const iterator&) = default;
                iterator& operator =(iterator&&) = default;

                static iterator end() noexcept { return iterator(); }

                bool is_end() const noexcept {
                        return level_m == 0;
                }

                bool operator == (const iterator &ot) const noexcept {
                        if (this->is_end() || ot.is_end())
                                return level_m == ot.level_m;
                        return it_m == ot.it_m;
                }
                bool operator != (const iterator &ot) const noexcept {
                        return ! (*this == ot);
                }

                auto operator*() const noexcept {
                        return static_cast<const node_t&>(*it_m);
                }
                auto operator*() noexcept {
                        return static_cast<node_t&>(*it_m);
                }

                auto operator++() noexcept {
                        level_m += 1;
                        if (level_m < it_m->drift)
                                level_m = 0;
                        else
                                level_m -= it_m->drift;
                        it_m++;
                        return static_cast<iterator&>(*this);
                }

                auto operator++(int) noexcept {
                        auto __tmp = *this;
                        ++(*this);
                        return __tmp;
                }

                auto unwrap() const noexcept { return it_m; }
                auto level() const noexcept { return level_m; }

        private:
                iterator_t it_m = {};
                level_t level_m = {};
        };

        explicit subtree(iterator_t it) noexcept
                : it_m(it) {}

        auto begin() const noexcept { return iterator(it_m); }
        auto end() const noexcept { return iterator::end(); }

        auto unwrap() const noexcept { return it_m; }

private:
        iterator_t it_m = {};
};

template< typename _data_t, typename _drift_t, typename _alloc_t>
typename drift_tree<_data_t, _drift_t, _alloc_t>::iterator
drift_tree<_data_t, _drift_t, _alloc_t>::erase_subtree(subtree<drift_tree<_data_t, _drift_t, _alloc_t>> st)
{
        using std::begin;
        using std::end;
        auto st_it = begin(st);
        auto vec_begin = st_it.unwrap();
        level_t level = st_it.level();
        while(st_it != end(st)) {
                level = st_it.level();
                ++st_it;
        }
        auto vec_end = st_it.unwrap();
        auto drift = (vec_end - 1)->drift - level;
        st.unwrap()->drift = drift;
        return vector_m.erase(vec_begin, vec_end);
}

} // namespace vt
