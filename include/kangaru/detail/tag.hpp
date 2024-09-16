#ifndef KANGARU5_DETAIL_TAG_HPP
#define KANGARU5_DETAIL_TAG_HPP

#include "source.hpp"
#include "concepts.hpp"
#include "utility.hpp"
#include <type_traits>

namespace kangaru {
	template<typename>
	struct tag_for {};
	
	struct tags_tag {};
	
	template<typename T>
	inline constexpr auto tag_for_v = tag_for<T>{};
	
	template<typename T>
	concept weak_tag = requires {
		requires std::same_as<tags_tag, typename T::meta>;
	};
	
	namespace detail::tag {
		auto tag(auto&&) -> void requires false;
		
		template<typename T>
		concept adl_nonmember_tag =
			requires(tag_for<T> t) {
				{ tag(t) } -> kangaru::weak_tag;
			};
		
		template<typename T>
		struct tag_function {};
		
		template<adl_nonmember_tag T>
		struct tag_function<T> {
			using type = decltype(tag(tag_for_v<T>));
		};
		
		template<typename T>
		using tag_function_t = typename tag_function<T>::type;
	}
	
	template<typename T>
	struct tags_of {};
	
	template<detail::tag::adl_nonmember_tag T>
	struct tags_of<T> {
		using type = detail::tag::tag_function_t<T>;
	};
	
	template<typename T>
	using tags_of_t = typename tags_of<T>::type;
	
	template<typename T>
	concept provides_tags = weak_tag<tags_of_t<T>>;
	 
	template<typename T>
	concept tag_empty_injection_constructible =
		    provides_tags<T>
		and requires {
			requires std::same_as<typename tags_of_t<T>::allow_empty_injection, std::true_type>;
		};
	
	template<typename T>
	inline constexpr auto is_empty_injection_constructible_v = tag_empty_injection_constructible<T>;
	
	template<typename T>
	concept tag_overrides_types_in_cache =
		    provides_tags<T>
		and requires {
			typename tags_of_t<T>::overrides_types_in_cache;
			{ detail::utility::decay_copy(std::tuple_size<typename tags_of_t<T>::overrides_types_in_cache>::value) } -> std::same_as<std::size_t>;
		};
	
	template<typename>
	struct overrides_types_in_cache {
		using type = std::tuple<>;
	};
	
	template<tag_overrides_types_in_cache T>
	struct overrides_types_in_cache<T> {
		using type = typename tags_of_t<T>::overrides_types_in_cache;
	};
	
	template<typename T>
	using overrides_types_in_cache_t = typename overrides_types_in_cache<T>::type;
	
	struct empty_injectable {
		using meta = tags_tag;
		using allow_empty_injection = std::true_type;
	};
	
	struct cached {
		using meta = tags_tag;
		using allow_runtime_caching = std::true_type;
	};
	
	template<typename T>
	inline constexpr auto is_cachable_v = tag_empty_injection_constructible<T>;
	
	template<typename... Ts> requires (sizeof...(Ts) > 0)
	struct overrides {
		using meta = tags_tag;
		
		// TODO: Use meta list
		using overrides_types_in_cache = std::tuple<Ts...>;
	};
	
	template<weak_tag... Tags>
	struct tags : Tags... {
		using meta = tags_tag;
	};
	
	template<typename T>
	concept tag_cache_using_source_base =
		    provides_tags<T>
		and requires {
			typename tags_of_t<T>::cache_using_source;
		};
	
	template<typename T>
	concept tag_cache_using_source_template =
		    tag_cache_using_source_base<T>
		and requires {
			typename tags_of_t<T>::cache_using_source::template source_for<T>;
			requires source_of<typename tags_of_t<T>::cache_using_source::template source_for<T>, T>;
		};
	
	template<typename T>
	concept tag_cache_using_source_type =
		    tag_cache_using_source_base<T>
		and requires {
			typename tags_of_t<T>::cache_using_source::source;
			requires source_of<typename tags_of_t<T>::cache_using_source::source, T>;
		};
	
	template<typename T>
	concept tag_cache_using_source = tag_cache_using_source_type<T> or tag_cache_using_source_template<T>;
	
	template<tag_cache_using_source T>
	struct cache_using_source {};
	
	template<tag_cache_using_source_type T>
	struct cache_using_source<T> {
		using type = typename tags_of_t<T>::cache_using_source::source;
	};
	
	template<tag_cache_using_source_template T>
	struct cache_using_source<T> {
		using type = typename tags_of_t<T>::cache_using_source::template source_for<T>;
	};
	
	template<tag_cache_using_source T>
	using cache_using_source_t = typename cache_using_source<T>::type;
	
	template<typename T>
	struct cache_using {
		using meta = tags_tag;
		struct cache_using_source {
			using source = T;
		};
	};
	
	template<template<typename> typename Source>
	struct cache_using_source_type {
		using meta = tags_tag;
		struct cache_using_source {
			template<typename T>
			using source_for = Source<std::remove_cvref_t<T>>;
		};
	};
}

#endif // KANGARU5_DETAIL_TAG_HPP