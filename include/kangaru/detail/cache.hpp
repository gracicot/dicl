#ifndef KANGARU5_DETAIL_CACHE_HPP
#define KANGARU5_DETAIL_CACHE_HPP

#include "utility.hpp"
#include "source.hpp"
#include "ctti.hpp"
#include "concepts.hpp"
#include "cache_types.hpp"

#include <type_traits>
#include <concepts>
#include <utility>

#include "define.hpp"

namespace kangaru {
	template<
		source Source,
		cache_map Cache = std::unordered_map<std::size_t, void*>
	>
	struct with_cache {
		using source_type = Source;
		using cache_type = Cache;

	private:
		using unwrapped_cache_type = maybe_wrapped_t<cache_type>;
	
	public:
		explicit constexpr with_cache(source_type source) noexcept
		requires (
			std::default_initializable<cache_type>
		) : source{std::move(source)} {}
		
		constexpr with_cache(source_type source, cache_type cache) noexcept
			: source{std::move(source)}, cache{std::move(cache)} {}
		
		using key_type = typename unwrapped_cache_type::key_type;
		using value_type = typename unwrapped_cache_type::value_type;
		using mapped_type = typename unwrapped_cache_type::mapped_type;
		using iterator = typename unwrapped_cache_type::iterator;
		using const_iterator = typename unwrapped_cache_type::const_iterator;
		
		source_type source;
		
		constexpr auto insert(auto&& value) requires requires(unwrapped_cache_type c) { c.insert(KANGARU5_FWD(value)); } {
			return kangaru::maybe_unwrap(cache).insert(KANGARU5_FWD(value));
		}
		
		template<typename It>
		constexpr auto insert(It begin, It end) requires requires(unwrapped_cache_type c) { c.insert(begin, end); } {
			return kangaru::maybe_unwrap(cache).insert(begin, end);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) requires requires(unwrapped_cache_type c) { c.find(key); } {
			return kangaru::maybe_unwrap(cache).find(key);
		}
		
		[[nodiscard]]
		constexpr auto find(auto const& key) const requires requires(unwrapped_cache_type const c) { c.find(key); } {
			return kangaru::maybe_unwrap(cache).find(key);
		}
		
		[[nodiscard]]
		constexpr auto contains(auto const& key) const requires requires(unwrapped_cache_type c) { c.contains(key); } {
			return kangaru::maybe_unwrap(cache).contains(key);
		}
		
		[[nodiscard]]
		constexpr auto begin() -> typename unwrapped_cache_type::iterator {
			return kangaru::maybe_unwrap(cache).begin();
		}
		
		[[nodiscard]]
		constexpr auto end() -> typename unwrapped_cache_type::iterator {
			return kangaru::maybe_unwrap(cache).end();
		}
		
		[[nodiscard]]
		constexpr auto begin() const {
			return kangaru::maybe_unwrap(cache).begin();
		}
		
		[[nodiscard]]
		constexpr auto end() const {
			return kangaru::maybe_unwrap(cache).end();
		}
		
		[[nodiscard]]
		constexpr auto cbegin() const -> typename unwrapped_cache_type::const_iterator {
			return kangaru::maybe_unwrap(cache).cbegin();
		}
		
		[[nodiscard]]
		constexpr auto cend() const -> typename unwrapped_cache_type::const_iterator {
			return kangaru::maybe_unwrap(cache).cend();
		}
		
		[[nodiscard]]
		constexpr auto empty() const -> bool {
			return kangaru::maybe_unwrap(cache).empty();
		}
		
		constexpr auto clear() -> void {
			return kangaru::maybe_unwrap(cache).clear();
		}
		
		[[nodiscard]]
		constexpr auto size() const {
			return kangaru::maybe_unwrap(cache).size();
		}
		
		[[nodiscard]]
		constexpr auto erase(auto const& key) requires requires(unwrapped_cache_type c) { c.erase(key); } {
			return kangaru::maybe_unwrap(cache).erase(key);
		}
		
		constexpr auto swap(with_cache& other) noexcept -> void {
			using std::swap;
			swap(source, other.source);
			swap(cache, other.cache);
		}
		
	private:
		template<object T, forwarded<with_cache> Self>
			requires source_of<detail::utility::forward_like_t<Self, source_type>, T>
		friend constexpr auto provide(provide_tag<T> tag, Self&& source) -> T {
			constexpr auto id = detail::ctti::type_id_for<T>();
			auto const it = kangaru::maybe_unwrap(source.cache).find(id);
			
			if (it == kangaru::maybe_unwrap(source.cache).end()) {
				auto object = provide(tag, KANGARU5_FWD(source).source);
				auto const [it, _] = kangaru::maybe_unwrap(source.cache).insert(std::pair{id, std::move(object)});
				return static_cast<T>(it->second);
			} else {
				return static_cast<T>(it->second);
			}
		}
		
		cache_type cache = {};
	};
	
	template<typename Source, typename Cache = std::unordered_map<std::size_t, void*>>
		requires(source<std::remove_cvref_t<Source>> and cache_map<std::remove_cvref_t<Cache>>)
	constexpr auto make_source_with_cache(Source&& source, Cache&& cache = std::unordered_map<std::size_t, void*>{}) {
		return with_cache<std::remove_cvref_t<Source>, std::remove_cvref_t<Cache>>{KANGARU5_FWD(source), KANGARU5_FWD(cache)};
	}
	
	static_assert(cache_map<with_cache<noop_source>>);
	static_assert(cache_map<source_reference_wrapper<with_cache<with_cache<noop_source>>>>);
	static_assert(cache_map<with_cache<noop_source, source_reference_wrapper<with_cache<noop_source>>>>);
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CACHE_HPP