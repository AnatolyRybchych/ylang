#include <string.h>

#include <ylang.h>
#include Y_CMDARGS
#include Y_UTIL

#define WS(STRING) fputs(STRING, stream)
#define WC(CHAR) fputc(CHAR, stream)
#define WFMT(FMT, ...) fprintf(stream, FMT,##__VA_ARGS__)

static bool is_file(const char *input);

static void write_doc(FILE *stream, const Y_CmdDoc *doc, unsigned int tabs);
static void write_tabs(FILE *stream, unsigned int tabs);
static void write_multiline(FILE *stream, unsigned int tabs, const char *text);
static void write_doc_name(FILE *stream, const char *name);
static void write_doc_description(FILE *stream, const char *descritpion, unsigned int tabs);
static void write_required_arguments_usage(FILE *stream, const Y_CmdMgr *mgr);
static void write_flags_usage(FILE *stream, const Y_CmdMgr *mgr);
static void write_rest_usage(FILE *stream, const Y_CmdMgr *mgr);
static void write_required_docs(FILE *stream, const Y_CmdMgr *mgr);
static void write_rest_docs(FILE *stream, const Y_CmdMgr *mgr);
static void write_rest_amout(FILE *stream, const Y_CmdRest *rest);
static void write_type_docs(FILE *stream, const Y_CmdInputType *type, unsigned int tabs);
static void write_flags_docs(FILE *stream, const Y_CmdMgr *mgr);
static bool str_overlaps(const char *first, const char *second);
static Y_CmdFlag *get_flag(Y_CmdMgr *mgr, const char *flag);
static bool is_satisfies(const Y_CmdInputType *type, const char *input);

const Y_CmdInputType *y_cmd_type_file = &(Y_CmdInputType){
    .doc = {
        .name = "file",
        .description = "path to the file",
    },
    .is_satisfies = is_file,
};

void y_cmd_mgr_write_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(mgr == NULL){
        Y_FAULT("%s(): mgr == NULL", __func__);
    }

    write_doc(stream, &mgr->doc, 1);
    WC('\n');

    WS("USAGE:\n");
    write_tabs(stream, 1);
        write_doc_name(stream, mgr->doc.name);
        write_required_arguments_usage(stream, mgr);
        write_flags_usage(stream, mgr);
        write_rest_usage(stream, mgr);
    WS("\n\n");

    write_required_docs(stream, mgr);
    WS("\n");

    write_rest_docs(stream, mgr);
    WS("\n");

    write_flags_docs(stream, mgr);

    fflush(stream);
}

bool y_cmd_mgr_flags_overlapped(const Y_CmdMgr *mgr){
    if(mgr == NULL){
        Y_FAULT("%s(): mgr == NULL", __func__);
    }

    for (unsigned i = 0; i < mgr->flags_cnt; i++){
        for (unsigned j = 0; j < i; j++){
            if(str_overlaps(mgr->flags[i].flag, mgr->flags[j].flag)){
                return true;
            }
        }
    }
    return false;
}

static int pseudo_fprintf(FILE *file, const char *fmt, ...){
    (void)file;//unused
    (void)fmt;//unused
    return 0;
}

//TODO: more convinient error messages
//TODO?: devide into different parts
bool y_cmd_mgr_try_cpature(Y_CmdMgr *mgr, int argc, const char **argv, FILE *error_stream, bool write_errors){
    if(mgr == NULL) Y_FAULT("%s(): mgr == NULL", __func__);
    if(argv == NULL) Y_FAULT("%s(): argv == NULL", __func__);
    if(argc <= 0) Y_FAULT("%s(): argc == %i", __func__, argc);
    
    int (*log)(FILE *file, const char *fmt, ...) = write_errors ? fprintf : pseudo_fprintf;

    unsigned curr_required = 0;
    unsigned cnt_variadic = 0;
    
    for(int i = 1; i < argc; i++){
        const char *arg = argv[i];
        if(*arg == 0){
            log(error_stream, "WARNING: empty argument\n");
        }
        else if(*arg == '-'){
            Y_CmdFlag *flag = get_flag(mgr, arg + 1);

            if(!flag){
                log(error_stream, "ERROR: unknown flag \"%s\"\n", arg);
                return false;
            }
            if(!flag->input_type){
                if(flag->on_capture) flag->on_capture(flag->data, "");
                continue;
            }
            if(++i >= argc){
                log(error_stream, "ERROR: missing parameter for flag [%s]\n", arg);
                return false;
            }
            const char *param = argv[i];

            if(!is_satisfies(flag->input_type, param)){
                log(error_stream, "ERROR: \"%s\" parameter for flag \"-%s\" is not satisfied for type \"%s\"\n", param, flag->doc.name, flag->input_type->doc.name);
                return false;
            }
            if(flag->on_capture) flag->on_capture(flag->data, param);
        }
        else if(curr_required < mgr->requred_cnt){
            const Y_CmdRequred *required = mgr->requred + curr_required;
            if(!is_satisfies(required->input_type, arg)){
                log(error_stream, "ERROR: \"%s\" parameter for required param \"%s\" is not satisfied for type \"%s\"\n", arg, required->doc.name, required->input_type->doc.name);
                return false;
            }
            if(required->on_capture) required->on_capture(required->data, arg);
            curr_required++;
        }
        else if(!mgr->rest){
            log(error_stream, "ERROR: unexpected argument \"%s\"\n", arg);
            return false;
        }
        else if(!((int)cnt_variadic < mgr->rest->max_cout || mgr->rest->max_cout < 0)){
            log(error_stream, "ERROR: to mutch variadic arguments (max count is %i)\n", mgr->rest->max_cout);
            return false;   
        }
        else if(!is_satisfies(mgr->rest->input_type, arg)){
            log(error_stream, "ERROR: \"%s\" parameter for variadic param \"%s\" is not satisfied for type \"%s\"\n", arg, mgr->rest->doc.name, mgr->rest->input_type->doc.name);
            return false;
        }
        else{
            if(mgr->rest->on_capture) mgr->rest->on_capture(mgr->rest->data, arg);
            cnt_variadic++;
        }
    }

    if(curr_required < mgr->requred_cnt){
        log(error_stream, "ERROR: missing required argument <%s>\n", mgr->requred[curr_required].doc.name);
        return false;
    }

    if(mgr->rest && cnt_variadic < mgr->rest->min_cout){
        log(error_stream, "ERROR: not enough variadic args (min count is %i)", mgr->rest->min_cout);
        return false;
    }

    return true;
}


//static

static bool is_file(const char *input){
    if(input){
        Y_TODO("is_filepath()");
        return true;
    }
    else{
        return false;
    }
}

static void write_doc(FILE *stream, const Y_CmdDoc *doc, unsigned int tabs){
    if(doc == NULL){
        WS("***undocumented***");
    }
    else{
        write_doc_name(stream, doc->name);
        WS(":\n");

        write_tabs(stream, tabs );
        write_doc_description(stream, doc->description, tabs);
    }
    WC('\n');
}

static void write_tabs(FILE *stream, unsigned int tabs){
    while (tabs--){
        WS("    ");
    }
}

static void write_multiline(FILE *stream, unsigned int tabs, const char *text){
    if(text){
        while (*text){
            WC(*text);
            if(*text == '\n'){
                write_tabs(stream, tabs);
            }
            text++;
        }
    }
}

static void write_doc_name(FILE *stream, const char *name){
    if(name){
        WS(name);
    }
    else{
        WS("***unnamed***");
    }
}

static void write_doc_description(FILE *stream, const char *descritpion, unsigned int tabs){
    if(descritpion){
        write_multiline(stream, tabs, descritpion);
    }
    else{
        WS("***undescribed***");
    }
}

static void write_required_arguments_usage(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr) return;

    for(unsigned int i = 0; i < mgr->requred_cnt; i++){
        const Y_CmdRequred *required = mgr->requred + i;

        WC(' ');
        WC('<');
        write_doc_name(stream, required->doc.name);

        WC(':');

        if(mgr->requred[i].input_type){
            write_doc_name(stream, required->input_type->doc.name);
        }
        else{
            WS("NULL");
        }
        WC('>');
    }
}

static void write_flags_usage(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr) return;
    for(unsigned int i = 0; i < mgr->flags_cnt; i++){
        const Y_CmdFlag *flag = mgr->flags + i;

        WC(' ');
        WS("[-");
        if(flag->flag){
            WS(flag->flag);
        }
        else{
            WS("NULL");
        }

        WC(':');

        if(flag->input_type){
            write_doc_name(stream, flag->input_type->doc.name);
        }
        else{
            WS("NULL");
        }
        WC(']');
    }
}

static void write_rest_usage(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr || mgr->rest) return;
    
    WS(" [");
    write_doc_name(stream, mgr->rest->doc.name);
    WC(':');
    if(mgr->rest->input_type){
        write_doc_name(stream, mgr->rest->input_type->doc.name);
    }
    else{
        WS("NULL");
    }
    WFMT(" ... (>= %u", mgr->rest->min_cout);
    if(mgr->rest->max_cout >= 0){
        WFMT(", < %i", mgr->rest->max_cout + 1);
    }
    WS(" items)");
    WC(']');
}

static void write_required_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr || !mgr->requred_cnt) return;

    WS("REQUIRED:\n");
    for (unsigned int i = 0; i < mgr->requred_cnt; i++){
        const Y_CmdRequred *required = mgr->requred + i;

        write_tabs(stream, 1);
        write_doc(stream, &required->doc, 2);
        WC('\n');

        write_type_docs(stream, required->input_type, 2);

        if(i != mgr->requred_cnt - 1){
            WS("\n");
        }
    }
}

static void write_rest_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr || !mgr->rest) return;

    WS("VARIADIC:\n");
    write_tabs(stream, 1);

    const Y_CmdRest *rest = mgr->rest;
    write_doc(stream, &rest->doc, 2);

    WC('\n');
    write_type_docs(stream, rest->input_type, 1);
    WC('\n');

    write_tabs(stream, 1);
    WS("COUNT:\n");
    write_tabs(stream, 2);

    write_rest_amout(stream, rest);
    WS("\n");
}

static void write_rest_amout(FILE *stream, const Y_CmdRest *rest){
    if(rest->min_cout == 0 && rest->max_cout < 0){
        WS("any amount of items");
    }
    else if(rest->min_cout == 0){
        WFMT("less or equal to %i", rest->max_cout);
    }
    else if(rest->max_cout < 0){
        WFMT("greater or equal to %i", (int)rest->min_cout);
    }
    else{
        WFMT("greater or equal to %i and less or equal to %i", (int)rest->min_cout, rest->max_cout);
    }
}

static void write_type_docs(FILE *stream, const Y_CmdInputType *type, unsigned int tabs){
    write_tabs(stream, tabs);
    WS("TYPE:");
    if(type){
        write_doc(stream, &type->doc, tabs + 1);
    }
    else{
        WS("NULL\n");
    }
}

static void write_flags_docs(FILE *stream, const Y_CmdMgr *mgr){
    if(!mgr ||  !mgr->flags_cnt) return;

    WS("FLAGS:\n");

    for (unsigned int i = 0; i < mgr->flags_cnt; i++){
        const Y_CmdFlag *flag = mgr->flags + i;

        write_tabs(stream, 1);
        WFMT("-%s:\n", flag->flag);

        write_tabs(stream, 2);
        write_doc(stream, &flag->doc, 3);
        WS("\n");
        write_type_docs(stream, flag->input_type, 2);

        if(i != mgr->flags_cnt - 1){
            WS("\n");
        }
    }
}

static bool str_overlaps(const char *first, const char *second){
    bool result = false;
    while (*first && *second){
        if(*first++ == *second++){
            result = true;
        }
        else{
            result = false;
        }
    }

    return result;
}

static Y_CmdFlag *get_flag(Y_CmdMgr *mgr, const char *flag){
    for (unsigned i = 0; i < mgr->flags_cnt; i++){
        Y_CmdFlag *f = mgr->flags + i;

        if(!strncmp(f->flag, flag, strlen(flag))){
            return f;
        }
    }
    return NULL;
}

static bool is_satisfies(const Y_CmdInputType *type, const char *input){
    if(input && type && type->is_satisfies){
        return type->is_satisfies(input);
    }
    else{
        return false;
    }
}

