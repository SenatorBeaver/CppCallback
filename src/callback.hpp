#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <utility>

namespace helpers
{
    template <typename ARRAY> struct ArraySize;
    template <typename T, std::size_t N> struct ArraySize<std::array<T, N>>
    {
        static constexpr std::size_t value = N;
    };
} // namespace helpers

template <std::size_t StorageSize, typename ErrorPolicy, typename Signature> class Callback;
template <std::size_t StorageSize, typename ErrorPolicy, typename Ret, typename... Args>
class Callback<StorageSize, ErrorPolicy, Ret(Args...)>
{
  public:
    using FuncType = Ret (*)(void*, Args...);

    constexpr Callback() = default;

    constexpr Callback(std::nullptr_t) noexcept
        : closureStorage_{}, objPtr_{nullptr}, funcPtr_{nullptr}, storingClosure_{false}
    {}
    constexpr Callback& operator=(std::nullptr_t) noexcept
    {
        objPtr_         = nullptr;
        funcPtr_        = nullptr;
        storingClosure_ = false;
        return *this;
    }

    constexpr Callback(const Callback& another) noexcept
        : closureStorage_{another.closureStorage_}, objPtr_{&closureStorage_}, funcPtr_{another.funcPtr_},
          storingClosure_{another.storingClosure_}
    {}
    constexpr Callback& operator=(const Callback& another) noexcept
    {
        closureStorage_ = another.closureStorage_;
        objPtr_         = &closureStorage_;
        funcPtr_        = another.funcPtr_;
        storingClosure_ = another.storingClosure_;
        return *this;
    }

    constexpr Callback(Callback&& another) noexcept
        : closureStorage_{std::move(another.closureStorage_)}, objPtr_{&closureStorage_},
          funcPtr_{std::move(another.funcPtr_)}, storingClosure_{std::exchange(another.storingClosure_, false)}
    {}

    constexpr Callback& operator=(Callback&& another) noexcept
    {
        closureStorage_ = std::move(another.closureStorage_);
        objPtr_         = &closureStorage_;
        funcPtr_        = std::move(another.funcPtr_);
        storingClosure_ = std::exchange(another.storingClosure_, false);
        return *this;
    }

    template <typename Closure, class DecayClosure = std::decay_t<Closure>>
        requires(std::is_invocable_r_v<Ret, DecayClosure&, Args...> && !std::is_same_v<Callback, DecayClosure>)
    constexpr Callback(Closure&& closure)
        : objPtr_(new(closureStorage_.data()) DecayClosure(std::forward<Closure>(closure))),
          funcPtr_(&ClosureWrapper<DecayClosure>), storingClosure_{true}
    {
        static_assert(sizeof(closure) <= helpers::ArraySize<decltype(closureStorage_)>::value,
                      "Too big object, change storage size");
        static_assert(std::is_copy_constructible_v<DecayClosure>, "Closure should be copy constructible");
    }

    template <typename T, Ret (T::*ptrToMemFun)(Args... args)>
    static constexpr Callback<StorageSize, ErrorPolicy, Ret(Args...)> Create(T& instance)
    {
        Callback<StorageSize, ErrorPolicy, Ret(Args...)> d;
        d.Bind<T, ptrToMemFun>(instance);
        return d;
    }

    template <typename T, Ret (T::*ptrToMemFun)(Args... args) const>
    static constexpr Callback<StorageSize, ErrorPolicy, Ret(Args...)> Create(T& instance)
    {
        Callback<StorageSize, ErrorPolicy, Ret(Args...)> d;
        d.Bind<T, ptrToMemFun>(instance);
        return d;
    }

    template <typename Closure, typename DecayClosure = std::decay_t<Closure>> void Bind(Closure&& closure)
    {
        static_assert(sizeof(closure) <= helpers::ArraySize<decltype(closureStorage_)>::value,
                      "Too big object, change storage size");
        static_assert(std::is_invocable_r_v<Ret, DecayClosure&, Args...>, "Incompatible type");
        static_assert(std::is_copy_constructible_v<DecayClosure>, "Closure should be copy constructible");
        funcPtr_        = &ClosureWrapper<DecayClosure>;
        objPtr_         = new (closureStorage_.data()) Closure(closure);
        storingClosure_ = true;
    }

    template <typename T, Ret (T::*ptrToMemFun)(Args... args)> void Bind(T& instance)
    {
        funcPtr_        = &MemberWrapper<T, ptrToMemFun>;
        objPtr_         = &instance;
        storingClosure_ = false;
    }

    template <typename T, Ret (T::*ptrToMemFun)(Args... args) const> void Bind(T& instance)
    {
        funcPtr_        = &ConstMemberWrapper<T, ptrToMemFun>;
        objPtr_         = &instance;
        storingClosure_ = false;
    }

    Ret operator()(Args... args) const
        requires(std::is_same_v<Ret, void>)
    {
        if (funcPtr_) {
            if constexpr (std::is_same_v<Ret, void>) {
                (*funcPtr_)(objPtr_, std::forward<Args>(args)...);
            }
            else {
                return (*funcPtr_)(objPtr_, std::forward<Args>(args)...);
            }
        }

        ErrorPolicy::OnBadCall();
    }

    bool IsEmpty() const
    {
        return funcPtr_ == nullptr;
    }

    operator bool() const
    {
        return !IsEmpty();
    }

    void Clear()
    {
        funcPtr_ = nullptr;
    }

  private:
    alignas(alignof(std::max_align_t)) std::array<uint8_t, StorageSize> closureStorage_{};
    void* objPtr_{nullptr};
    FuncType funcPtr_{nullptr};
    bool storingClosure_{false};

    template <typename Closure> static inline Ret ClosureWrapper(void* object, Args... args)
    {
        Closure* const p = static_cast<Closure*>(object);
        return (*p)(std::forward<Args>(args)...);
    }

    template <typename T, Ret (T::*memberFunction)(Args...)> static inline Ret MemberWrapper(void* object, Args... args)
    {
        T* const p = static_cast<T*>(object);
        return (p->*memberFunction)(std::forward<Args>(args)...);
    }

    template <typename T, Ret (T::*memberFunction)(Args...) const>
    static inline Ret ConstMemberWrapper(void* object, Args... args)
    {
        T* const p = static_cast<T*>(object);
        return (p->*memberFunction)(std::forward<Args>(args)...);
    }
};
