#include "gcc-plugin.h"
#include "tree.h"
#include "tree-pretty-print.h"
#include <stdio.h>

int plugin_is_GPL_compatible;

static tree
handle_pythonapi_attribute (tree *node, tree name, tree args,
			int flags, bool *no_add_attrs)
{
    //print_generic_decl(stderr, *node, TDF_RAW);
    print_generic_stmt(stderr, *node, TDF_NONE);
    //print_generic_stmt(stderr, name, TDF_NONE);
    print_generic_stmt_indented(stderr, args, TDF_NONE, 1);
    return NULL_TREE;
}

static struct attribute_spec pythonapi_attr = {
    .name="pythonapi", 
    .min_length=2,
    .max_length=2,
    .decl_required=true,
};

static void 
register_attributes(void *event_data, void *data) 
{
    pythonapi_attr.handler = handle_pythonapi_attribute;
    register_attribute(&pythonapi_attr);
}

int
plugin_init(struct plugin_name_args *plugin_info,
            struct plugin_gcc_version *version)
{
    const char *plugin_name = plugin_info->base_name;
    register_callback(plugin_name, PLUGIN_ATTRIBUTES, register_attributes, NULL);
    return 0;
}
