

/* proto utils */
#include <string>
using Field = google::protobuf::RepeatedPtrField<std::string>;
static Field make_field(std::initializer_list<const char *> tags)
{
	Field f;
	for (auto t : tags)
		*f.Add() = t;
	return f;
}
/* end proto utils */
