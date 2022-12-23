#include <linux/module.h>

#include <linux/syscalls.h>

#include <linux/printk.h>

#define COPY_TO_USER(dest, src, size)                                          \
	if (copy_to_user(dest, src, size))                                     \
		return -EFAULT;

struct cstm_lsmod_module_info {
	unsigned int size;
	int references_count;
	char name[MODULE_NAME_LEN];
};

SYSCALL_DEFINE1(lsmod_amount_modules, long *, modules_amount)
{
	struct kobject *k;

	long count = 0;

	list_for_each_entry (k, &module_kset->list, entry) {
		struct module_kobject *cur_module_kobject =
			container_of(k, struct module_kobject, kobj);

		if (cur_module_kobject->mod != NULL) {
			count++;
		}
	}

	COPY_TO_USER(modules_amount, &count, sizeof(modules_amount))

	return 0;
}

SYSCALL_DEFINE2(lsmod_info, struct cstm_lsmod_module_info **, goal_modules_info,
		char **, references)
{
	struct kobject *k;

	struct cstm_lsmod_module_info **helper_modules_info = goal_modules_info;
	long num = 0;

	list_for_each_entry (k, &module_kset->list, entry) {
		struct module *cur_module =
			container_of(k, struct module_kobject, kobj)->mod;

		if (cur_module != NULL) {
			int cur_module_refcnt = module_refcount(cur_module);

			COPY_TO_USER(helper_modules_info[num]->name,
				     cur_module->name,
				     sizeof(helper_modules_info[num]->name));

			COPY_TO_USER(&helper_modules_info[num]->size,
				     &cur_module->core_layout.size,
				     sizeof(helper_modules_info[num]->size));

			COPY_TO_USER(
				&helper_modules_info[num]->references_count,
				&cur_module_refcnt,
				sizeof(helper_modules_info[num]
					       ->references_count));

			struct module_use *use;
			size_t ch;
			long count_drc = 0;
			list_for_each_entry (use, &cur_module->source_list,
					     source_list) {
				for (ch = 0; ch < MODULE_NAME_LEN; ch++) {
					if (use->target->name[ch] == '\0') {
						break;
					}
					COPY_TO_USER(
						&references[num][count_drc],
						&use->target->name[ch],
						sizeof(char));

					count_drc++;
				}

				printk("%s", use->target->name);

				char zap = ',';
				COPY_TO_USER(&references[num][count_drc], &zap,
					     sizeof(char));
				count_drc++;
			}

			num++;
		}
	}

	return 0;
}