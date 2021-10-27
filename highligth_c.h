enum c_tk_type_t
{
    multiline_comment,
    oneline_comment,
    preproc_dir,
    keyword,
    type,
    number,
    chararcter,
    string,
    operator,
    empty,
    undefined
};

typedef enum c_tk_type_t c_token_type;

void output_c_line(WINDOW *win, wchar_t* a, size_t len, int start, size_t offset, c_token_type *gl);