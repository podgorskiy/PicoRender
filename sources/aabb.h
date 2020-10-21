#pragma once
#include <glm/glm.hpp>
#include <limits>

namespace glm
{
	template <int Dim, typename Type>
	class aabb
	{
	public:
		typedef vec<Dim, Type> vec_type;

		// Default constructor creates aabb with largest possible negative volume.
		// Union with any point will make zero size aabb
		aabb() : minp(std::numeric_limits<Type>().max()), maxp(-std::numeric_limits<Type>().max()) {}

		// Creates aabb of zero size, where max == p and min == p
		explicit aabb(vec_type p) : minp(p), maxp(p) {}

		// Initializes with given maxp and minp
		aabb(vec_type minp, vec_type maxp) : minp(minp), maxp(maxp) {}

		void set(const vec_type& minp_, const vec_type& maxp_) { minp = minp_; maxp = maxp_; }

		// Sets the aabb to largest possible negative volume.
		void reset() { *this = aabb(); }

		// Sets the aabb to largest possible positive volume.
		void set_any() {
			set(vec_type(-std::numeric_limits<Type>().max()), vec_type(std::numeric_limits<Type>().max()));
		}

		vec_type size() const { return maxp - minp; }

		vec_type center() const { return (maxp + minp) / Type(2); }

		// Returns true if bounds positive volume
		bool is_positive() const { return all(lessThan(minp, maxp)); }

		// Returns true if bounds negative volume
		bool is_negative() const { return any(greaterThan(minp, maxp)); }

		vec_type minp, maxp;
	};

	// Union of a point with aabb
	template <int Dim, typename Type>
	inline aabb<Dim, Type> operator | (const aabb<Dim, Type>& x, const vec<Dim, Type>& point)
	{
		return aabb<Dim, Type>(min(x.minp, point), max(x.maxp, point));
	}

	// Union of a point with aabb
	template <int Dim, typename Type>
	inline aabb<Dim, Type> operator | (const vec<Dim, Type>& point, const aabb<Dim, Type>& x)
	{
		return aabb<Dim, Type>(min(x.minp, point), max(x.maxp, point));
	}

	// Union of a point with aabb
	template <int Dim, typename Type>
	inline aabb<Dim, Type> operator |= (const aabb<Dim, Type>& x, const vec<Dim, Type>& point)
	{
		x = x | point;
		return x;
	}

	// Union of a two aabb's
	template <int Dim, typename Type>
	inline aabb<Dim, Type> operator | (const aabb<Dim, Type>& a, const aabb<Dim, Type>& b)
	{
		return (a | b.minp) | b.maxp;
	}

	// Union of a two aabb's
	template <int Dim, typename Type>
	inline aabb<Dim, Type> operator |= (aabb<Dim, Type>& a, const aabb<Dim, Type>& b)
	{
		a = a | b;
		return a;
	}

	// Intersection of a two aabb's
	template <int Dim, typename Type>
	inline aabb<Dim, Type> operator & (const aabb<Dim, Type>& a, const aabb<Dim, Type>& b)
	{
		return aabb<Dim, Type>(max(a.minp, b.minp), min(a.maxp, b.maxp));
	}

	// Intersection of a two aabb's
	template <int Dim, typename Type>
	inline aabb<Dim, Type> operator &= (aabb<Dim, Type>& a, const aabb<Dim, Type>& b)
	{
		a = a & b;
		return a;
	}

	// If intersection of the two aabb is not empty returns true, otherwise false
	template <int Dim, typename Type>
	inline bool is_overlapping(aabb<Dim, Type> a, aabb<Dim, Type> b)
	{
		return !(a & b).is_negative();
	}

	// If the point is inside or on the border of the aabb returns true, otherwise false
	template <int Dim, typename Type>
	inline bool is_inside(aabb<Dim, Type> x, const vec<Dim, Type> point)
	{
		return all(lessThanEqual(x.minp, point)) && all(greaterThanEqual(x.maxp, point));
	}

	typedef aabb<2, float> aabb2;
	typedef aabb<3, float> aabb3;
	typedef aabb<4, float> aabb4;

	typedef aabb<2, double> daabb2;
	typedef aabb<3, double> daabb3;
	typedef aabb<4, double> daabb4;

	typedef aabb<2, int> iaabb2;
	typedef aabb<3, int> iaabb3;
	typedef aabb<4, int> iaabb4;
}
