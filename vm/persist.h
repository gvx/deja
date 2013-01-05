bool valid_persist_name(V);
char *make_persist_path(V);
bool persist(char*, V);
bool persist_all(char*, Stack*);
bool persist_all_file(FILE*, Stack*);
