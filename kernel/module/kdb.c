// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * Improved kdb lsmod implementation
 * Original by Jason Wessel (2010)
 * Enhanced for clarity, structure and maintainability
 */

#include <linux/module.h>
#include <linux/kdb.h>
#include "internal.h"

/*
 * Print module memory sizes
 */
static void kdb_print_mem_sizes(struct module *mod)
{
    int i;

    for (i = 0; i < MOD_MEM_NUM_TYPES; i++)
        kdb_printf("/%8u", mod->mem[i].size);
}

/*
 * Print module memory base addresses
 */
static void kdb_print_mem_bases(struct module *mod)
{
    int i;

    for (i = 0; i < MOD_MEM_NUM_TYPES; i++)
        kdb_printf("/0x%px", mod->mem[i].base);
}

/*
 * Return a string describing module state
 */
static const char *kdb_module_state_str(enum module_state state)
{
    switch (state) {
    case MODULE_STATE_GOING:
        return "Unloading";
    case MODULE_STATE_COMING:
        return "Loading";
    default:
        return "Live";
    }
}

/*
 * Print "Used by" list with no trailing spaces
 */
#ifdef CONFIG_MODULE_UNLOAD
static void kdb_print_used_by(struct module *mod)
{
    struct module_use *use;
    bool first = true;

    kdb_printf(" [");

    list_for_each_entry(use, &mod->source_list, source_list) {
        kdb_printf("%s%s",
            first ? " " : ", ",
            use->target->name);
        first = false;
    }

    kdb_printf(" ]");
}
#endif

/*
 * Improved implementation of the kdb 'lsmod' command
 */
int kdb_lsmod(int argc, const char **argv)
{
    struct module *mod;

    if (argc != 0)
        return KDB_ARGCOUNT;

    kdb_printf("Module                  Size            modstruct   State       Base addresses\n");

    list_for_each_entry(mod, &modules, list) {

        if (mod->state == MODULE_STATE_UNFORMED)
            continue;

        /* Name and text section size */
        kdb_printf("%-20s %8u", mod->name, mod->mem[MOD_TEXT].size);

        /* Remaining section sizes */
        kdb_print_mem_sizes(mod);

        /* Module struct address */
        kdb_printf("  0x%px", (void *)mod);

#ifdef CONFIG_MODULE_UNLOAD
        /* Reference count */
        kdb_printf(" %4d", module_refcount(mod));
#endif

        /* State */
        kdb_printf(" (%s)", kdb_module_state_str(mod->state));

        /* Base memory addresses */
        kdb_print_mem_bases(mod);

#ifdef CONFIG_MODULE_UNLOAD
        /* Used by [] list */
        kdb_print_used_by(mod);
#endif

        kdb_printf("\n");
    }

    return 0;
}
