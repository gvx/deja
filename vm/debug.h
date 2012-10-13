#define IF_DBG(x) if (vm_debug) { x }
#define DBG_PRINT(x) IF_DBG(fputs("vm: " x "\n", stderr);)
#define DBG_PRINTF(x, args...) IF_DBG(fprintf(stderr, "vm: " x "\n", args);)
