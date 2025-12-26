#include <SFE/sfe.hpp>

struct Glyph {
    int width, height;
    int x_baseline_offset;
    int y_baseline_offset;
    float x_advance;
    Texture bitmap;
};

struct Font {
    stbtt_fontinfo info;
    float scale;

    float ascent;
    float descent;
    float line_gap;
    float line_height;

    DS::Hashmap<int, Glyph> glyphs;
};

bool LoadFontAndCacheGlyphs(Font &font, const unsigned char* ttf_data, float font_point_scale, int first_codepoint, int last_codepoint) {
    if (!stbtt_InitFont(&font.info, ttf_data, stbtt_GetFontOffsetForIndex(ttf_data, 0))) {
        return false;
    }

    font.scale = stbtt_ScaleForPixelHeight(&font.info, font_point_scale);

    int ascent, descent, lineGap;
    stbtt_GetFontVMetrics(&font.info, &ascent, &descent, &lineGap);

    font.ascent  = ascent  * font.scale;
    font.descent = descent * font.scale;
    font.line_gap = lineGap * font.scale;
    font.line_height = (ascent - descent + lineGap) * font.scale;

    font.glyphs = DS::Hashmap<int, Glyph>(last_codepoint - first_codepoint);
    for (int codepoint = first_codepoint; codepoint <= last_codepoint; codepoint++) {
        Glyph glyph;

        int advance, lsb;
        stbtt_GetCodepointHMetrics(&font.info, codepoint, &advance, &lsb);
        glyph.x_advance = advance * font.scale;

        u8* bmp = stbtt_GetCodepointBitmap(
            &font.info,
            0, font.scale,
            codepoint,
            &glyph.width,
            &glyph.height,
            &glyph.x_baseline_offset,
            &glyph.y_baseline_offset
        );

        glyph.bitmap = Texture::LoadFromMemory(bmp, glyph.width, glyph.height, 4, true);
        stbtt_FreeBitmap(bmp, nullptr);

        font.glyphs.put(codepoint, glyph);
    }

    return true;
}