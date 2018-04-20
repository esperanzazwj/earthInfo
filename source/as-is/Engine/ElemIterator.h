#pragma once
#include <map>
namespace HW
{
	// a wrapper for map list iterator .
	template<typename Tkey,typename Tval>
	class mapIterator
	{
		typedef typename std::map<Tkey,Tval>::iterator IterType;
		IterType iter;
	public:
		// should never be used.
		mapIterator(){}
		// constructor
		mapIterator(const IterType &it)
		{
			this->iter = it;
		}
		// copy constructor
		mapIterator(const mapIterator<Tkey,Tval> &other)
		{
			this->iter = other.iter;
		}
		Tval get() const
		{
			return iter->second;
		}

		mapIterator<Tkey,Tval>& operator++()
		{
			this->iter++;
			return *this;
		}

		mapIterator<Tkey,Tval> operator++(int)
		{
			mapIterator<Tkey,Tval> tmp = *this;
			this->iter++;
			return tmp;
		}

		mapIterator<Tkey,Tval>& operator=(const mapIterator<Tkey,Tval> &other)
		{
			this->iter = other.iter;
			return *this;
		}

		bool operator!=(const mapIterator<Tkey,Tval>& other) const
		{
			return (this->iter != other.iter);
		}

	};
}
