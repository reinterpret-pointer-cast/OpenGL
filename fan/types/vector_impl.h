#define make_for_all(todo) \
	vec_t ret = 0; for (access_type_t i = 0; i < size(); ++i) { todo; } return ret
#define make_for_all_test1_noret(todo) for (access_type_t i = 0; i < size() && i < test0.size(); ++i) { todo; }
#define make_for_all_test1(todo) \
	vec_t ret = 0; for (access_type_t i = 0; i < size() && i < test0.size(); ++i) { todo; } return ret
#define make_for_all_test2(todo) \
	vec_t ret = 0; for (access_type_t i = 0; i < size() && i < test0.size() && i < test1.size(); ++i) { todo; } return ret

#define make_operator_const(arithmetic) \
template <typename T> \
requires (!std::is_arithmetic_v<T>) \
constexpr vec_t operator arithmetic(const T& test0) const \
{ \
	make_for_all_test1(ret[i] = (*this)[i] arithmetic test0[i]); \
} \
\
template <typename T> \
requires (std::is_arithmetic_v<T>)\
constexpr vec_t operator arithmetic(T v0) const \
{ \
	make_for_all(ret[i] = (*this)[i] arithmetic v0); \
}
#define make_operator_assign(arithmetic) \
template <typename T> \
requires (!std::is_arithmetic_v<T>) \
constexpr vec_t& operator CONCAT(arithmetic,=) (const T& test0) \
{ \
	make_for_all_test1_noret((*this)[i] CONCAT(arithmetic,=) test0[i]); \
  return *this; \
} \
\
template <typename T> \
requires (std::is_arithmetic_v<T>)\
constexpr vec_t operator CONCAT(arithmetic,=)(T v0) \
{ \
	make_for_all((*this)[i] CONCAT(arithmetic,=) v0); \
}

#define make_operator_comparison(comp) \
template <typename T> \
requires (!std::is_arithmetic_v<T>) \
constexpr bool operator comp(const T& rhs) const { \
    return (*this <=> rhs) comp 0; \
}\
template <typename T> \
requires (std::is_arithmetic_v<T>) \
constexpr bool operator comp(const T& rhs) const { \
    return (*this <=> rhs) comp 0;\
}

using value_type = value_type_t;

static constexpr access_type_t size() { return vec_n; }

vec_t() = default;
template <typename T>
requires std::is_arithmetic_v<T>
constexpr vec_t(T single_init) { for (access_type_t i = 0; i < vec_n; ++i) operator[](i) = single_init; } 
template<typename... Args>
requires ((std::is_arithmetic_v<std::remove_reference_t<Args>> && ...) &&
          sizeof...(Args) == size())
constexpr vec_t(Args&&...args) {
  access_type_t i = 0;
  ((operator[](i++) = args), ...);
}
template<typename... Args>
requires(
  (std::is_same_v<value_type_t, std::remove_reference_t<Args>> && ...) &&
  (!std::is_arithmetic_v<std::remove_reference_t<Args>> && ...) &&
  sizeof...(Args) == size()
  )
constexpr vec_t(Args&&...args) {
  access_type_t i = 0;
  ((operator[](i++) = args), ...);
}
template <typename T>
constexpr vec_t(const vec_t<T>& test0) { for (int i = 0; i < size(); ++i) operator[](i) = test0[i]; } 

constexpr std::partial_ordering operator<=>(const auto& rhs) const {
  for (access_type_t i = 0; i < std::min(size(), rhs.size()); ++i) {
      if (auto cmp = (*this)[i] <=> rhs[i]; cmp != 0) {
        return cmp;
      }
  }
  return size() <=> rhs.size();
}
template <typename T>
requires std::is_arithmetic_v<T>
constexpr std::partial_ordering operator<=>(const T& rhs) const {
  for (access_type_t i = 0; i < size(); ++i) {
      if (auto cmp = (*this)[i] <=> (value_type)rhs; cmp != 0) {
        return cmp;
      }
  }
  return size() <=> 1;
}

#define make_operators(arithmetic) \
  make_operator_const(arithmetic); \
  make_operator_assign(arithmetic)

constexpr vec_t operator-() const { make_for_all(ret[i] = -(*this)[i]); }
make_operators(-);  make_operator_comparison(==);
make_operators(+);  make_operator_comparison(!=);
make_operators(*);  make_operator_comparison(<);
make_operators(/);  make_operator_comparison(<=);
make_operators(%);  make_operator_comparison(>);
                    make_operator_comparison(>=);

#define __FAN_SWITCH_IDX(x, idx) case size() - (idx + 1): return x

constexpr value_type_t& operator[](access_type_t idx) { 
  switch (idx) {
    #if vec_n
    __FAN__FOREACH(__FAN_SWITCH_IDX, fan_coordinate(vec_n);)
      #else
    default: return *(value_type_t*)nullptr;
      #endif
  }
    // force crash with stackoverflow or gives error if idx is knowable at compiletime
  return operator[](idx);
}
constexpr value_type_t operator[](access_type_t idx) const {
  switch (idx) { 
    #if vec_n
    __FAN__FOREACH(__FAN_SWITCH_IDX, fan_coordinate(vec_n);)
    #else
    default: return *(value_type_t*)nullptr;
    #endif
  }
    // force crash with stackoverflow or gives error if idx is knowable at compiletime
  return operator[](idx);
}
#undef __FAN_SWITCH_IDX

constexpr auto begin() const { 
  return
  #if vec_n == 0 
    nullptr;
#else
    &x;
#endif
}
constexpr auto end() const { return begin() + size(); }
constexpr auto data() const { return begin(); }

constexpr auto begin() { return &operator[](0); }
constexpr auto end() { return begin() + size(); }
constexpr auto data() { return begin(); }

constexpr auto multiply() const { return std::accumulate(begin(), end(), 1, std::multiplies<value_type_t>()); }
constexpr auto floor() const { make_for_all(ret[i] = std::floor((*this)[i])); }
constexpr auto floor(auto value) const { make_for_all(ret[i] = std::floor((*this)[i] / value)); }
constexpr auto ceil() const { make_for_all(ret[i] = std::ceil((*this)[i])); }
constexpr auto round() const { make_for_all(ret[i] = std::round((*this)[i])); }
constexpr auto abs() const { make_for_all(ret[i] = std::abs((*this)[i])); }
constexpr auto min() const { return *std::min_element(begin(), end()); }
constexpr auto min(const auto& test0) const { make_for_all_test1(ret[i] = std::min((*this)[i], test0[i])); }
constexpr auto max() const { return *std::max_element(begin(), end()); }
constexpr auto max(const auto& test0) const { make_for_all_test1(ret[i] = std::max((*this)[i], test0[i])); }
constexpr void clamp(const vec_t& test0) { make_for_all_test1_noret((*this)[i] = fan::clamp((*this)[i], test0[0], (*this)[i])); }
constexpr auto clamp(auto min, auto max) const { make_for_all(ret[i] = std::clamp((*this)[i], min, max)); }
constexpr auto clamp(const auto& test0, const auto& test1) const { make_for_all_test2(ret[i] = std::clamp((*this)[i], test0[i], test1[i])); }
constexpr auto dot(const auto& test0) const { return fan::math::dot(*this, test0); }
// for cross product, its only for vec3 so make custom
constexpr auto length() const { return sqrt(dot(*this)); }
constexpr auto normalize() const { auto l = length(); if (l == 0) return vec_t(0); make_for_all(ret[i] = (*this)[i] / l); }
constexpr vec_t square_normalize() const { return *this / abs().max(); }
void from_string(const std::string& str) { std::stringstream ss(str); char ch; for (access_type_t i = 0; i < size(); ++i) ss >> ch >> (*this)[i]; }
constexpr std::string to_string(int precision = 2) const {
  std::string out("{");
  for (access_type_t i = 0; i < size() - 1; ++i) { out += fan::to_string((*this)[i], precision) + ", "; }
  if constexpr (size()) {
    out += fan::to_string((*this)[size() - 1], precision);
  }
  out += '}';
  return out;
}

friend std::ostream& operator<<(std::ostream& os, const vec_t& test0) { os << test0.to_string(); return os; }

#if vec_n
value_type_t fan_coordinate(vec_n);
#endif


#undef make_operator_comparison
#undef make_operator_const
#undef make_operators
#undef make_operator_assign
#undef vec_n
#undef vec_t
#undef make_for_all
#undef make_for_all_test1
#undef make_for_all_test2
#undef __FAN_PTR_EACH