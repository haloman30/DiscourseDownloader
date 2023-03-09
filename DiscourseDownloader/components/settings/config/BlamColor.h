#pragma once

#include <string>

#ifdef STRINGS_EXPORTS
#define STRINGS_API __declspec(dllexport) 
#else
#define STRINGS_API __declspec(dllimport) 
#endif

typedef unsigned char byte;

/**
* Class representing a color. Used for a variety of purposes in the engine.
*
* Color values range from 0 to 255. To convert to a float-based color, use #Blam::Colors::D2DColorFromBlamColor.
*/
class STRINGS_API BlamColor
{
private:
	byte convert_float_clamped(float f);

public:
	byte r = 255; //!< The Red value of the color.
	byte g = 255; //!< The Green value of the color.
	byte b = 255; //!< The Blue value of the color.
	byte a = 255; //!< The Alpha value of the color.

	BlamColor();

	/**
	* Creates a new BlamColor.
	* 
	* @param _r - The Red value of the color.
	* @param _g - The Green value of the color.
	* @param _b - The Blue value of the color.
	*/
	BlamColor(byte _r, byte _g, byte _b);

	/**
	* Creates a new BlamColor.
	*
	* @param _r - The Red value of the color.
	* @param _g - The Green value of the color.
	* @param _b - The Blue value of the color.
	* @param _a - The Alpha value of the color.
	*/
	BlamColor(byte _r, byte _g, byte _b, byte _a);

	/**
	* Converts the color value to a string.
	*
	* @returns The color information in `r,g,b,a` format.
	*/
	std::string ToString();

	/**
	* Gets the color's red value as a float, between 0 and 1.
	*
	* @returns The color's red value, converted to a float.
	*/
	float GetRedAsFloat();

	/**
	* Gets the color's red value as a float, between 0 and 1.
	*
	* @returns The color's red value, converted to a float.
	*/
	float GetGreenAsFloat();

	/**
	* Gets the color's red value as a float, between 0 and 1.
	*
	* @returns The color's red value, converted to a float.
	*/
	float GetBlueAsFloat();

	/**
	* Gets the color's red value as a float, between 0 and 1.
	*
	* @returns The color's red value, converted to a float.
	*/
	float GetAlphaAsFloat();

	/**
	* Gets the color's value as an integer.
	*
	* This function will convert this color's value as an integer, in `0xAABBGGRR` format. Each color is written
	* in its hexadecimal format (ranging from 0x0 to 0xFF).
	*
	* @returns The color's value converted to an integer.
	*/
	int GetColorAsInt();

	/**
	* Converts the color value to a string in the specified format.
	*
	* The provided string can contain the following keys which will be replaced with the color values:
	* * `{r}` - Red
	* * `{g}` - Green
	* * `{b}` - Blue
	* * `{a}` - Alpha
	*
	* Additionally, you can have a float representation of the value (instead of 0-255) by adding `>f` inside a color placeholder.
	* Example: `{r}` of 255 would be 1.0 with `{r>f}`.
	*
	* @param format - The string containing any color placeholders to be replaced.
	*
	* @returns The string with the color placeholders replaced.
	*/
	std::string ToString(std::string format);

	/**
	* Converts the color to a string that can be used with CSS.
	*
	* @returns The color represented as a string, in the following format: `rgba(r,g,b,a)`
	*/
	std::string ToStringForCSS();

	/**
	* Attempts to set color information from a string.
	*
	* String should be in either `r,g,b` or `r,g,b,a` format - the same format as is returned when using #BlamColor::ToString();
	*
	* @param string - The string to try to convert to a color.
	*
	* @returns `true` if the color was converted successfully, `false` if the conversion failed.
	*/
	bool FromString(std::string string);

	/**
	* Checks if this color is equal to another color.
	* 
	* This will check the r, g, b, and a values of the color against `other_color`. If all colors match,
	* then this will return `true`. Otherwise, this will return `false`.
	* 
	* @param other_color - The color to check against.
	* 
	* @returns `true` if all RBGA values are equal, otherwise returns `false`.
	*/
	bool Equals(BlamColor other_color);
};