#define DBG_PRINT(x) if (vm_debug) { fputs("vm: " x "\n", stderr); }
#define DBG_PRINTF(x, args...) if (vm_debug) { fprintf(stderr, "vm: " x "\n", args); }

