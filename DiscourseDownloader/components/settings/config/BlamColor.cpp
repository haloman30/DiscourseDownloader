#include "BlamColor.h"

#include "components/utils/string/string.h"

byte BlamColor::convert_float_clamped(float f)
{
	int integer_value = 255 * f;

	// Clamp value
	{
		if (integer_value > 255)
		{
			integer_value = 255;
		}

		if (integer_value < 0)
		{
			integer_value = 0;
		}
	}

	return integer_value;
}

BlamColor::BlamColor()
{
	r = 255;
	g = 255;
	b = 255;
	a = 255;
}

BlamColor::BlamColor(byte _r, byte _g, byte _b)
{
	r = _r;
	g = _g;
	b = _b;
	a = 255;
}

BlamColor::BlamColor(byte _r, byte _g, byte _b, byte _a)
{
	r = _r;
	g = _g;
	b = _b;
	a = _a;
}

std::string BlamColor::ToString()
{
	return std::string(std::to_string(r) + "," + std::to_string(g) + "," + std::to_string(b) + "," + std::to_string(a));
}

float BlamColor::GetRedAsFloat()
{
	return r / 255.0f;
}

float BlamColor::GetGreenAsFloat()
{
	return g / 255.0f;
}

float BlamColor::GetBlueAsFloat()
{
	return b / 255.0f;
}

float BlamColor::GetAlphaAsFloat()
{
	return a / 255.0f;
}

int BlamColor::GetColorAsInt()
{
	int color_int = 0xFFFFFFFF;

	char* color_int_ptr = (char*)&color_int;

	memcpy(color_int_ptr + 3, &r, 1);
	memcpy(color_int_ptr + 2, &g, 1);
	memcpy(color_int_ptr + 1, &b, 1);
	memcpy(color_int_ptr, &a, 1);

	return color_int;
}

std::string BlamColor::ToString(std::string format)
{
	std::string new_string = format;

	new_string = DDL::Utils::String::Replace(new_string, "{r}", std::to_string(r));
	new_string = DDL::Utils::String::Replace(new_string, "{g}", std::to_string(g));
	new_string = DDL::Utils::String::Replace(new_string, "{b}", std::to_string(b));
	new_string = DDL::Utils::String::Replace(new_string, "{a}", std::to_string(a));

	new_string = DDL::Utils::String::Replace(new_string, "{r>f}", std::to_string(GetRedAsFloat()));
	new_string = DDL::Utils::String::Replace(new_string, "{g>f}", std::to_string(GetGreenAsFloat()));
	new_string = DDL::Utils::String::Replace(new_string, "{b>f}", std::to_string(GetBlueAsFloat()));
	new_string = DDL::Utils::String::Replace(new_string, "{a>f}", std::to_string(GetAlphaAsFloat()));

	return new_string;
}

std::string BlamColor::ToStringForCSS()
{
	return ToString("rgba({r},{g},{b},{a>f})");
}

bool BlamColor::FromString(std::string string)
{
	if (string.starts_with("#"))
	{
		// html not supported yet
	}
	else
	{
		std::vector<std::string> color_values = DDL::Utils::String::Split(string, ",");

		if (color_values.size() == 3)
		{
			r = atoi(color_values.at(0).c_str());
			g = atoi(color_values.at(1).c_str());
			b = atoi(color_values.at(2).c_str());
			a = 255;

			return true;
		}
		else if (color_values.size() == 4)
		{
			r = atoi(color_values.at(0).c_str());
			g = atoi(color_values.at(1).c_str());
			b = atoi(color_values.at(2).c_str());
			a = atoi(color_values.at(3).c_str());

			return true;
		}
	}

	return false;
}

bool BlamColor::Equals(BlamColor other_color)
{
	if (other_color.r != r)
	{
		return false;
	}

	if (other_color.g != g)
	{
		return false;
	}

	if (other_color.b != b)
	{
		return false;
	}

	if (other_color.a != a)
	{
		return false;
	}

	return true;
}