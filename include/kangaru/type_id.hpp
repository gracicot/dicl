#ifndef KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP

#ifdef _MSC_VER
#ifndef __clang__
#define KGR_KANGARU_NONCONST_TYPEID
#endif
#endif

namespace kgr {
namespace detail {

/*
 * Template class that hold the declaration of the id.
 * 
 * We use the pointer of this id as type id.
 */
template<typename T>
struct type_id_ptr {
	// Having a static data member will ensure us that it has only one address for the whole program.
	// Furthermore, the static data member having different types will ensure it won't get optimized.
#ifdef KGR_KANGARU_NONCONST_TYPEID
    static T const* id;
#else
	static T const* id;
#endif
};

/*
 * Definition of the id.
 * 
 * Before that, we used the pointer to a function as type_id.
 * 
 * However, on some platform, the rule that a pointer to a function
 * always yeild to the same value, no matter the dll or TU was not always respected.
 * 
 * Using the pointer of a static data member is more stable.
 */
#ifdef KGR_KANGARU_NONCONST_TYPEID
template<typename T>
T const* type_id_ptr<T>::id = nullptr;
#else
template<typename T>
T const* const type_id_ptr<T>::id = nullptr;
#endif

} // namespace detail

/*
 * The type of a type id.
 */
using type_id_t = const void*;

/*
 * The function that returns the type id.
 * 
 * It uses the pointer to the static data member of a class template to achieve this.
 * Altough the value is not predictible, it's stable.
 */
template <typename T>
constexpr const void* type_id() {
	return &detail::type_id_ptr<T>::id;
}

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_TYPE_ID_HPP
