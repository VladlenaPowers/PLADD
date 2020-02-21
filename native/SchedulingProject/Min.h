#pragma once


template<typename K, typename V>
class Min
{
public:
	Min(K key, V val)
	{
		Min::val = val;
	}
	bool Compare(K key, V & val)
	{
		if (key < Min::key)
		{
			Min::key = key;
			Min::val = val;
			return true;
		}
		else
		{
			return false;
		}
	}

	V val;
	K key;
};
