#include "types.h"

Label create_label(const char* str, float fontSize, float spacing, Font font) {
	Label label;
	label.str = str;
	label.font = font;
	label.fontSize = fontSize;
	label.spacing = spacing;
	label.bounds = MeasureTextEx(font, str, fontSize, spacing);
	return label;
}