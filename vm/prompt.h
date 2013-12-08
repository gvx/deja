#define MAX_PROMPT 1024
typedef char prompt_t[MAX_PROMPT];

enum prompt_result {
	prompt_result_normal,
	prompt_result_interrupt,
	prompt_result_eof
};

enum prompt_result prompt(const char *in, prompt_t out);
