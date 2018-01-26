#pragma once

#include <any>
#include <array>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <type_traits>

namespace cdif
{
    template <typename T>
    struct remove_cvref
    {
        using type = std::remove_cv_t<std::remove_reference_t<T>>;
    };

    template <typename T>
    using remove_cvref_t = typename remove_cvref<T>::type;

    template <typename T>
    struct remove_cvref_pointer
    {
        using type = remove_cvref_t<std::remove_pointer_t<T>>;
    };

    template <typename T>
    using remove_cvref_pointer_t = typename remove_cvref_pointer<T>::type;

    template <typename T, template <typename> typename C>
    struct is_ptr_type
    {
        template <typename U, template <typename> typename X>
        static decltype(std::is_same<U, X<typename U::element_type>>{}) matches(std::remove_reference<U>*);

        template <typename U, template <typename> typename X>
        static std::false_type matches(...);

        using type = decltype(matches<remove_cvref_t<T>, C>(nullptr));

        static constexpr bool value = { type::value };
    };

    template <typename T, template <typename> typename C>
    inline constexpr bool is_ptr_type_v = is_ptr_type<T, C>::value;

    template <typename T>
    inline constexpr bool is_shared_ptr = is_ptr_type_v<T, std::shared_ptr>;

    template <typename T>
    inline constexpr bool is_unique_ptr = is_ptr_type_v<T, std::unique_ptr>;

    template <typename T>
    inline constexpr bool is_smart_ptr = is_shared_ptr<T> || is_unique_ptr<T>;

    template <typename T>
    inline constexpr bool is_any_pointer_type =
        std::is_pointer_v<T> || is_smart_ptr<T>;

    template <typename T>
    struct remove_smart_ptr
    {
        using type = T;
    };

    template <typename T>
    struct remove_smart_ptr<std::shared_ptr<T>>
    {
        using type = T;
    };

    template <typename T>
    struct remove_smart_ptr<std::unique_ptr<T>>
    {
        using type = T;
    };

    template <typename T>
    using remove_smart_ptr_t = typename remove_smart_ptr<T>::type;

    template <typename T>
    struct get_base_type
    {
        using type = remove_smart_ptr_t<remove_cvref_pointer_t<T>>;
    };

    template <typename T>
    inline constexpr bool is_singleton_type =
        std::is_pointer_v<remove_cvref_t<T>> ||
        is_shared_ptr<T> ||
        std::is_reference_v<T>;

    template <typename TLeft, typename TRight, size_t Index>
    static constexpr std::optional<size_t> is_same_optional()
    {
        if constexpr (std::is_same_v<remove_cvref_t<TLeft>, remove_cvref_t<TRight>>)
            return Index;
        return {};
    }

    template <typename TValue, typename ... Ts, size_t ... Indices, size_t Size = sizeof...(Ts)>
    static constexpr std::array<std::optional<size_t>, Size> matching_indices(std::index_sequence<Indices...>)
    {
        return { is_same_optional<TValue, Ts, Indices>()... };
    }

    template <size_t Index, typename TValue, typename ... Ts, typename Indices = std::make_index_sequence<sizeof...(Ts)>>
    static constexpr bool type_at_index_matches()
    {
        return static_cast<bool>(matching_indices<TValue, Ts...>(Indices{})[Index]);
    }

    template <typename T, typename ... TArgs>
    struct has_constructor_with_args
    {
        template <typename Tc, typename ... Tas>
        static decltype(Tc {std::declval<Tas>()...}, std::true_type{}) hasCtor(std::remove_reference<Tc>*);

        template <typename Tc, typename ... Tas>
        static std::false_type hasCtor(...);

        using type = decltype(hasCtor<T, TArgs...>(nullptr));

        static constexpr bool value = { type::value };
    };

    template <typename TDep, size_t Index, size_t FuncCount>
    static const std::function<TDep (const Container&)> getResolverAtIndex(
        const std::array<std::function<std::any (const cdif::Container&)>, FuncCount>& resolvers)
    {
        auto func = resolvers[Index];
        return [func] (const cdif::Container& ctx) { return std::any_cast<TDep>(func(ctx)); };
    }
}
