#include <map>
#include <string>
using namespace std;

typedef struct segment{
  string   seg_name  ;
  int           size;
  bool          is_mapped;
  bool          in_use;
  void*         address;

  segment()
	{
		is_mapped = 0;
		in_use = 0;
	};
	segment(void *seg_address, int seg_size)
	{
		address = seg_address;
		size = seg_size;
		is_mapped = 0;
		in_use = 0;
	};
}seg_t;

typedef struct rvm{
  string store_dir;
	map<string, seg_t> seg_map;

	rvm(const char *directory)
	{
		store_dir = directory;
	}
}rvm_data;

typedef rvm_data* rvm_t;
