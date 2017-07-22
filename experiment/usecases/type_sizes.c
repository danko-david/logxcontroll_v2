
#include "experiment/usecases/usecases.h"

#define PRINT_TYPE_SIZE(a) \
	printf("sizeof("#a"): %d bytes\n", sizeof(#a));

void type_sizes(int argc, char **argv, int start_from)
{
	PRINT_TYPE_SIZE(void*);
	PRINT_TYPE_SIZE(struct lxc_gate_behavior);
	PRINT_TYPE_SIZE(struct lxc_generic_portb_behavior);
	PRINT_TYPE_SIZE(struct lxc_generic_porti_instance);
	PRINT_TYPE_SIZE(struct lxc_generic_portb_behavior);
	PRINT_TYPE_SIZE(struct lxc_generic_porti_instance);
	PRINT_TYPE_SIZE(struct lxc_port_manager);
	PRINT_TYPE_SIZE(struct lxc_value);
	PRINT_TYPE_SIZE(struct lxc_value_operation);
	PRINT_TYPE_SIZE(struct lxc_primitive_value);
	PRINT_TYPE_SIZE(struct lxc_wire);
	PRINT_TYPE_SIZE(struct lxc_instance);
	PRINT_TYPE_SIZE(struct lxc_tokenport);
	PRINT_TYPE_SIZE(struct lxc_signal_type);
	PRINT_TYPE_SIZE(struct circuit);

	PRINT_TYPE_SIZE(struct worker_pool);
	PRINT_TYPE_SIZE(struct rerunnable_thread);
	PRINT_TYPE_SIZE(struct queue_element);
	PRINT_TYPE_SIZE(struct lxc_task);

	PRINT_TYPE_SIZE(short_lock);
	PRINT_TYPE_SIZE(long_lock);

	PRINT_TYPE_SIZE(struct lxc_property_manager);

}
