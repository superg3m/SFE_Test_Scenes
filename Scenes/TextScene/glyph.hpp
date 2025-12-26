#include <SFE/sfe.hpp>

struct Glyph {
    int width, height;
    int x_baseline_offset;
    int y_baseline_offset;
    float x_advance;
    Texture texture;
    GFX::Geometry quad;
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

        // Texture::FlipVerticallyInPlace(bmp, glyph.width, glyph.height);
        glyph.texture = Texture::LoadFromMemory(bmp, glyph.width, glyph.height, 1, false);

        Math::Vec3 tr = Math::Vec3(glyph.width, glyph.height, +0.0f);
        Math::Vec3 br = Math::Vec3(glyph.width, 0, +0.0f);
        Math::Vec3 bl = Math::Vec3(0, 0, +0.0f);
        Math::Vec3 tl = Math::Vec3(0, glyph.height, +0.0f);
        glyph.quad = GFX::Geometry::Quad(tr, br, bl, tl);
        font.glyphs.put(codepoint, glyph);
        stbtt_FreeBitmap(bmp, nullptr);
    }

    return true;
}