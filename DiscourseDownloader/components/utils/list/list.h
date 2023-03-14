#pragma once

#include <vector>

/**
* Namespace containing utility functions for working with lists, such as arrays or vectors.
*/
namespace DDL::Utils::List
{
	template <typename T> bool VectorContains(std::vector<T> list, T item)
	{
		for (T list_item : list)
		{
			if (list_item == item)
			{
				return true;
			}
		}

		return false;
	}
}