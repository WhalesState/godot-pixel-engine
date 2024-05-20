func print_enum(e: BoxContainer.AlignmentMode) -> BoxContainer.AlignmentMode:
	print(e)
	return e

func test():
	var v: BoxContainer.AlignmentMode
	v = BoxContainer.ALIGNMENT_BEGIN
	v = print_enum(v)
	v = print_enum(BoxContainer.ALIGNMENT_BEGIN)
	v = BoxContainer.AlignmentMode.ALIGNMENT_BEGIN
	v = print_enum(v)
	v = print_enum(BoxContainer.AlignmentMode.ALIGNMENT_BEGIN)

	v = BoxContainer.ALIGNMENT_CENTER
	v = print_enum(v)
	v = print_enum(BoxContainer.ALIGNMENT_CENTER)
	v = BoxContainer.AlignmentMode.ALIGNMENT_CENTER
	v = print_enum(v)
	v = print_enum(BoxContainer.AlignmentMode.ALIGNMENT_CENTER)
