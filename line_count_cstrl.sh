#!/bin/bash

SRC_WORD_COUNT="$(find ./src -type f \( -iname \*.h -o -iname \*.c \) | \
  xargs sed '/^\s*#/d;/^\s*$/d' | \
  grep -v "^\s*//" | \
  sed -r ':a; s%(.*)/\*.*\*/%\1%; ta; /\/\*/ !b; N; ba' | \
  wc -l)"
echo $((SRC_WORD_COUNT))
