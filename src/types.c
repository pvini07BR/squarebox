#include "types.h"

#include <raymath.h>
#include <rlgl.h>

Label create_label(const char* str, float fontSize, float spacing, Font font) {
	Label label;
	label.str = str;
	label.font = font;
	label.fontSize = fontSize;
	label.spacing = spacing;
	label.bounds = MeasureTextEx(font, str, fontSize, spacing);
	return label;
}

void draw_label(Label* label, Vector2 pos, float rotation, float scale, Color color, LabelAlignment horizontal_alignment, LabelAlignment vertical_alignment) {
    Vector2 origin = Vector2Zero();

    switch (horizontal_alignment) {
    case LABEL_ALIGN_CENTER:
        origin.x += label->bounds.x / 2.0f;
        break;
    case LABEL_ALIGN_END:
        origin.x += label->bounds.x;
        break;
    default:
        break;
    }

    switch (vertical_alignment) {
    case LABEL_ALIGN_CENTER:
        origin.y += label->bounds.y / 2.0f;
        break;
    case LABEL_ALIGN_END:
        origin.y += label->bounds.y;
        break;
    default:
        break;
    }

    rlPushMatrix();

    rlTranslatef(pos.x, pos.y, 0.0f);
    rlScalef(scale, scale, 1.0f);
    
    DrawTextPro(
        label->font,
        label->str,
        Vector2Zero(),
        origin,
        rotation,
        label->fontSize,
        label->spacing,
        color
    );

    rlPopMatrix();
}