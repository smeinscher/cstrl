#!/bin/bash

date >> line_count_history.txt;
SRC_WORD_COUNT="$(find ./src -type f \( -iname \*.h -o -iname \*.c \) | \
  xargs sed '/^\s*#/d;/^\s*$/d' | \
  grep -v "^\s*//" | \
  sed -r ':a; s%(.*)/\*.*\*/%\1%; ta; /\/\*/ !b; N; ba' | \
  wc -l)"
TEST_WORD_COUNT="$(find ./test -type f \( -iname \*.h -o -iname \*.c \) | \
  xargs sed '/^\s*#/d;/^\s*$/d' | \
  grep -v "^\s*//" | \
  sed -r ':a; s%(.*)/\*.*\*/%\1%; ta; /\/\*/ !b; N; ba' | \
  wc -l)"
INCLUDE_WORD_COUNT="$(find ./include -type f \( -iname \*.h -o -iname \*.c \) | \
  xargs sed '/^\s*#/d;/^\s*$/d' | \
  grep -v "^\s*//" | \
  sed -r ':a; s%(.*)/\*.*\*/%\1%; ta; /\/\*/ !b; N; ba' | \
  wc -l)"
SANDBOX_WORD_COUNT="$(find ./sandbox -type f \( -iname \*.h -o -iname \*.c \) | \
  xargs sed '/^\s*#/d;/^\s*$/d' | \
  grep -v "^\s*//" | \
  sed -r ':a; s%(.*)/\*.*\*/%\1%; ta; /\/\*/ !b; N; ba' | \
  wc -l)"
echo $((SRC_WORD_COUNT + TEST_WORD_COUNT + INCLUDE_WORD_COUNT + SANDBOX_WORD_COUNT)) >> line_count_history.txt
tail -n 2 line_count_history.txt;
