
TML - Tiny Markup Language
==========================

Specification:

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

Name-value pairs that do not use a string as type for the name are also allowed.
Furthermore, name-value pairs without a value are also allowed.

Some examples for these exotic name-value pairs:

	1234 = hello
	
	this is an array as name without a value
	
	12 ab 34 cd 1.0 4.0 = a b c

Such name-value pairs are not compatible with other data-serialization languages like JSON.

## Templates (prefabs)

TML also supports templates (`template`) and template references (`use-template`). With template references a template can be used multiple times. The sense behind this is that objects can be defined as templates and these objects can be used several times. The content of a template can also be a simple name-value pair. Also multiple name-value pairs at one template are allowed. A template represents one or more name-value pairs. A template can't be used to represent a single value. Only a value-pair is possible.

A template is a TML object with the name/keyword `template`.
This object must define a `name` and `parameters`. If no parameters are used then the value `none` must be used.

Syntax of a template:

	template
		name = <name-of-the-template>
		parameters = <none or names-for-parameters>
		<custom-template-content>

A simple example of a template without parameters:

	template
		name = ITEM-A
		parameters = none
		entity
			texture-component
				tex-id-unit-0 = TEX1
			shader-component
				shader-program-id = SHADER1
			mesh-component
				mesh-id = MESH1

A template can be used by a `use-template` reference.
The syntax of a use-template reference is as follow:

	use-template <template-name> [<param1> <param2> <param3> <param4> ...]

The following example use the above template three times with `use-template`.

	scenes
		scene
			id = SCENE-ONE
			entity
				transform-component
					translate -20 0
					scale 2.5 2.5
				use-template ITEM-A
			entity
				transform-component
					translate 200 100
				use-template ITEM-A
			entity
				transform-component
					translate 300 500
					rotate-degree 45
				use-template ITEM-A

The result of the replaced `use-template` references is:

	scenes
		scene
			id = SCENE-ONE
			entity
				transform-component
					translate -20 0
					scale 2.5 2.5
				entity
					texture-component
						tex-id-unit-0 = TEX1
					shader-component
						shader-program-id = SHADER1
					mesh-component
						mesh-id = MESH1
			entity
				transform-component
					translate 200 100
				entity
					texture-component
						tex-id-unit-0 = TEX1
					shader-component
						shader-program-id = SHADER1
					mesh-component
						mesh-id = MESH1
			entity
				transform-component
					translate 300 500
					rotate-degree 45
				entity
					texture-component
						tex-id-unit-0 = TEX1
					shader-component
						shader-program-id = SHADER1
					mesh-component
						mesh-id = MESH1

### Templates with parameters

Instead of `none` one or more names for parameters can be defined.

Example of a template with parameters:

	template
		name = COLOR
		parameters = RED GREEN BLUE ALPHA
		alpha-for-fill = ALPHA
		color = RED GREEN BLUE

A use-template reference must use the correct count of arguments after the template name.

Referencing a template with parameters like:

	game-object
		game-object
			mesh = MESH-A
			use-template COLOR 1.0 0.5 0.0 1.0
		game-object
			mesh = MESH-B
			use-template COLOR 0.0 0.0 1.0 1.0
			texture = NORMAL

The result is:

	game-object
		game-object
			mesh = MESH-A
			alpha-for-fill = 1.0
			color = 1.0 0.5 0.0
		game-object
			mesh = MESH-B
			alpha-for-fill = 1.0
			color = 0.0 0.0 1.0
			texture = NORMAL

### Templates with template references

A template can also include another template by using a template reference inside.

In the following example the template SWITCH-RGB uses the template COLOR.

	template
		name = COLOR
		parameters = RED GREEN BLUE ALPHA
		alpha-for-fill = ALPHA
		color = RED GREEN BLUE
	template
		name = SWITCH-RGB
		parameters = RED GREEN BLUE ALPHA
		use-template COLOR BLUE GREEN RED ALPHA

Example for using this templates:

	game-object
		game-object
			mesh = MESH-A
			use-template COLOR 1.0 0.5 0.0 1.0
		game-object
			mesh = MESH-B
			use-template SWITCH-RGB 0.0 0.0 1.0 1.0
			texture = NORMAL

Result:

	game-object
		game-object
			mesh = MESH-A
			alpha-for-fill = 1.0
			color = 1.0 0.5 0.0
		game-object
			mesh = MESH-B
			alpha-for-fill = 1.0
			color = 1.0 0.0 0.0
			texture = NORMAL
