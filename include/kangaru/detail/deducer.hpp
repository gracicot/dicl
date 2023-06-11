#ifndef KANGARU_DETAIL_DEDUCER_HPP
#define KANGARU_DETAIL_DEDUCER_HPP

#include "concepts.hpp"
#include "source.hpp"
#include "type_traits.hpp"
#include "utility.hpp"

#include <utility>
#include <type_traits>

#include "define.hpp"

struct Scene;
struct Camera;

namespace kangaru {
	struct kangaru_deducer_tag {};
	
	template<typename Deducer>
	concept deducer =
		    object<Deducer>
		and requires {
			requires std::same_as<typename Deducer::is_deducer, kangaru_deducer_tag>;
		};
	
	template<typename Deducer, typename T>
	concept deducer_for = deducer<Deducer> and requires(Deducer deducer) {
		{ deducer.operator T() } -> std::same_as<T>;
	};

	template<typename T>
	concept deducible = unqualified_object<T> and not deducer<T>;
	
	template<typename T, typename Source>
	concept deducible_prvalue = deducible<T> and source_of<Source, T>;
	
	template<typename T, typename Source>
	concept deducible_lvalue = deducible<T> and source_of<Source, T&>;
	
	template<typename T, typename Source>
	concept deducible_const_lvalue = deducible<T> and (
		   source_of<Source, T const&>
		or source_of<Source, T&>
		or source_of<Source, T const&&>
		or source_of<Source, T&&>
	);

	template<typename T, typename Source>
	concept deducible_rvalue = deducible<T> and source_of<Source, T&&>;
	
	template<typename T, typename Source>
	concept deducible_const_rvalue = deducible<T> and (source_of<Source, T const&&> or source_of<Source, T&&>);
	
	struct ambiguous_prvalue_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		template<deducible T>
		operator T () const;
		
		template<deducible T>
		operator T& () const;
		
		template<deducible T>
		operator T const& () const;
	};
	
	struct placeholder_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		template<deducible T>
		operator T ();
		
		template<deducible T>
		operator T& () const;
		
		template<deducible T>
		operator T const& () const;
		
		template<deducible T>
		operator T&& () const;
		
		template<deducible T>
		operator T const&& () const;
	};
	
	template<source_ref Source>
	struct basic_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr basic_deducer(forwarded<std::remove_cvref_t<Source>> auto const&& source)
		noexcept requires std::is_const_v<std::remove_reference_t<Source>> :
			source{std::addressof(source)} {}
		
		explicit constexpr basic_deducer(forwarded<std::remove_cvref_t<Source>> auto&& source) noexcept :
			source{std::addressof(source)} {}
		
		template<deducible_prvalue<Source> T>
		constexpr operator T() {
			return provide(provide_tag_v<T>, static_cast<Source&&>(*source));
		}
		
		template<deducible_lvalue<Source> T>
		constexpr operator T&() const {
			return provide(provide_tag_v<T&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_const_lvalue<Source> T>
		constexpr operator T const&() const {
			if constexpr (source_of<Source, T const&>) {
				return provide(provide_tag_v<T const&>, static_cast<Source&&>(*source));
			} else if constexpr (source_of<Source, T&>) {
				return std::as_const(provide(provide_tag_v<T&>, static_cast<Source&&>(*source)));
			} else if constexpr (source_of<Source, T const&&>) {
				return static_cast<T const&>(provide(provide_tag_v<T const&&>, static_cast<Source&&>(*source)));
			} else {
				return static_cast<T const&>(provide(provide_tag_v<T&&>, static_cast<Source&&>(*source)));
			}
		}
		
		template<deducible_rvalue<Source> T>
		constexpr operator T&&() const {
			return provide(provide_tag_v<T&&>, static_cast<Source&&>(*source));
		}
		
		template<deducible_const_rvalue<Source> T>
		constexpr operator T const&&() const {
			if constexpr (source_of<Source, T const&&>) {
				return provide(provide_tag_v<T const&&>, static_cast<Source&&>(*source));
			} else {
				return provide(provide_tag_v<T&&>, static_cast<Source&&>(*source));
			}
		}
		
	private:
		std::remove_reference_t<Source>* source;
	};
	
	template<typename Exclude, deducer Deducer>
	struct exclude_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T>)
		constexpr operator T() {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T&> and deducer_for<Deducer const, T&>)
		constexpr operator T&() const {
			return deducer.operator T&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T const&> and deducer_for<Deducer const, T const&>)
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T&&> and deducer_for<Deducer const, T&&>)
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T const&&> and deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<typename T>
	inline constexpr auto exclude_deduction(deducer auto deducer) {
		return exclude_deducer<T, decltype(deducer)>{deducer};
	}

	template<typename Exclude, deducer Deducer>
	struct exclude_special_constructors_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_special_constructors_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer, T>)
		constexpr operator T() {
			return deducer.operator T();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T&>)
		constexpr operator T&() const {
			return deducer.operator T&();
		}

		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T const&>)
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T&&>)
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires (different_from<Exclude, T> and deducer_for<Deducer const, T const&&>)
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<deducer Deducer>
	struct exclude_prvalue_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_prvalue_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<deducible T>
			requires deducer_for<Deducer const, T&>
		constexpr operator T&() const {
			return deducer.operator T&();
		}

		template<deducible T>
			requires deducer_for<Deducer const, T const&>
		constexpr operator T const&() const {
			return deducer.operator T const&();
		}
		
		template<deducible T>
			requires deducer_for<Deducer const, T&&>
		constexpr operator T&&() const {
			return deducer.operator T&&();
		}
		
		template<deducible T>
			requires deducer_for<Deducer const, T const&&>
		constexpr operator T const&&() const {
			return deducer.operator T const&&();
		}
		
	private:
		Deducer deducer;
	};
	
	template<deducer Deducer>
	struct exclude_references_deducer {
		using is_deducer = kangaru_deducer_tag;
		
		explicit constexpr exclude_references_deducer(std::same_as<Deducer> auto deducer) noexcept :
			deducer{deducer} {}
		
		template<deducible T>
			requires deducer_for<Deducer, T>
		constexpr operator T() {
			return deducer.operator T();
		}
		
	private:
		Deducer deducer;
	};
	
	template<deducible T>
	inline constexpr auto exclude_special_constructors_for(deducer auto deducer) {
		return exclude_special_constructors_deducer<T, decltype(deducer)>{deducer};
	}
	
	namespace detail::deducer {
		// These traits and function are only required for compilers that always prefer prvalue
		template<typename T, typename F, std::size_t... before, std::size_t... after>
		inline constexpr auto callbale_with_nth_parameter_being_expand(std::index_sequence<before...>, std::index_sequence<after...>) -> bool {
			return callable<
				F,
				detail::utility::expand<placeholder_deducer, before>...,
				T,
				detail::utility::expand<placeholder_deducer, after>...
			>;
		}
		
		template<typename T, typename F, std::size_t nth, std::size_t max>
		inline constexpr auto callbale_with_nth_parameter_being() -> bool {
			return callbale_with_nth_parameter_being_expand<T, F>(std::make_index_sequence<nth>{}, std::make_index_sequence<max - nth - 1>{});
		}
		
		template<kangaru::deducer... Deducers, std::size_t... S>
		inline constexpr auto invoke_with_deducers_prvalue_filter(
			callable<Deducers...> auto&& function,
			std::index_sequence<S...>,
			Deducers... deduce
		) -> decltype(auto) {
			using F = decltype(function);
			return KANGARU5_FWD(function)(
				detail::type_traits::conditional_t<
					    callbale_with_nth_parameter_being<ambiguous_prvalue_deducer, F, S, sizeof...(S)>()
					and callbale_with_nth_parameter_being<exclude_prvalue_deducer<Deducers>, F, S, sizeof...(S)>(),
					exclude_prvalue_deducer<Deducers>,
					exclude_references_deducer<Deducers>
				>{deduce}...
			);
		}
	}
	
	template<deducer... Deducers>
	inline constexpr auto invoke_with_deducers(callable<Deducers...> auto&& function, Deducers... deduce) -> decltype(auto) {
		#if NEEDS_PRVALUE_PREPASS == 1
			return detail::deducer::invoke_with_deducers_prvalue_filter(KANGARU5_FWD(function), std::index_sequence_for<Deducers...>{}, deduce...);
		#else
			return KANGARU5_FWD(function)(deduce...);
		#endif
	}
} // namespace kangaru

#include "undef.hpp"

#endif // KANGARU_DETAIL_DEDUCER_HPP
