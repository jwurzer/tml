#ifndef CFG_CPPSTRING_EXAMPLE_H
#define CFG_CPPSTRING_EXAMPLE_H

#include <cfg/cfg.h>

// This is an example for the following tml snippet.

/*
# This is a comment followed by an empty line.

objExample
	a = 1
	b = 2
	c = 3

txt = hello
array = 1 3 5 3 2
*/

static cfg::Value example =
cfg::object({
	cfg::comment(" This is a comment followed by an empty line."),
	cfg::empty(),
	cfg::pair(
		cfg::text("objExample"),
		cfg::object({
			cfg::pair(cfg::text("a"), cfg::intValue(1)),
			cfg::pair(cfg::text("b"), cfg::intValue(2)),
			cfg::pair(cfg::text("c"), cfg::intValue(3)),
			cfg::empty()})),
	cfg::pair(cfg::text("txt"), cfg::text("hello")),
	cfg::pair(
		cfg::text("array"),
		cfg::array({
			cfg::intValue(1),
			cfg::intValue(3),
			cfg::intValue(5),
			cfg::intValue(3),
			cfg::intValue(2)}))});
#endif
