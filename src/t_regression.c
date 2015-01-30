#include <glib.h>
#include <stdint.h>
#include "glue.h"
#include "hammer.h"
#include "test_suite.h"
#include "internal.h"

static void test_bug118(void) {
  // https://github.com/UpstandingHackers/hammer/issues/118
  // Adapted from https://gist.github.com/mrdomino/c6bc91a7cb3b9817edb5

  HParseResult* p;
  const uint8_t *input = (uint8_t*)"\x69\x5A\x6A\x7A\x8A\x9A";
 
#define MY_ENDIAN (BIT_BIG_ENDIAN | BYTE_LITTLE_ENDIAN)
    H_RULE(nibble, h_with_endianness(MY_ENDIAN, h_bits(4, false)));
    H_RULE(sample, h_with_endianness(MY_ENDIAN, h_bits(10, false)));
#undef MY_ENDIAN
 
    H_RULE(samples, h_sequence(h_repeat_n(sample, 3), h_ignore(h_bits(2, false)), NULL));
 
    H_RULE(header_ok, h_sequence(nibble, nibble, NULL));
    H_RULE(header_weird, h_sequence(nibble, nibble, nibble, NULL));
 
    H_RULE(parser_ok, h_sequence(header_ok, samples, NULL));
    H_RULE(parser_weird, h_sequence(header_weird, samples, NULL));
 
 
    p = h_parse(parser_weird, input, 6);
    g_check_cmp_int32(p->bit_length, ==, 44);
    h_parse_result_free(p);
    p = h_parse(parser_ok, input, 6);
    g_check_cmp_int32(p->bit_length, ==, 40);
    h_parse_result_free(p);
}

static void test_seq_index_path(void) {
  HArena *arena = h_new_arena(&system_allocator, 0);

  HParsedToken *seq = h_make_seqn(arena, 1);
  HParsedToken *seq2 = h_make_seqn(arena, 2);
  HParsedToken *tok1 = h_make_uint(arena, 41);
  HParsedToken *tok2 = h_make_uint(arena, 42);

  seq->seq->elements[0] = seq2;
  seq->seq->used = 1;
  seq2->seq->elements[0] = tok1;
  seq2->seq->elements[1] = tok2;
  seq2->seq->used = 2;

  g_check_cmp_int(h_seq_index_path(seq, 0, -1)->token_type, ==, TT_SEQUENCE);
  g_check_cmp_int(h_seq_index_path(seq, 0, 0, -1)->token_type, ==, TT_UINT);
  g_check_cmp_int64(h_seq_index_path(seq, 0, 0, -1)->uint, ==, 41);
  g_check_cmp_int64(h_seq_index_path(seq, 0, 1, -1)->uint, ==, 42);
}

void register_regression_tests(void) {
  g_test_add_func("/core/regression/bug118", test_bug118);
  g_test_add_func("/core/regression/seq_index_path", test_seq_index_path);
}
