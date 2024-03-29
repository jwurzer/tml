TML - Tiny Markup Language
==========================

TML is a human-readable smart data-serialization language. Something like YAML, JSON, XML.

The content of a TML file consists of one or more name-value pairs. The following is an example with different name-value pairs.

	someText = hello
	a-number = 13
	perentage = 0.34
	array-with-names = Alice Bob Carol Eve Mallory
	aObjectWithTml
		name = Max
		weight = 71.7
		body-height = 123
	use-mipmap = false
	active = true
	asdf = null
	# this is a comment

	text = "text with spaces must be between apostrophes"

A indent must be one or more tabs or one or more spaces.
The first indent defines the characters for a indent (must be one or more tabs or one or more spaces). Mixing is not allowed!

TML supports following types:

 * **Null**:             `null`
 * **Boolean**:          Can be `true` or `false`
 * **Integer**:          Sequence of digits. Optional starting with `+` or `-`.
 * **Floating point**:   Sequence of digits with a dot `.`. Optional starting with `+` or `-`.
 * **Text**:             A Text is everthing which is not a Null, a Boolean, a Integer and not a Floating point.
                         e.g. `ThisIsAText123`. A text with spaces is not classified a simple text.
                         If a text with spaces is necessary then apostrophes must be used.
                         e.g. `"This is a example with spaces"`.
 * **Array**:            The elements of an array are separated with spaces.
 * **Object (Section)**: An object has an object name at the first line without a value (no = ... at the first line).
                         The members of of the object are the following lines. The members must be indent.
			 An object can also be seen as a section (like from an INI file).

Objects can be nested like the following example:

	aObject
		name = Max
		weight = 71.7
		body-height = 123

		subObj
			idName = TEX123
			links = 3
			date = null
			counter = 3
			numbers = 4 2 8 0 2 4

		subObj
			idName = OBJ321
			links = 6
			numbers = 4.3 2.1 8 0 2 4

			otherObj
				idName = 321
				links = 6
				numbers = 4 2 8 0 2 4

		subObj
			idName = 321-A
			links = 6
			numbers = 4 2 8 0 2 4

TML also supports templates (prefabs) and special name-value pairs. For more details see the [TML Specification](tml-specification.md) file.

## Compilation

### Linux

Install g++ make cmake

```
tml$ make
```

### Windows

Install Visual Studio 2022

Install CMake

Open "Developer Command Prompt for VS2022"

```
tml> nmake /F Makefile.nmake
```

Open build/tml.sln with Visual Studio 2022 and compile it.

For direct compilation the target `compile` can be used.

```
tml> nmake /F Makefile.nmake compile
```

For using the Makefile (without .nmake) the GNU make utility for Windows is needed (https://gnuwin32.sourceforge.net/packages/make.htm). Also cygwin is supported.

License
-------
TML is dual-licensed under the very permissive [zlib license](LICENSE) and [MIT license](MIT-LICENSE).
Every source file (excepted include/json/jsmn.h) includes an explicit dual-license for you to choose from.
Just choose one license of the two that is more suitable for you.

This program is free software; you can redistribute it and/or modify
it under the terms of either zlib license and/or MIT license.

For more information about the licences see the [LICENSE](LICENSE) file for zlib license or 
[MIT-LICENSE](MIT-LICENSE) for the MIT license.

The file include/json/jsmn.h is only under the MIT license.
It's from the external project https://github.com/zserge/jsmn
