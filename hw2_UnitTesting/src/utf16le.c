#include "utf.h"
#include "debug.h"
#include "wrappers.h"
#include <unistd.h>

int
from_utf16le_to_utf16be(int infile, int outfile)
{
  int bom;
  utf16_glyph_t buf;
  ssize_t bytes_read;
  size_t bytes_to_write;
  int ret = 0;

  bom = UTF16BE;
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
  reverse_bytes(&bom, 2);
#endif
  write_to_bigendian(outfile, &bom, 2);

  while ((bytes_read = read_to_bigendian(infile, &(buf.upper_bytes), 2)) > 0) {
    bytes_to_write = 2;
    reverse_bytes(&(buf.upper_bytes), 2);
    if(is_lower_surrogate_pair(buf)) {
      if((bytes_read = read_to_bigendian(infile, &(buf.lower_bytes), 2)) < 0) {
        break;
      }
      reverse_bytes(&(buf.lower_bytes), 2);
      bytes_to_write += 2;
    }
    write_to_bigendian(outfile, &buf, bytes_to_write);
  }
  ret = bytes_read;
  return ret;
}

int
from_utf16le_to_utf8(int infile, int outfile)
{
  int ret = 0;
  utf16_glyph_t utf16_buf;
  ssize_t bytes_read;
  size_t size_of_glyph;
  code_point_t code_point;
  utf8_glyph_t utf8_buf;
  //int scanBom = 0;

  while((bytes_read = read_to_bigendian(infile, &utf16_buf.upper_bytes, 2)) > 0) {

    if(bytes_read < 0) break;

    if(is_upper_surrogate_pair(utf16_buf)){
      if((bytes_read = read_to_bigendian(infile, &utf16_buf.lower_bytes, 2)) > 0){
        if(bytes_read < 0) break;

        code_point = utf16_glyph_to_code_point(&utf16_buf);
        utf8_buf = code_point_to_utf8_glyph(code_point, &size_of_glyph);
        write_to_bigendian(outfile, &utf8_buf, size_of_glyph);
        utf16_buf.lower_bytes = 0;
      continue;
      }

    }

    code_point = utf16_glyph_to_code_point(&utf16_buf);
    utf8_buf = code_point_to_utf8_glyph(code_point, &size_of_glyph);
    write_to_bigendian(outfile, &utf8_buf, size_of_glyph); //LE or BE doesn't matter
  }


  ret = bytes_read;
  return ret;
}

utf16_glyph_t
code_point_to_utf16le_glyph(code_point_t code_point, size_t *size_of_glyph)
{
  utf16_glyph_t ret;

  memeset(&ret, 0, sizeof ret);
  if(is_code_point_surrogate(code_point)) {
    code_point -= 0x10000;
    ret.upper_bytes = (code_point >> 10) + 0xD800;
    ret.lower_bytes = (code_point & 0x3FF) + 0xDC00;
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      reverse_bytes(&ret.upper_bytes, 2);
      reverse_bytes(&ret.lower_bytes, 2);
    #endif
      *size_of_glyph = 4;
  }
  else {
    ret.upper_bytes |= code_point;
    #if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
      reverse_bytes(&ret.upper_bytes, 2);
    #endif
      *size_of_glyph = 2;
  }
  return ret;
}

code_point_t
utf16le_glyph_to_code_point(utf16_glyph_t glyph){
  code_point_t ret = 0;

  uint32_t encodedGlyph = (glyph.upper_bytes << 16) + glyph.lower_bytes;
  ret &= encodedGlyph;
  return ret;
}