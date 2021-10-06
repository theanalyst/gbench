// ----------------------------------------------------------------------
// File: StringSplit.hh
// Author: Abhishek Lekshmanan - CERN
// ----------------------------------------------------------------------

/************************************************************************
 * EOS - the CERN Disk Storage System                                   *
 * Copyright (C) 2021 CERN/Switzerland                                  *
 *                                                                      *
 * This program is free software: you can redistribute it and/or modify *
 * it under the terms of the GNU General Public License as published by *
 * the Free Software Foundation, either version 3 of the License, or    *
 * (at your option) any later version.                                  *
 *                                                                      *
 * This program is distributed in the hope that it will be useful,      *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 * GNU General Public License for more details.                         *
 *                                                                      *
 * You should have received a copy of the GNU General Public License    *
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.*
 ************************************************************************/

#include<string_view>
#include<string>
#include<algorithm>

namespace eos::common {
namespace detail {

template <typename T, typename = void>
struct has_const_iter : std::false_type {};

template <typename T>
struct has_const_iter<T, std::void_t<decltype(std::declval<T>().cbegin(),
                                              std::declval<T>().cend())>>
  : std::true_type {};

template <typename T>
bool constexpr has_const_iter_v = has_const_iter<T>::value;
} // detail

template <typename str_type = std::string_view,
          typename delim_type = std::string_view>
class LazySplit{
public:
    LazySplit(str_type s, delim_type d) : str(s), delim(d) {}

class iterator {

public:
  // A base declaration of the underlying string type so that we don't have to
  // decay every time, this is to ensure that we correctly have a reference type
  // when we hold a const std::string&,
  using base_string_type = typename std::decay<str_type>::type;

  // Basic iterator definition member types
  using iterator_category = std::forward_iterator_tag;
  using value_type = str_type;
  using difference_type = std::string_view::difference_type; // basically std::ptrdiff_t
  using pointer = std::add_pointer_t<base_string_type>;
  using const_pointer = std::add_const_t<pointer>;
  using reference = std::add_lvalue_reference_t<base_string_type>;
  using const_reference = std::add_const_t<reference>;
  using size_type = std::string_view::size_type;

  iterator(str_type s, delim_type d): str(s), delim(d), segment(next(0)) {}
  iterator(size_type sz) : pos(sz) {}

  iterator& operator++() {
    segment = next(pos);
    return *this;
  }

  iterator operator++(int) {
    iterator curr = *this;
    segment = next(pos);
    return curr;
  }

  reference operator*() { return segment; }
  pointer operator->()  { return &segment; }

  friend bool operator==(const iterator& a, const iterator& b) {
    return a.segment == b.segment;
    // return a.segment.size() == b.segment.size()&&
    //  a.segment.data() == b.segment.data();
  }

  friend bool operator!=(const iterator& a, const iterator& b) {
    return !(a==b);
  }

private:
  // we need to collapse the reference here, hence we have to return by value
  template <typename T=delim_type,
            std::enable_if_t<std::is_same_v<T,char>,bool> = true>
  base_string_type next(size_type start_pos) {
    static_assert(std::is_same_v<delim_type,char>, "expected char!");
    // this loop is needed to advance past empty delims
    while (start_pos < str.size()) {
      pos = str.find_first_of(delim, start_pos);
      // check if we are at the end or at a delim
      if (pos != start_pos) {
        return str.substr(start_pos, pos - start_pos);
      }
      start_pos = pos + 1;
    }
    return {};
  }

  template <typename T=delim_type,
            typename = std::enable_if_t<detail::has_const_iter_v<T>>>
  base_string_type next(size_type start_pos) {
    while (start_pos < str.size()) {
      auto p = std::find_first_of(str.cbegin()+start_pos, str.cend(),
                                  delim.cbegin(), delim.cend());
      if (p != str.cbegin() + start_pos) {
        pos = std::distance(str.cbegin(), p);
        return str.substr(start_pos, pos - start_pos);
      }
      start_pos = pos + 1;
    }
    return {};
  }

  size_type pos {0};
  str_type str;
  delim_type delim;
  str_type segment;

};

  using const_iterator = iterator;
  iterator begin() const { return {str, delim}; }
  const_iterator cbegin() const { return {str, delim}; }

  iterator end() const { return { std::string::npos }; }
  const_iterator cend() const { return {std::string::npos }; }
private:
  str_type str;
  delim_type delim;
};

} // namespace eos::common
