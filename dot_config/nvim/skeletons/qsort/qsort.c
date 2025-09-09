typedef struct {
	int val;
	int i;
} item_t;

// example for above struct
// note that one can be cute for these numeric types
//  (left > right) - (left < right);
static int cmp_item_by_value_then_index(const void *pa, const void *pb)
{
	const item_t *a = pa, *b = pb;
	int dv = (a->val > b->val) - (a->val < b->val);
	if (dv)
		return dv;
	return (a->i > b->i) - (a->i < b->i);
}
