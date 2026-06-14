#include <iostream>
#include <atomic>
#include <functional>
#include <stdexcept>

template<typename T>
class SimpleSharedPtr;  // 先声明

template<typename T>
class SimpleWeakPtr;

template<typename T>
class ControlBlock {
public:
    std::atomic<int> shared_count;
    std::atomic<int> weak_count;
    T* ptr;
    std::function<void(T*)> deleter;

    // 每个shared_ptr自持一个weak count
    ControlBlock(T* p, std::function<void(T*)> d)
        : shared_count(1), weak_count(1), ptr(p), deleter(d ? d : [](T* p){ delete p; }) {}

    ~ControlBlock() = default;
};

template<typename T>
class SimpleSharedPtr {
private:
    ControlBlock<T>* ctrl;

    void release() {
        if (ctrl) {
            // 减shared_count
            if (ctrl->shared_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                // 最后一个shared_ptr，释放资源
                try {
                    ctrl->deleter(ctrl->ptr);
                } catch (...) {
                    std::cerr << "Exception in deleter\n";
                }
                ctrl->ptr = nullptr;

                // 再减少weak_count，自己占有一个weak计数，减少它
                if (ctrl->weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                    delete ctrl;
                }
            }
            ctrl = nullptr;
        }
    }

public:
    explicit SimpleSharedPtr(T* p = nullptr, std::function<void(T*)> d = nullptr) {
        if (p) {
            ctrl = new ControlBlock<T>(p, d);
            // shared_count=1, weak_count=0
        } else {
            ctrl = nullptr;
        }
    }

    // 拷贝构造
    SimpleSharedPtr(const SimpleSharedPtr& other) : ctrl(other.ctrl) {
        if (ctrl) {
            ctrl->shared_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 移动构造
    SimpleSharedPtr(SimpleSharedPtr&& other) noexcept : ctrl(other.ctrl) {
        other.ctrl = nullptr;
    }

    // 析构
    ~SimpleSharedPtr() {
        release();
    }

    // 拷贝赋值
    SimpleSharedPtr& operator=(const SimpleSharedPtr& other) {
        if (this != &other) {
            release();
            ctrl = other.ctrl;
            if (ctrl) {
                ctrl->shared_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }

    // 移动赋值
    SimpleSharedPtr& operator=(SimpleSharedPtr&& other) noexcept {
        if (this != &other) {
            release();
            ctrl = other.ctrl;
            other.ctrl = nullptr;
        }
        return *this;
    }

    T* get() const {
        return ctrl ? ctrl->ptr : nullptr;
    }

    T& operator*() const {
        if (!get()) throw std::runtime_error("Dereferencing null pointer");
        return *(ctrl->ptr);
    }

    T* operator->() const {
        return get();
    }

    int use_count() const {
        return ctrl ? ctrl->shared_count.load(std::memory_order_relaxed) : 0;
    }

    bool unique() const {
        return use_count() == 1;
    }

    explicit operator bool() const {
        return get() != nullptr;
    }

    void reset(T* p = nullptr, std::function<void(T*)> d = nullptr) {
        release();
        if (p) {
            ctrl = new ControlBlock<T>(p, d);
        } else {
            ctrl = nullptr;
        }
    }

    // 给 SimpleWeakPtr 访问 ControlBlock 指针权限
    friend class SimpleWeakPtr<T>;

private:
    // SimpleSharedPtr增加一个私有构造，能从控制块构造shared_ptr（配合weak_ptr的lock）
    SimpleSharedPtr(ControlBlock<T>* c) : ctrl(c) {}
};

template<typename T>
class SimpleWeakPtr {
private:
    ControlBlock<T>* ctrl;

    void release() {
        if (ctrl) {
            if (ctrl->weak_count.fetch_sub(1, std::memory_order_acq_rel) == 1) {
                // 只有弱引用且没有shared_ptr存在时，销毁控制块
                if (ctrl->shared_count.load(std::memory_order_acquire) == 0) {
                    delete ctrl;
                }
            }
            ctrl = nullptr;
        }
    }

public:
    SimpleWeakPtr() : ctrl(nullptr) {}

    // 从 shared_ptr 构造 weak_ptr
    SimpleWeakPtr(const SimpleSharedPtr<T>& shared) : ctrl(shared.ctrl) {
        if (ctrl) {
            ctrl->weak_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 拷贝构造
    SimpleWeakPtr(const SimpleWeakPtr& other) : ctrl(other.ctrl) {
        if (ctrl) {
            ctrl->weak_count.fetch_add(1, std::memory_order_relaxed);
        }
    }

    // 移动构造
    SimpleWeakPtr(SimpleWeakPtr&& other) noexcept : ctrl(other.ctrl) {
        other.ctrl = nullptr;
    }

    ~SimpleWeakPtr() {
        release();
    }

    // 拷贝赋值
    SimpleWeakPtr& operator=(const SimpleWeakPtr& other) {
        if (this != &other) {
            release();
            ctrl = other.ctrl;
            if (ctrl) {
                ctrl->weak_count.fetch_add(1, std::memory_order_relaxed);
            }
        }
        return *this;
    }

    // 移动赋值
    SimpleWeakPtr& operator=(SimpleWeakPtr&& other) noexcept {
        if (this != &other) {
            release();
            ctrl = other.ctrl;
            other.ctrl = nullptr;
        }
        return *this;
    }

    int use_count() const {
        return ctrl ? ctrl->shared_count.load(std::memory_order_relaxed) : 0;
    }

    bool expired() const {
        return use_count() == 0;
    }

    // 尝试获取shared_ptr，资源已被销毁则返回空shared_ptr
    SimpleSharedPtr<T> lock() const {
        if (expired()) {
            return SimpleSharedPtr<T>();
        } else {
            // 尝试增加shared_count
            int old_count = ctrl->shared_count.load(std::memory_order_relaxed);
            while (old_count != 0) {
                if (ctrl->shared_count.compare_exchange_weak(old_count, old_count + 1,
                    std::memory_order_acq_rel, std::memory_order_relaxed)) {
                    // 成功增加shared_count
                    return SimpleSharedPtr<T>(ctrl);
                }
                // old_count 已被更新，继续循环
            }
            return SimpleSharedPtr<T>();
        }
    }

private:
    // 让 SimpleSharedPtr 可以通过控制块构造 shared_ptr
    friend class SimpleSharedPtr<T>;

    explicit SimpleWeakPtr(ControlBlock<T>* c) : ctrl(c) {}
};


int main() {
    SimpleSharedPtr<int> sp1(new int(42));
    SimpleWeakPtr<int> wp1 = sp1;

    std::cout << "use_count: " << sp1.use_count() << "\n"; // 1
    std::cout << "expired: " << wp1.expired() << "\n";    // 0

    if (auto sp2 = wp1.lock()) {
        std::cout << "locked value: " << *sp2 << "\n";    // 42
        std::cout << "use_count after lock: " << sp2.use_count() << "\n"; // 2
    }

    sp1.reset();

    std::cout << "expired after reset: " << wp1.expired() << "\n"; // 1

    if (auto sp3 = wp1.lock()) {
        std::cout << "Should not print\n";
    } else {
        std::cout << "lock failed, resource expired\n";
    }

    return 0;
}
