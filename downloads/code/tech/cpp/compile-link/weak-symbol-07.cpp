// C++ 中天然的弱符号：inline 函数 / 模板实例化
template <typename T>
T add(T a, T b) { return a + b; }

inline int twice(int x) { return x * 2; }

int use() { return add(1, 2) + add(1.5, 2.5) + twice(3); }
